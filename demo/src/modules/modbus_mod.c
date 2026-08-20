#include "demo.h"
#include "menu_util.h"
#include "cli_util.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#ifdef WITH_MODBUS
#include "iepro_hw.h"
#include "serial_port.h"
#include <modbus/modbus.h>

#define MB_HOST_SIZE        128
#define MB_MAX_REGS         125
#define MB_QUERY_SIZE       MODBUS_TCP_MAX_ADU_LENGTH

enum mb_link_type {
    MB_LINK_RTU = 0,
    MB_LINK_TCP = 1
};

enum mb_role_type {
    MB_ROLE_MASTER = 0,
    MB_ROLE_SLAVE = 1
};

struct modbus_config {
    int link;
    int role;
    int serial_port;
    int baud;
    char parity;
    int data_bits;
    int stop_bits;
    char tcp_host[MB_HOST_SIZE];
    int tcp_port;
    int slave_id;
    int start_addr;
    int reg_count;
    int function_code;
    int poll_interval_sec;
    int nb_holding_regs;
};

static struct modbus_config g_mb;
static pthread_t g_mb_thread;
static volatile int g_mb_running;
static volatile int g_mb_stop;
static volatile int g_mb_thread_done;
static int g_mb_thread_created;
static modbus_mapping_t *g_mb_mapping;
static pthread_mutex_t g_mb_mapping_lock = PTHREAD_MUTEX_INITIALIZER;

static int mb_read_line(const char *prompt, char *buf, size_t len)
{
    if (menu_read_line(prompt, buf, len) < 0)
        return -1;
    return 0;
}

static char mb_parse_parity(const char *line, char current)
{
    char p;

    if (!line || line[0] == '\0')
        return current;

    p = line[0];
    if (p >= 'a' && p <= 'z')
        p = (char)(p - 'a' + 'A');
    if (p == 'N' || p == 'E' || p == 'O')
        return p;
    return current;
}

static const char *mb_link_name(int link)
{
    return link == MB_LINK_TCP ? "tcp" : "rtu";
}

static const char *mb_role_name(int role)
{
    return role == MB_ROLE_SLAVE ? "slave" : "master";
}

static void modbus_config_init(void)
{
    g_mb.link = MB_LINK_RTU;
    g_mb.role = MB_ROLE_MASTER;
    g_mb.serial_port = IEPRO_SERIAL_PORT_RS485_1;
    g_mb.baud = 9600;
    g_mb.parity = 'N';
    g_mb.data_bits = 8;
    g_mb.stop_bits = 1;
    snprintf(g_mb.tcp_host, sizeof(g_mb.tcp_host), "127.0.0.1");
    g_mb.tcp_port = 502;
    g_mb.slave_id = 1;
    g_mb.start_addr = 0;
    g_mb.reg_count = 10;
    g_mb.function_code = 3;
    g_mb.poll_interval_sec = 5;
    g_mb.nb_holding_regs = 64;
}

static int modbus_is_running(void)
{
    return g_mb_running != 0;
}

