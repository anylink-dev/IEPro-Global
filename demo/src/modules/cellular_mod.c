#include "demo.h"
#include "iepro_hw.h"
#include "gpio_util.h"
#include "menu_util.h"
#include "cli_util.h"

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

#define AT_RESP_SIZE 1024
#define AT_TIMEOUT_MS 1500
#define MODEM_DEV_WAIT_MS 10000
#define MODEM_DEV_POLL_MS 100

static int g_at_fd = -1;
static int g_cell_power_on = 0;

static int cellular_wait_modem_dev(void)
{
    int elapsed = 0;

    while (elapsed < MODEM_DEV_WAIT_MS) {
        if (access(IEPRO_MODEM_AT_DEV, F_OK) == 0)
            return 0;
        usleep((useconds_t)MODEM_DEV_POLL_MS * 1000);
        elapsed += MODEM_DEV_POLL_MS;
    }

    fprintf(stderr, "Timeout (%d s) waiting for modem AT device %s\n",
            MODEM_DEV_WAIT_MS / 1000, IEPRO_MODEM_AT_DEV);
    return -1;
}

static int cellular_ensure_power(void)
{
    if (g_cell_power_on)
        return 0;
    if (gpio_cell_power_on() < 0) {
        fprintf(stderr, "Failed to enable 4G module power (GPIO %d)\n",
                GPIO_CELL_PWR);
        return -1;
    }
    if (cellular_wait_modem_dev() < 0)
        return -1;
    g_cell_power_on = 1;
    return 0;
}

static speed_t cellular_baud_flag(void)
{
    return B115200;
}

static int cellular_at_open(void)
{
    struct termios tty;

    if (g_at_fd >= 0)
        return 0;

    if (cellular_ensure_power() < 0)
        return -1;

    g_at_fd = open(IEPRO_MODEM_AT_DEV, O_RDWR | O_NOCTTY | O_SYNC);
    if (g_at_fd < 0) {
        perror("open modem AT port");
        return -1;
    }

    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(g_at_fd, &tty) != 0) {
        perror("tcgetattr");
        close(g_at_fd);
        g_at_fd = -1;
        return -1;
    }

    cfsetospeed(&tty, cellular_baud_flag());
    cfsetispeed(&tty, cellular_baud_flag());
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(g_at_fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(g_at_fd);
        g_at_fd = -1;
        return -1;
    }
    return 0;
}

static void cellular_at_close(void)
{
    if (g_at_fd >= 0) {
        close(g_at_fd);
        g_at_fd = -1;
    }
}

static int cellular_at_read(int fd, char *buf, size_t len, int timeout_ms)
{
    size_t total = 0;
    struct timeval tv;
    fd_set rfds;

    buf[0] = '\0';
    while (total < len - 1) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        if (select(fd + 1, &rfds, NULL, NULL, &tv) <= 0)
            break;

        {
            ssize_t n = read(fd, buf + total, len - 1 - total);
            if (n <= 0)
                break;
            total += (size_t)n;
            buf[total] = '\0';
            if (strstr(buf, "OK") || strstr(buf, "ERROR"))
                break;
        }
    }
    return (int)total;
}

static int cellular_at_send_raw(const char *cmd, char *resp, size_t resp_len)
{
    char tx[256];
    int n;

    if (cellular_at_open() < 0)
        return -1;

    tcflush(g_at_fd, TCIOFLUSH);
    snprintf(tx, sizeof(tx), "%s\r\n", cmd);
    if (write(g_at_fd, tx, strlen(tx)) < 0) {
        perror("modem AT write");
        return -1;
    }

    if (!resp || resp_len == 0)
        return 0;

    n = cellular_at_read(g_at_fd, resp, resp_len, AT_TIMEOUT_MS);
    return n > 0 ? 0 : -1;
}

static int cellular_at_cmd(const char *cmd, char *resp, size_t resp_len)
{
    if (cellular_at_send_raw(cmd, resp, resp_len) != 0)
        return -1;
    if (!resp || resp_len == 0)
        return 0;
    return strstr(resp, "OK") ? 0 : -1;
}

static int cellular_at_ok(const char *cmd)
{
    char resp[AT_RESP_SIZE];

    return cellular_at_cmd(cmd, resp, sizeof(resp));
}