static modbus_t *modbus_create_ctx(void)
{
    modbus_t *ctx;

    if (g_mb.link == MB_LINK_TCP) {
        if (g_mb.role == MB_ROLE_SLAVE)
            ctx = modbus_new_tcp("0.0.0.0", g_mb.tcp_port);
        else
            ctx = modbus_new_tcp(g_mb.tcp_host, g_mb.tcp_port);
    } else {
        const char *dev = iepro_serial_device_for_port(g_mb.serial_port);

        if (!dev) {
            fprintf(stderr, "invalid serial port selection\n");
            return NULL;
        }
        ctx = modbus_new_rtu(dev, g_mb.baud, g_mb.parity,
                             g_mb.data_bits, g_mb.stop_bits);
    }

    if (!ctx) {
        fprintf(stderr, "failed to create modbus context\n");
        return NULL;
    }

    if (modbus_set_slave(ctx, g_mb.slave_id) < 0) {
        fprintf(stderr, "modbus_set_slave failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return NULL;
    }

    modbus_set_response_timeout(ctx, 2, 0);
    if (g_mb.link != MB_LINK_TCP)
        modbus_set_byte_timeout(ctx, 0, 500000);
    return ctx;
}

static int modbus_open_ctx(modbus_t *ctx)
{
    if (g_mb.link == MB_LINK_TCP && g_mb.role == MB_ROLE_SLAVE)
        return 0;

    if (modbus_connect(ctx) < 0) {
        fprintf(stderr, "modbus_connect failed: %s\n", modbus_strerror(errno));
        return -1;
    }
    return 0;
}

static void modbus_print_regs(const char *title, uint16_t *regs, int count)
{
    int i;

    printf("%s:", title);
    for (i = 0; i < count; i++) {
        if (i % 8 == 0)
            printf("\n ");
        printf(" %5u", regs[i]);
    }
    printf("\n");
}

static int modbus_master_read_once(modbus_t *ctx, int start, int count,
                                   uint16_t *regs)
{
    if (g_mb.function_code == 4)
        return modbus_read_input_registers(ctx, start, count, regs);
    return modbus_read_registers(ctx, start, count, regs);
}

static void modbus_stop_worker(void)
{
    if (!g_mb_thread_created)
        return;

    g_mb_stop = 1;
    pthread_join(g_mb_thread, NULL);
    g_mb_thread_created = 0;
    g_mb_running = 0;
    g_mb_stop = 0;
}

static void modbus_cli_run_until_stop(void)
{
    menu_reset_stop();
    printf("Modbus worker running — press Ctrl+C to stop.\n");
    while (!menu_stop_requested()) {
        if (modbus_is_running()) {
            sleep(1);
            continue;
        }
        if (!g_mb_thread_created)
            break;
        if (g_mb_thread_done) {
            fprintf(stderr,
                    "Modbus worker exited "
                    "(check serial port, wiring, or slave device).\n");
            break;
        }
        usleep(100000);
    }
}

static void modbus_free_mapping(void)
{
    if (!g_mb_mapping)
        return;
    pthread_mutex_lock(&g_mb_mapping_lock);
    modbus_mapping_free(g_mb_mapping);
    g_mb_mapping = NULL;
    pthread_mutex_unlock(&g_mb_mapping_lock);
}

static void *modbus_master_thread(void *arg)
{
    modbus_t *ctx = NULL;
    uint16_t regs[MB_MAX_REGS];
    int count = g_mb.reg_count;

    (void)arg;

    if (count < 1)
        count = 1;
    if (count > MB_MAX_REGS)
        count = MB_MAX_REGS;

    ctx = modbus_create_ctx();
    if (!ctx || modbus_open_ctx(ctx) < 0) {
        modbus_free(ctx);
        goto done;
    }

    if (g_mb_stop) {
        modbus_close(ctx);
        modbus_free(ctx);
        ctx = NULL;
        goto done;
    }

    g_mb_running = 1;
    printf("Modbus master started (%s/%s).\n",
           mb_link_name(g_mb.link), mb_role_name(g_mb.role));

    while (!g_mb_stop) {
        int rc;
        int waited = 0;

        rc = modbus_master_read_once(ctx, g_mb.start_addr, count, regs);
        if (rc < 0)
            fprintf(stderr, "modbus read failed: %s\n", modbus_strerror(errno));
        else
            modbus_print_regs("[Modbus] registers", regs, count);

        while (waited < g_mb.poll_interval_sec && !g_mb_stop) {
            sleep(1);
            waited++;
        }
    }

    modbus_close(ctx);
    modbus_free(ctx);
    ctx = NULL;

done:
    g_mb_running = 0;
    g_mb_thread_done = 1;
    return NULL;
}

static void *modbus_slave_thread(void *arg)
{
    modbus_t *ctx = NULL;
    uint8_t query[MB_QUERY_SIZE];
    int server_socket = -1;
    int rc;

    (void)arg;

    ctx = modbus_create_ctx();
    if (!ctx)
        goto done;

    pthread_mutex_lock(&g_mb_mapping_lock);
    g_mb_mapping = modbus_mapping_new(0, 0,
                                      (unsigned int)g_mb.nb_holding_regs,
                                      (unsigned int)g_mb.nb_holding_regs);
    pthread_mutex_unlock(&g_mb_mapping_lock);

    if (!g_mb_mapping) {
        fprintf(stderr, "modbus_mapping_new failed\n");
        goto done;
    }

    if (g_mb.link == MB_LINK_TCP) {
        server_socket = modbus_tcp_listen(ctx, 1);
        if (server_socket < 0) {
            fprintf(stderr, "modbus_tcp_listen failed: %s\n",
                    modbus_strerror(errno));
            goto done;
        }
        printf("Modbus TCP slave listening on port %d ...\n", g_mb.tcp_port);
        if (modbus_tcp_accept(ctx, &server_socket) < 0) {
            fprintf(stderr, "modbus_tcp_accept failed: %s\n",
                    modbus_strerror(errno));
            close(server_socket);
            server_socket = -1;
            goto done;
        }
    } else if (modbus_open_ctx(ctx) < 0) {
        goto done;
    }

    if (g_mb_stop)
        goto done;

    g_mb_running = 1;
    printf("Modbus slave started (%s/%s, unit id %d).\n",
           mb_link_name(g_mb.link), mb_role_name(g_mb.role), g_mb.slave_id);

    while (!g_mb_stop) {
        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            pthread_mutex_lock(&g_mb_mapping_lock);
            modbus_reply(ctx, query, (int)rc, g_mb_mapping);
            pthread_mutex_unlock(&g_mb_mapping_lock);
        } else if (rc == -1 && errno != EINTR && errno != EAGAIN) {
            fprintf(stderr, "modbus_receive failed: %s\n",
                    modbus_strerror(errno));
            break;
        }
    }

done:
    if (server_socket >= 0)
        close(server_socket);
    if (ctx) {
        modbus_close(ctx);
        modbus_free(ctx);
    }
    modbus_free_mapping();
    g_mb_running = 0;
    g_mb_thread_done = 1;
    return NULL;
}