static void cellular_at_send_user(const char *cmd)
{
    char resp[AT_RESP_SIZE];

    printf(">> %s\n", cmd);
    if (cellular_at_send_raw(cmd, resp, sizeof(resp)) == 0)
        printf("%s\n", resp);
    else
        printf("(no response)\n");
}

typedef struct {
    const char *cmd;
    const char *desc;
    const char *menu;
} cellular_at_help_t;

static const cellular_at_help_t g_at_help[] = {
    { "ATI", "Module model and identification", "1" },
    { "AT+CGMR", "Firmware / revision string", "2" },
    { "AT+CGSN", "IMEI", "3" },
    { "AT+CCID", "SIM ICCID", "4" },
    { "AT+CIMI", "IMSI", "5" },
    { "AT+CPIN?", "SIM PIN status (READY / SIM PIN / SIM PUK)", "6" },
    { "AT+CSQ", "Signal strength (0-31, 99=no signal)", "7" },
    { "AT+COPS=3,0;+COPS?", "Current operator name", "8" },
    { "AT+COPS=3,2;+COPS?", "Current operator PLMN code", "8" },
    { "AT+CNSMOD?", "Current radio access technology", "9" },
    { "AT+CNSMOD=1", "Enable network mode URC (one-time setup)", "-" },
    { "AT+CEREG?", "LTE EPS registration status", "10" },
    { "AT+CGREG?", "GPRS registration status", "10" },
    { "AT$QCRMCALL?", "NDIS data call status", "11" },
    { "AT+CPSI?", "Serving cell information", "12" },
    { "AT$QCRMCALL=1,1", "Start NDIS dial-up (3GPP auto APN)", "13" },
    { "AT$QCRMCALL=1,1,,,,,\"<apn>\",\"<user>\",\"<pass>\",3",
      "Start dial-up with custom APN (menu 13 -> option 2)", "13" },
    { "AT$QCRMCALL=0,1", "Stop NDIS data call", "14" },
    { "AT+CFUN?", "Query phone functionality level", "-" },
    { "AT+CFUN=1", "Set full functionality (radio on)", "-" },
    { "AT+CFUN=0", "Set minimum functionality (radio off, use with care)", "-" },
};

static void cellular_print_help(void)
{
    size_t i;
    size_t count = sizeof(g_at_help) / sizeof(g_at_help[0]);

    printf("\n--- SIM7600G-H-PCIE AT command reference ---\n");
    printf("AT port : %s\n", IEPRO_MODEM_AT_DEV);
    printf("Data IF : %s\n", IEPRO_CELL_IFACE);
    printf("Dial-up : NDIS via AT$QCRMCALL (see docs/zh-CN/03-4g-connectivity.md)\n\n");
    printf("%-42s %-6s %s\n", "Command", "Menu", "Description");
    printf("%-42s %-6s %s\n", "-------", "----", "-----------");
    for (i = 0; i < count; i++) {
        printf("%-42s %-6s %s\n",
               g_at_help[i].cmd, g_at_help[i].menu, g_at_help[i].desc);
    }
    printf("\nMenu \"-\" = use custom AT console (option 18) or not in quick menu.\n");
    printf("CSQ guide: 0-9 weak, 10-14 fair, 15-19 good, 20-31 excellent.\n");
    printf("CEREG/CGREG: stat 1=home, 5=roaming, 0=not registered, 2=searching.\n");
}

static void cellular_at_console(void)
{
    char line[256];
    char cmd[280];

    printf("\n--- Custom AT console ---\n");
    printf("Enter AT command (AT prefix optional). Empty line or 0 to return.\n");
    for (;;) {
        if (menu_read_line("AT> ", line, sizeof(line)) < 0)
            break;
        if (line[0] == '\0' || !strcmp(line, "0"))
            break;

        if ((line[0] == 'A' || line[0] == 'a') &&
            (line[1] == 'T' || line[1] == 't'))
            snprintf(cmd, sizeof(cmd), "%s", line);
        else
            snprintf(cmd, sizeof(cmd), "AT%s", line);

        cellular_at_send_user(cmd);
    }
}

static const char *cellular_extract_line(const char *resp, const char *prefix)
{
    const char *p = resp;

    while (p && *p) {
        if (strncmp(p, prefix, strlen(prefix)) == 0)
            return p;
        p = strchr(p, '\n');
        if (!p)
            break;
        p++;
    }
    return NULL;
}