static int modbus_start_worker(void)
{
    int rc;
    void *(*worker)(void *) = modbus_master_thread;

    if (g_mb_thread_created) {
        if (modbus_is_running())
            printf("Modbus worker already running.\n");
        else
            modbus_stop_worker();
    }

    if (g_mb.role == MB_ROLE_SLAVE)
        worker = modbus_slave_thread;

    if (g_mb.role == MB_ROLE_MASTER) {
        if (g_mb.reg_count < 1 || g_mb.reg_count > MB_MAX_REGS) {
            printf("Invalid register count (1-%d).\n", MB_MAX_REGS);
            return -1;
        }
        if (g_mb.poll_interval_sec < 1) {
            printf("Invalid poll interval (>=1 s).\n");
            return -1;
        }
    } else if (g_mb.nb_holding_regs < 1 || g_mb.nb_holding_regs > MB_MAX_REGS) {
        printf("Invalid holding register map size (1-%d).\n", MB_MAX_REGS);
        return -1;
    }

    g_mb_stop = 0;
    g_mb_thread_done = 0;
    rc = pthread_create(&g_mb_thread, NULL, worker, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_create failed: %s\n", strerror(rc));
        return -1;
    }
    g_mb_thread_created = 1;
    return 0;
}

static void modbus_show_config(void)
{
    printf("\n--- Modbus configuration ---\n");
    printf("Link         : %s\n", mb_link_name(g_mb.link));
    printf("Role         : %s\n", mb_role_name(g_mb.role));
    if (g_mb.link == MB_LINK_TCP) {
        if (g_mb.role == MB_ROLE_SLAVE)
            printf("TCP listen   : 0.0.0.0:%d\n", g_mb.tcp_port);
        else
            printf("TCP remote   : %s:%d\n", g_mb.tcp_host, g_mb.tcp_port);
    } else {
        const char *dev = iepro_serial_device_for_port(g_mb.serial_port);

        printf("Serial port  : %s (%s)\n",
               iepro_serial_port_label(g_mb.serial_port),
               dev ? dev : "?");
        printf("Baud         : %d\n", g_mb.baud);
        printf("Parity       : %c\n", g_mb.parity);
        printf("Data bits    : %d\n", g_mb.data_bits);
        printf("Stop bits    : %d\n", g_mb.stop_bits);
    }
    printf("Unit ID      : %d\n", g_mb.slave_id);
    if (g_mb.role == MB_ROLE_MASTER) {
        printf("Start addr   : %d\n", g_mb.start_addr);
        printf("Reg count    : %d\n", g_mb.reg_count);
        printf("Function code: %d (3=holding, 4=input)\n", g_mb.function_code);
        printf("Poll interval: %d s\n", g_mb.poll_interval_sec);
    } else {
        printf("Holding regs : %d\n", g_mb.nb_holding_regs);
    }
    printf("Running      : %s\n", modbus_is_running() ? "yes" : "no");
    printf("----------------------------\n");
}

static void modbus_configure_params(void)
{
    char line[MB_HOST_SIZE];
    int value;

    if (modbus_is_running()) {
        printf("Stop Modbus worker before changing configuration.\n");
        return;
    }

    printf("\n--- Configure Modbus ---\n");
    printf("(Press Enter on each field to keep the current value.)\n\n");

    printf("Link type (rtu/tcp) [%s]: ", mb_link_name(g_mb.link));
    fflush(stdout);
    if (mb_read_line("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] != '\0') {
        if (!strcasecmp(line, "tcp"))
            g_mb.link = MB_LINK_TCP;
        else if (!strcasecmp(line, "rtu"))
            g_mb.link = MB_LINK_RTU;
        else
            printf("Unknown link type, keeping current.\n");
    }

    printf("Role (master/slave) [%s]: ", mb_role_name(g_mb.role));
    fflush(stdout);
    if (mb_read_line("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] != '\0') {
        if (!strcasecmp(line, "slave"))
            g_mb.role = MB_ROLE_SLAVE;
        else if (!strcasecmp(line, "master"))
            g_mb.role = MB_ROLE_MASTER;
        else
            printf("Unknown role, keeping current.\n");
    }

    if (g_mb.link == MB_LINK_TCP) {
        if (g_mb.role == MB_ROLE_MASTER) {
            printf("TCP host [%s]: ", g_mb.tcp_host);
            fflush(stdout);
            if (mb_read_line("", line, sizeof(line)) < 0) {
                printf("Cancelled.\n");
                return;
            }
            if (line[0] != '\0')
                snprintf(g_mb.tcp_host, sizeof(g_mb.tcp_host), "%s", line);
        }
        value = menu_read_int("TCP port: ", g_mb.tcp_port);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value > 0 && value <= 65535)
            g_mb.tcp_port = value;
    } else {
        value = iepro_serial_pick_port(g_mb.serial_port);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        g_mb.serial_port = value;

        value = menu_read_int("Baud rate: ", g_mb.baud);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value > 0)
            g_mb.baud = value;

        printf("Parity (N/E/O) [%c]: ", g_mb.parity);
        fflush(stdout);
        if (mb_read_line("", line, sizeof(line)) < 0) {
            printf("Cancelled.\n");
            return;
        }
        g_mb.parity = mb_parse_parity(line, g_mb.parity);

        value = menu_read_int("Data bits (7/8): ", g_mb.data_bits);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value == 7 || value == 8)
            g_mb.data_bits = value;

        value = menu_read_int("Stop bits (1/2): ", g_mb.stop_bits);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value == 1 || value == 2)
            g_mb.stop_bits = value;
    }

    value = menu_read_int("Unit ID: ", g_mb.slave_id);
    if (value == MENU_CANCEL) {
        printf("Cancelled.\n");
        return;
    }
    if (value >= 0 && value <= 247)
        g_mb.slave_id = value;

    if (g_mb.role == MB_ROLE_MASTER) {
        value = menu_read_int("Start address: ", g_mb.start_addr);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value >= 0)
            g_mb.start_addr = value;

        value = menu_read_int("Register count: ", g_mb.reg_count);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value >= 1 && value <= MB_MAX_REGS)
            g_mb.reg_count = value;

        value = menu_read_int("Function code (3/4): ", g_mb.function_code);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value == 3 || value == 4)
            g_mb.function_code = value;

        value = menu_read_int("Poll interval (seconds): ",
                              g_mb.poll_interval_sec);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value >= 1)
            g_mb.poll_interval_sec = value;
    } else {
        value = menu_read_int("Holding register map size: ",
                              g_mb.nb_holding_regs);
        if (value == MENU_CANCEL) {
            printf("Cancelled.\n");
            return;
        }
        if (value >= 1 && value <= MB_MAX_REGS)
            g_mb.nb_holding_regs = value;
    }

    printf("Modbus configuration updated.\n");
}