static void cellular_if_init(void)
{
    char cmd[160];

    snprintf(cmd, sizeof(cmd),
             "ifconfig %s up 2>/dev/null; ifconfig %s 0.0.0.0 2>/dev/null",
             IEPRO_CELL_IFACE, IEPRO_CELL_IFACE);
    system(cmd);

    snprintf(cmd, sizeof(cmd),
             "kill $(cat /var/run/udhcpc.%s.pid 2>/dev/null) 2>/dev/null",
             IEPRO_CELL_IFACE);
    system(cmd);
}

static void cellular_if_dhcp(void)
{
    char cmd[192];

    cellular_if_init();
    snprintf(cmd, sizeof(cmd),
             "udhcpc -R -n -p /var/run/udhcpc.%s.pid -i %s >/dev/null 2>&1 &",
             IEPRO_CELL_IFACE, IEPRO_CELL_IFACE);
    system(cmd);
    sleep(3);
}

static void cellular_show_iface_ip(void)
{
    char cmd[128];
    FILE *fp;

    snprintf(cmd, sizeof(cmd), "ip -4 -o addr show dev %s 2>/dev/null", IEPRO_CELL_IFACE);
    fp = popen(cmd, "r");
    if (!fp) {
        printf("Interface %s: (unable to query)\n", IEPRO_CELL_IFACE);
        return;
    }
    if (fgets(cmd, sizeof(cmd), fp))
        printf("Interface %s: %s", IEPRO_CELL_IFACE, cmd);
    else
        printf("Interface %s: no IPv4 address\n", IEPRO_CELL_IFACE);
    pclose(fp);
}

static void cellular_print_at_response(const char *title, const char *cmd)
{
    char resp[AT_RESP_SIZE];

    if (cellular_at_cmd(cmd, resp, sizeof(resp)) != 0) {
        printf("%s: query failed\n", title);
        return;
    }
    printf("%s:\n%s\n", title, resp);
}

static const char *cellular_netmode_name(const char *code)
{
    static const struct {
        const char *code;
        const char *name;
    } modes[] = {
        { "0",  "no service" },
        { "1",  "GSM" },
        { "2",  "GPRS" },
        { "3",  "EGPRS (EDGE)" },
        { "4",  "WCDMA" },
        { "5",  "HSDPA" },
        { "6",  "HSUPA" },
        { "7",  "HSPA" },
        { "8",  "LTE" },
        { "9",  "TD-SCDMA" },
        { "13", "CDMA" },
        { "14", "EVDO" },
        { "15", "HYBRID (CDMA+EVDO)" },
        { "16", "1XLTE" },
    };
    size_t i;

    for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
        if (!strcmp(code, modes[i].code))
            return modes[i].name;
    }
    return "unknown";
}

static void cellular_print_version(void)
{
    cellular_print_at_response("Module version (ATI)", "ATI");
}

static void cellular_print_imei(void)
{
    cellular_print_at_response("IMEI (AT+CGSN)", "AT+CGSN");
}

static void cellular_print_iccid(void)
{
    cellular_print_at_response("ICCID (AT+CCID)", "AT+CCID");
}

static void cellular_print_firmware(void)
{
    cellular_print_at_response("Firmware version (AT+CGMR)", "AT+CGMR");
}

static void cellular_print_imsi(void)
{
    cellular_print_at_response("IMSI (AT+CIMI)", "AT+CIMI");
}

static void cellular_print_sim_status(void)
{
    char resp[AT_RESP_SIZE];
    const char *line;

    if (cellular_at_cmd("AT+CPIN?", resp, sizeof(resp)) != 0) {
        printf("SIM status: query failed\n");
        return;
    }
    line = cellular_extract_line(resp, "+CPIN:");
    if (!line) {
        printf("SIM status:\n%s\n", resp);
        return;
    }
    printf("SIM status: %s\n", line);
    if (strstr(line, "READY"))
        printf("  -> SIM ready\n");
    else if (strstr(line, "SIM PIN"))
        printf("  -> PIN required\n");
    else if (strstr(line, "SIM PUK"))
        printf("  -> PUK required\n");
}

static void cellular_print_operator(void)
{
    char resp[AT_RESP_SIZE];
    const char *line;
    const char *p;

    if (cellular_at_cmd("AT+COPS=3,0;+COPS?", resp, sizeof(resp)) != 0) {
        printf("Operator: query failed\n");
        return;
    }
    line = cellular_extract_line(resp, "+COPS:");
    if (line) {
        printf("Operator (name): %s\n", line);
        p = strchr(line, ',');
        if (p) {
            p = strchr(p + 1, ',');
            if (p) {
                p = strchr(p + 1, ',');
                if (p && p[1] == '"')
                    printf("  -> %s\n", p + 1);
            }
        }
    }

    if (cellular_at_cmd("AT+COPS=3,2;+COPS?", resp, sizeof(resp)) != 0)
        return;
    line = cellular_extract_line(resp, "+COPS:");
    if (line)
        printf("Operator (code): %s\n", line);
}

static void cellular_print_csq(void)
{
    char resp[AT_RESP_SIZE];
    const char *line;

    if (cellular_at_cmd("AT+CSQ", resp, sizeof(resp)) != 0) {
        printf("Signal CSQ: query failed\n");
        return;
    }
    line = cellular_extract_line(resp, "+CSQ:");
    if (line)
        printf("Signal CSQ: %s\n", line);
    else
        printf("Signal CSQ:\n%s\n", resp);
}

static void cellular_print_netmode(void)
{
    char resp[AT_RESP_SIZE];
    const char *line;
    const char *comma;
    char code[8];

    if (cellular_at_cmd("AT+CNSMOD?", resp, sizeof(resp)) != 0) {
        printf("Network mode: query failed\n");
        return;
    }
    line = cellular_extract_line(resp, "+CNSMOD:");
    if (!line) {
        printf("Network mode:\n%s\n", resp);
        return;
    }
    printf("Network mode: %s\n", line);

    comma = strchr(line, ',');
    if (!comma)
        return;
    comma = strchr(comma + 1, ',');
    if (!comma)
        return;
    snprintf(code, sizeof(code), "%s", comma + 1);
    code[strcspn(code, "\r\n ")] = '\0';
    printf("  -> %s (%s)\n", cellular_netmode_name(code), code);
}

static void cellular_print_reg_line(const char *title, const char *cmd,
                                    const char *prefix)
{
    char resp[AT_RESP_SIZE];
    const char *line;
    int stat = -1;

    if (cellular_at_cmd(cmd, resp, sizeof(resp)) != 0) {
        printf("%s: query failed\n", title);
        return;
    }
    line = cellular_extract_line(resp, prefix);
    if (!line) {
        printf("%s:\n%s\n", title, resp);
        return;
    }
    if (sscanf(line, "+CEREG: %*d,%d", &stat) == 1 ||
        sscanf(line, "+CGREG: %*d,%d", &stat) == 1) {
        const char *desc = "unknown";

        switch (stat) {
        case 0: desc = "not registered"; break;
        case 1: desc = "registered (home)"; break;
        case 2: desc = "searching"; break;
        case 3: desc = "denied"; break;
        case 5: desc = "registered (roaming)"; break;
        }
        printf("%s: %s  [%s]\n", title, line, desc);
    } else {
        printf("%s: %s\n", title, line);
    }
}

static void cellular_print_registration(void)
{
    cellular_print_reg_line("LTE registration (AT+CEREG?)",
                            "AT+CEREG?", "+CEREG:");
    cellular_print_reg_line("PS registration (AT+CGREG?)",
                            "AT+CGREG?", "+CGREG:");
}

static void cellular_print_dial_status(void)
{
    char resp[AT_RESP_SIZE];
    const char *line;

    if (cellular_at_cmd("AT$QCRMCALL?", resp, sizeof(resp)) != 0) {
        printf("Dial-up status: query failed\n");
        return;
    }
    line = cellular_extract_line(resp, "$QCRMCALL:");
    if (!line)
        line = cellular_extract_line(resp, "+$QCRMCALL:");
    if (line)
        printf("Dial-up status: %s\n", line);
    else
        printf("Dial-up status:\n%s\n", resp);
}

static void cellular_print_cell_info(void)
{
    char resp[AT_RESP_SIZE];
    const char *line;

    if (cellular_at_cmd("AT+CPSI?", resp, sizeof(resp)) != 0) {
        printf("Cell info: query failed\n");
        return;
    }
    line = cellular_extract_line(resp, "+CPSI:");
    if (line)
        printf("Cell info: %s\n", line);
    else
        printf("Cell info:\n%s\n", resp);
}