static void modbus_master_read_once_menu(void)
{
    modbus_t *ctx;
    uint16_t regs[MB_MAX_REGS];
    int count = g_mb.reg_count;
    int rc;

    if (g_mb.role != MB_ROLE_MASTER) {
        printf("Current role is slave.\n");
        return;
    }
    if (modbus_is_running()) {
        printf("Stop continuous master loop before one-shot read.\n");
        return;
    }
    if (count < 1 || count > MB_MAX_REGS) {
        printf("Invalid register count.\n");
        return;
    }

    ctx = modbus_create_ctx();
    if (!ctx || modbus_open_ctx(ctx) < 0) {
        modbus_free(ctx);
        return;
    }

    rc = modbus_master_read_once(ctx, g_mb.start_addr, count, regs);
    if (rc < 0)
        fprintf(stderr, "modbus read failed: %s\n", modbus_strerror(errno));
    else
        modbus_print_regs("Read result", regs, count);

    modbus_close(ctx);
    modbus_free(ctx);
}

static void modbus_master_write_menu(void)
{
    modbus_t *ctx;
    int addr;
    int value;
    int rc;

    if (g_mb.role != MB_ROLE_MASTER) {
        printf("Current role is slave.\n");
        return;
    }
    if (modbus_is_running()) {
        printf("Stop continuous master loop before write.\n");
        return;
    }

    addr = menu_read_int("Holding register address: ", g_mb.start_addr);
    if (addr == MENU_CANCEL)
        return;
    value = menu_read_int("Value (0-65535): ", 0);
    if (value == MENU_CANCEL)
        return;
    if (value < 0 || value > 65535) {
        printf("Invalid value.\n");
        return;
    }

    ctx = modbus_create_ctx();
    if (!ctx || modbus_open_ctx(ctx) < 0) {
        modbus_free(ctx);
        return;
    }

    rc = modbus_write_register(ctx, addr, (uint16_t)value);
    if (rc < 0)
        fprintf(stderr, "modbus write failed: %s\n", modbus_strerror(errno));
    else
        printf("Wrote register %d = %d\n", addr, value);

    modbus_close(ctx);
    modbus_free(ctx);
}

static void modbus_slave_set_register_menu(void)
{
    int addr;
    int value;

    if (g_mb.role != MB_ROLE_SLAVE) {
        printf("Current role is master.\n");
        return;
    }
    if (!modbus_is_running() || !g_mb_mapping) {
        printf("Start slave worker first.\n");
        return;
    }

    addr = menu_read_int("Holding register address: ", 0);
    if (addr == MENU_CANCEL)
        return;
    if (addr < 0 || addr >= g_mb.nb_holding_regs) {
        printf("Address out of range (0-%d).\n", g_mb.nb_holding_regs - 1);
        return;
    }

    value = menu_read_int("Value (0-65535): ", 0);
    if (value == MENU_CANCEL)
        return;
    if (value < 0 || value > 65535) {
        printf("Invalid value.\n");
        return;
    }

    pthread_mutex_lock(&g_mb_mapping_lock);
    g_mb_mapping->tab_registers[addr] = (uint16_t)value;
    pthread_mutex_unlock(&g_mb_mapping_lock);
    printf("Slave holding register %d set to %d\n", addr, value);
}

static void modbus_show_menu(void)
{
    printf("\n--- Modbus module ---\n");
    printf(" 1) Configure connection parameters\n");
    printf(" 2) Show current configuration\n");
    printf(" 3) Start worker (master poll / slave server)\n");
    printf(" 4) Stop worker\n");
    printf(" 5) Master: one-shot read\n");
    printf(" 6) Master: write holding register\n");
    printf(" 7) Slave: set holding register value\n");
    printf(" 0) Back to main menu (Ctrl+C)\n");
}

int modbus_module_menu(void)
{
    modbus_config_init();

    for (;;) {
        int choice;

        modbus_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK) {
            modbus_stop_worker();
            return 0;
        }

        switch (choice) {
        case 1:
            modbus_configure_params();
            break;
        case 2:
            modbus_show_config();
            break;
        case 3:
            modbus_start_worker();
            break;
        case 4:
            modbus_stop_worker();
            printf("Modbus worker stopped.\n");
            break;
        case 5:
            modbus_master_read_once_menu();
            break;
        case 6:
            modbus_master_write_menu();
            break;
        case 7:
            modbus_slave_set_register_menu();
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}

void modbus_module_cli_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s modbus <action> [options]\n"
            "  run            Start worker until Ctrl+C (menu 3)\n"
            "  read           Master one-shot read (menu 5)\n"
            "  write          Master write register (menu 6)\n"
            "Options:\n"
            "  --link rtu|tcp  --role master|slave\n"
            "  --port 1|2|3   RTU serial (1=RS232, 2=RS485-1, 3=RS485-2)\n"
            "  --baud N  --parity N|E|O  --data-bits 7|8  --stop-bits 1|2\n"
            "  --host HOST  --tcp-port N  --unit-id N\n"
            "  --start-addr N  --count N  --function 3|4  --poll-interval N\n"
            "  --holding-regs N\n"
            "  --addr N  --value N   (write)\n"
            "\n"
            "Examples:\n"
            "  RTU master — poll RS485-1 slave id 1, holding regs 0-9:\n"
            "    %s modbus run --link rtu --role master --port 2 --baud 9600 \\\n"
            "      --unit-id 1 --start-addr 0 --count 10 --function 3 --poll-interval 5\n"
            "  RTU master — one-shot read:\n"
            "    %s modbus read --link rtu --role master --port 2 --baud 9600 \\\n"
            "      --unit-id 1 --start-addr 0 --count 10 --function 3\n"
            "  RTU master — write holding register 5 = 1234:\n"
            "    %s modbus write --link rtu --role master --port 2 --baud 9600 \\\n"
            "      --unit-id 1 --addr 5 --value 1234\n"
            "  RTU slave — listen on RS485-1, unit id 1, 64 holding regs:\n"
            "    %s modbus run --link rtu --role slave --port 2 --baud 9600 \\\n"
            "      --unit-id 1 --holding-regs 64\n"
            "  TCP master — poll remote PLC at 192.168.1.100:502:\n"
            "    %s modbus run --link tcp --role master --host 192.168.1.100 \\\n"
            "      --tcp-port 502 --unit-id 1 --start-addr 0 --count 10 --poll-interval 5\n"
            "  TCP master — one-shot read:\n"
            "    %s modbus read --link tcp --role master --host 192.168.1.100 \\\n"
            "      --tcp-port 502 --unit-id 1 --start-addr 0 --count 10 --function 3\n"
            "  TCP slave — listen on port 502:\n"
            "    %s modbus run --link tcp --role slave --tcp-port 502 \\\n"
            "      --unit-id 1 --holding-regs 64\n",
            prog, prog, prog, prog, prog, prog, prog, prog);
}