static int cellular_get_netmode_code(char *code, size_t len)
{
    char resp[AT_RESP_SIZE];
    const char *line;
    const char *comma;

    if (!code || len == 0)
        return -1;

    code[0] = '\0';
    if (cellular_at_cmd("AT+CNSMOD?", resp, sizeof(resp)) != 0)
        return -1;

    line = cellular_extract_line(resp, "+CNSMOD:");
    if (!line)
        return -1;

    comma = strchr(line, ',');
    if (!comma)
        return -1;
    comma = strchr(comma + 1, ',');
    if (!comma)
        return -1;

    snprintf(code, len, "%s", comma + 1);
    code[strcspn(code, "\r\n")] = '\0';
    return 0;
}

static int cellular_dial_try(const char *at_cmd)
{
    char resp[AT_RESP_SIZE];

    printf("Trying: %s\n", at_cmd);
    if (cellular_at_cmd(at_cmd, resp, sizeof(resp)) != 0) {
        printf("Failed: %s\n", resp);
        return -1;
    }
    return 0;
}

static int cellular_connect_with_apn(const char *apn, const char *user,
                                     const char *pass)
{
    char cmd[256];
    char net_code[8];
    int i;

    if (cellular_at_ok("AT+CPIN?") != 0) {
        printf("SIM not ready. Check card and antenna.\n");
        return -1;
    }

    if (cellular_get_netmode_code(net_code, sizeof(net_code)) == 0)
        printf("Network mode code: %s\n", net_code);

    if (!apn || apn[0] == '\0') {
        if (cellular_dial_try("AT$QCRMCALL=1,1") == 0)
            goto dial_ok;
    } else {
        snprintf(cmd, sizeof(cmd),
                 "AT$QCRMCALL=1,1,,,,,\"%s\",\"%s\",\"%s\",3",
                 apn, user ? user : "none", pass ? pass : "none");
        if (cellular_dial_try(cmd) == 0)
            goto dial_ok;
    }

    if (net_code[0] &&
        (!strcmp(net_code, "9") || !strcmp(net_code, "13") ||
         !strcmp(net_code, "14") || !strcmp(net_code, "15") ||
         !strcmp(net_code, "24"))) {
        const char *cdma_cmds[] = {
            "AT$QCRMCALL=1,1,,,,,,\"\",\"\"",
            "AT$QCRMCALL=1,1,,,,,,\"ctnet@mycdma.cn\",\"vnet.mobi\""
        };

        for (i = 0; i < 2; i++) {
            if (cellular_dial_try(cdma_cmds[i]) == 0)
                goto dial_ok;
            usleep(500000);
        }
    }

    printf("Dial-up failed.\n");
    return -1;

dial_ok:
    cellular_if_init();
    cellular_if_dhcp();
    printf("Dial-up OK. DHCP started on %s.\n", IEPRO_CELL_IFACE);
    cellular_show_iface_ip();
    return 0;
}

static int cellular_pick_apn_profile(char *apn, size_t apn_len,
                                     char *user, size_t user_len,
                                     char *pass, size_t pass_len)
{
    int profile;

    printf("\nAPN profile:\n");
    printf(" 1) Auto (3GPP, no APN)\n");
    printf(" 2) Custom APN\n");
    printf(" 0) Cancel\n");

    profile = menu_read_choice("Select profile: ");
    if (profile == MENU_BACK)
        return -1;

    apn[0] = user[0] = pass[0] = '\0';
    switch (profile) {
    case 1:
        return 0;
    case 2:
        if (menu_read_line("APN: ", apn, apn_len) < 0)
            return -1;
        if (menu_read_line("User (optional): ", user, user_len) < 0)
            return -1;
        if (menu_read_line("Password (optional): ", pass, pass_len) < 0)
            return -1;
        return apn[0] ? 0 : -1;
    default:
        return -1;
    }
}

static void cellular_disconnect(void)
{
    if (cellular_at_ok("AT$QCRMCALL=0,1") == 0) {
        cellular_if_init();
        printf("Data call disconnected.\n");
    } else {
        printf("Failed to disconnect data call.\n");
    }
}

static void cellular_ping_test(void)
{
    char cmd[128];
    int rc;

    snprintf(cmd, sizeof(cmd), "ping -c 4 -I %s 8.8.8.8", IEPRO_CELL_IFACE);
    printf("Running: %s\n", cmd);
    rc = system(cmd);
    if (rc != 0)
        printf("Ping test finished with status %d.\n", rc);
}