int modbus_module_cli(int argc, char **argv)
{
    const char *action = argv[1];
    int write_addr = -1;
    int write_value = -1;
    int opt;

    static const struct option opts[] = {
        { "link", required_argument, NULL, 'L' },
        { "role", required_argument, NULL, 'R' },
        { "port", required_argument, NULL, 'p' },
        { "baud", required_argument, NULL, 'b' },
        { "parity", required_argument, NULL, 'y' },
        { "data-bits", required_argument, NULL, 'd' },
        { "stop-bits", required_argument, NULL, 'S' },
        { "host", required_argument, NULL, 'H' },
        { "tcp-port", required_argument, NULL, 'P' },
        { "unit-id", required_argument, NULL, 'u' },
        { "start-addr", required_argument, NULL, 'a' },
        { "count", required_argument, NULL, 'n' },
        { "function", required_argument, NULL, 'f' },
        { "poll-interval", required_argument, NULL, 'I' },
        { "holding-regs", required_argument, NULL, 'g' },
        { "addr", required_argument, NULL, 'A' },
        { "value", required_argument, NULL, 'v' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    modbus_config_init();

    if (!action || !strcmp(action, "-h") || !strcmp(action, "--help")) {
        modbus_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    optind = 2;
    while ((opt = getopt_long(argc, argv,
                              "L:R:p:b:y:d:S:H:P:u:a:n:f:I:g:A:v:h",
                              opts, NULL)) != -1) {
        int val;

        switch (opt) {
        case 'L':
            if (!strcasecmp(optarg, "tcp"))
                g_mb.link = MB_LINK_TCP;
            else if (!strcasecmp(optarg, "rtu"))
                g_mb.link = MB_LINK_RTU;
            break;
        case 'R':
            if (!strcasecmp(optarg, "slave"))
                g_mb.role = MB_ROLE_SLAVE;
            else if (!strcasecmp(optarg, "master"))
                g_mb.role = MB_ROLE_MASTER;
            break;
        case 'p':
            val = cli_parse_serial_port(optarg, g_mb.serial_port);
            if (val > 0)
                g_mb.serial_port = val;
            break;
        case 'b':
            if (cli_parse_int(optarg, &val) == 0 && val > 0)
                g_mb.baud = val;
            break;
        case 'y':
            g_mb.parity = mb_parse_parity(optarg, g_mb.parity);
            break;
        case 'd':
            if (cli_parse_int(optarg, &val) == 0 &&
                (val == 7 || val == 8))
                g_mb.data_bits = val;
            break;
        case 'S':
            if (cli_parse_int(optarg, &val) == 0 &&
                (val == 1 || val == 2))
                g_mb.stop_bits = val;
            break;
        case 'H':
            snprintf(g_mb.tcp_host, sizeof(g_mb.tcp_host), "%s", optarg);
            break;
        case 'P':
            if (cli_parse_int(optarg, &val) == 0 &&
                val > 0 && val <= 65535)
                g_mb.tcp_port = val;
            break;
        case 'u':
            if (cli_parse_int(optarg, &val) == 0 &&
                val >= 0 && val <= 247)
                g_mb.slave_id = val;
            break;
        case 'a':
            if (cli_parse_int(optarg, &val) == 0 && val >= 0)
                g_mb.start_addr = val;
            break;
        case 'n':
            if (cli_parse_int(optarg, &val) == 0 &&
                val >= 1 && val <= MB_MAX_REGS)
                g_mb.reg_count = val;
            break;
        case 'f':
            if (cli_parse_int(optarg, &val) == 0 &&
                (val == 3 || val == 4))
                g_mb.function_code = val;
            break;
        case 'I':
            if (cli_parse_int(optarg, &val) == 0 && val >= 1)
                g_mb.poll_interval_sec = val;
            break;
        case 'g':
            if (cli_parse_int(optarg, &val) == 0 &&
                val >= 1 && val <= MB_MAX_REGS)
                g_mb.nb_holding_regs = val;
            break;
        case 'A':
            if (cli_parse_int(optarg, &val) == 0 && val >= 0)
                write_addr = val;
            break;
        case 'v':
            if (cli_parse_int(optarg, &val) == 0 &&
                val >= 0 && val <= 65535)
                write_value = val;
            break;
        case 'h':
            modbus_module_cli_usage(argv[0]);
            return CLI_EXIT_OK;
        default:
            modbus_module_cli_usage(argv[0]);
            return CLI_EXIT_USAGE;
        }
    }

    if (!strcmp(action, "run")) {
        if (modbus_start_worker() < 0)
            return CLI_EXIT_FAIL;

        modbus_cli_run_until_stop();
        modbus_stop_worker();
        printf("Modbus worker stopped.\n");
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "read")) {
        modbus_master_read_once_menu();
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "write")) {
        modbus_t *ctx;
        int rc;

        if (g_mb.role != MB_ROLE_MASTER) {
            fprintf(stderr, "Current role is slave.\n");
            return CLI_EXIT_FAIL;
        }
        if (modbus_is_running()) {
            fprintf(stderr, "Stop continuous master loop before write.\n");
            return CLI_EXIT_FAIL;
        }
        if (write_addr < 0 || write_value < 0) {
            fprintf(stderr, "write requires --addr and --value.\n");
            return CLI_EXIT_USAGE;
        }

        ctx = modbus_create_ctx();
        if (!ctx || modbus_open_ctx(ctx) < 0) {
            modbus_free(ctx);
            return CLI_EXIT_FAIL;
        }

        rc = modbus_write_register(ctx, write_addr, (uint16_t)write_value);
        if (rc < 0) {
            fprintf(stderr, "modbus write failed: %s\n", modbus_strerror(errno));
            modbus_close(ctx);
            modbus_free(ctx);
            return CLI_EXIT_FAIL;
        }

        printf("Wrote register %d = %d\n", write_addr, write_value);
        modbus_close(ctx);
        modbus_free(ctx);
        return CLI_EXIT_OK;
    }

    fprintf(stderr, "Unknown modbus action: %s\n", action);
    modbus_module_cli_usage(argv[0]);
    return CLI_EXIT_USAGE;
}

#else

int modbus_module_menu(void)
{
    printf("\nModbus module was not built.\n");
    printf("Rebuild with prebuilt deps (see deps/README.md).\n");
    menu_pause();
    return 0;
}

void modbus_module_cli_usage(const char *prog)
{
    fprintf(stderr, "Modbus module was not built (missing WITH_MODBUS).\n");
    (void)prog;
}

int modbus_module_cli(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("Modbus module was not built.\n");
    return CLI_EXIT_FAIL;
}

#endif