static void cellular_show_menu(void)
{
    printf("\n--- Cellular module (SIM7600G-H-PCIE) ---\n");
    printf(" 1) Module version (ATI)\n");
    printf(" 2) Firmware version (AT+CGMR)\n");
    printf(" 3) IMEI (AT+CGSN)\n");
    printf(" 4) ICCID (AT+CCID)\n");
    printf(" 5) IMSI (AT+CIMI)\n");
    printf(" 6) SIM status (AT+CPIN?)\n");
    printf(" 7) Signal CSQ (AT+CSQ)\n");
    printf(" 8) Operator (AT+COPS?)\n");
    printf(" 9) Network mode (AT+CNSMOD?)\n");
    printf("10) Registration status (AT+CEREG? / AT+CGREG?)\n");
    printf("11) Dial-up status (AT$QCRMCALL?)\n");
    printf("12) Cell info (AT+CPSI?)\n");
    printf("13) Connect (NDIS dial-up)\n");
    printf("14) Disconnect\n");
    printf("15) Renew DHCP on %s\n", IEPRO_CELL_IFACE);
    printf("16) Ping test (8.8.8.8 via %s)\n", IEPRO_CELL_IFACE);
    printf("17) AT command help\n");
    printf("18) Send custom AT command\n");
    printf(" 0) Back to main menu (Ctrl+C)\n");
}

int cellular_module_menu(void)
{
    for (;;) {
        int choice;

        cellular_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK) {
            cellular_at_close();
            return 0;
        }

        switch (choice) {
        case 1:
            cellular_print_version();
            break;
        case 2:
            cellular_print_firmware();
            break;
        case 3:
            cellular_print_imei();
            break;
        case 4:
            cellular_print_iccid();
            break;
        case 5:
            cellular_print_imsi();
            break;
        case 6:
            cellular_print_sim_status();
            break;
        case 7:
            cellular_print_csq();
            break;
        case 8:
            cellular_print_operator();
            break;
        case 9:
            cellular_print_netmode();
            break;
        case 10:
            cellular_print_registration();
            break;
        case 11:
            cellular_print_dial_status();
            break;
        case 12:
            cellular_print_cell_info();
            break;
        case 13: {
            char apn[64];
            char user[64];
            char pass[64];

            if (cellular_pick_apn_profile(apn, sizeof(apn),
                                          user, sizeof(user),
                                          pass, sizeof(pass)) == 0)
                cellular_connect_with_apn(apn, user, pass);
            break;
        }
        case 14:
            cellular_disconnect();
            break;
        case 15:
            cellular_if_dhcp();
            cellular_show_iface_ip();
            break;
        case 16:
            cellular_ping_test();
            break;
        case 17:
            cellular_print_help();
            break;
        case 18:
            cellular_at_console();
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}

void cellular_module_cli_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s cellular <action> [options]\n"
            "  version        Module version ATI (menu 1)\n"
            "  firmware       Firmware AT+CGMR (menu 2)\n"
            "  imei           IMEI (menu 3)\n"
            "  iccid          ICCID (menu 4)\n"
            "  imsi           IMSI (menu 5)\n"
            "  sim            SIM status (menu 6)\n"
            "  csq            Signal CSQ (menu 7)\n"
            "  operator       Operator (menu 8)\n"
            "  netmode        Network mode (menu 9)\n"
            "  reg            Registration (menu 10)\n"
            "  dial-status    Dial-up status (menu 11)\n"
            "  cell           Cell info (menu 12)\n"
            "  connect        NDIS dial-up (menu 13)\n"
            "  disconnect     Stop data call (menu 14)\n"
            "  dhcp           Renew DHCP (menu 15)\n"
            "  ping           Ping 8.8.8.8 via %s (menu 16)\n"
            "  help           AT command reference (menu 17)\n"
            "  at             Send custom AT command (menu 18)\n"
            "  --cmd STR      AT command for at action\n"
            "  --apn APN      Custom APN for connect\n"
            "  --user U       APN username (optional)\n"
            "  --pass P       APN password (optional)\n"
            "  --auto         Connect with auto APN (no --apn)\n"
            "\n"
            "Examples:\n"
            "  Query module / SIM / signal:\n"
            "    %s cellular version\n"
            "    %s cellular imei\n"
            "    %s cellular csq\n"
            "    %s cellular sim\n"
            "  Custom AT command:\n"
            "    %s cellular at --cmd +CSQ\n"
            "  Dial-up (requires SIM and antenna):\n"
            "    %s cellular connect --auto\n"
            "    %s cellular connect --apn cmnet --user user --pass pass\n"
            "  After dial-up:\n"
            "    %s cellular dhcp\n"
            "    %s cellular ping\n"
            "    %s cellular disconnect\n",
            prog, IEPRO_CELL_IFACE,
            prog, prog, prog, prog, prog, prog, prog, prog, prog, prog);
}

int cellular_module_cli(int argc, char **argv)
{
    const char *action = argv[1];
    const char *at_cmd = NULL;
    const char *apn = NULL;
    const char *user = NULL;
    const char *pass = NULL;
    int auto_apn = 0;
    int opt;
    int rc = CLI_EXIT_OK;

    static const struct option opts[] = {
        { "cmd", required_argument, NULL, 'c' },
        { "apn", required_argument, NULL, 'a' },
        { "user", required_argument, NULL, 'u' },
        { "pass", required_argument, NULL, 'p' },
        { "auto", no_argument, NULL, 'A' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    if (!action || !strcmp(action, "-h") || !strcmp(action, "--help")) {
        cellular_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    optind = 2;
    while ((opt = getopt_long(argc, argv, "c:a:u:p:Ah", opts, NULL)) != -1) {
        switch (opt) {
        case 'c':
            at_cmd = optarg;
            break;
        case 'a':
            apn = optarg;
            break;
        case 'u':
            user = optarg;
            break;
        case 'p':
            pass = optarg;
            break;
        case 'A':
            auto_apn = 1;
            break;
        case 'h':
            cellular_module_cli_usage(argv[0]);
            return CLI_EXIT_OK;
        default:
            cellular_module_cli_usage(argv[0]);
            return CLI_EXIT_USAGE;
        }
    }

    if (!strcmp(action, "version"))
        cellular_print_version();
    else if (!strcmp(action, "firmware"))
        cellular_print_firmware();
    else if (!strcmp(action, "imei"))
        cellular_print_imei();
    else if (!strcmp(action, "iccid"))
        cellular_print_iccid();
    else if (!strcmp(action, "imsi"))
        cellular_print_imsi();
    else if (!strcmp(action, "sim"))
        cellular_print_sim_status();
    else if (!strcmp(action, "csq"))
        cellular_print_csq();
    else if (!strcmp(action, "operator"))
        cellular_print_operator();
    else if (!strcmp(action, "netmode"))
        cellular_print_netmode();
    else if (!strcmp(action, "reg"))
        cellular_print_registration();
    else if (!strcmp(action, "dial-status"))
        cellular_print_dial_status();
    else if (!strcmp(action, "cell"))
        cellular_print_cell_info();
    else if (!strcmp(action, "connect")) {
        char apn_buf[64];
        char user_buf[64];
        char pass_buf[64];

        apn_buf[0] = user_buf[0] = pass_buf[0] = '\0';
        if (apn)
            snprintf(apn_buf, sizeof(apn_buf), "%s", apn);
        if (user)
            snprintf(user_buf, sizeof(user_buf), "%s", user);
        if (pass)
            snprintf(pass_buf, sizeof(pass_buf), "%s", pass);

        if (!auto_apn && !apn) {
            fprintf(stderr, "connect requires --auto or --apn.\n");
            return CLI_EXIT_USAGE;
        }

        if (cellular_connect_with_apn(apn_buf, user_buf, pass_buf) < 0)
            rc = CLI_EXIT_FAIL;
    } else if (!strcmp(action, "disconnect")) {
        cellular_disconnect();
    } else if (!strcmp(action, "dhcp")) {
        cellular_if_dhcp();
        cellular_show_iface_ip();
    } else if (!strcmp(action, "ping")) {
        cellular_ping_test();
    } else if (!strcmp(action, "help")) {
        cellular_print_help();
    } else if (!strcmp(action, "at")) {
        char cmd[280];

        if (!at_cmd || at_cmd[0] == '\0') {
            fprintf(stderr, "at requires --cmd.\n");
            return CLI_EXIT_USAGE;
        }

        if ((at_cmd[0] == 'A' || at_cmd[0] == 'a') &&
            (at_cmd[1] == 'T' || at_cmd[1] == 't'))
            snprintf(cmd, sizeof(cmd), "%s", at_cmd);
        else
            snprintf(cmd, sizeof(cmd), "AT%s", at_cmd);

        cellular_at_send_user(cmd);
    } else {
        fprintf(stderr, "Unknown cellular action: %s\n", action);
        cellular_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    cellular_at_close();
    return rc;
}
