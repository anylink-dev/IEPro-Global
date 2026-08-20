#include "demo.h"
#include "iepro_hw.h"
#include "menu_util.h"
#include "cli_util.h"

#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

static volatile sig_atomic_t g_wdt_stop;
static volatile sig_atomic_t g_wdt_reboot;

static void wdt_on_sigint(int sig)
{
    (void)sig;
    g_wdt_stop = 1;
}

static void wdt_on_sigusr1(int sig)
{
    (void)sig;
    g_wdt_reboot = 1;
}

static int wdt_write_pid(void)
{
    FILE *fp = fopen(IEPRO_WDT_PID_FILE, "w");

    if (!fp) {
        perror("write watchdog pid file");
        return -1;
    }
    fprintf(fp, "%d\n", (int)getpid());
    fclose(fp);
    return 0;
}

static int wdt_read_pid(pid_t *pid)
{
    FILE *fp = fopen(IEPRO_WDT_PID_FILE, "r");
    long v;

    if (!fp || !pid)
        return -1;

    if (fscanf(fp, "%ld", &v) != 1) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    if (v <= 0)
        return -1;

    *pid = (pid_t)v;
    return 0;
}

static void wdt_remove_pid(void)
{
    unlink(IEPRO_WDT_PID_FILE);
}

static int wdt_configure(int fd, int timeout)
{
    int opt = WDIOS_DISABLECARD;

    if (ioctl(fd, WDIOC_SETOPTIONS, &opt) < 0) {
        perror("WDIOC_SETOPTIONS disable");
        return -1;
    }

    if (ioctl(fd, WDIOC_SETTIMEOUT, &timeout) < 0) {
        perror("WDIOC_SETTIMEOUT");
        return -1;
    }

    opt = WDIOS_ENABLECARD;
    if (ioctl(fd, WDIOC_SETOPTIONS, &opt) < 0) {
        perror("WDIOC_SETOPTIONS enable");
        return -1;
    }

    return 0;
}

static int wdt_keepalive(int fd)
{
    int dummy = 0;

    if (ioctl(fd, WDIOC_KEEPALIVE, &dummy) < 0) {
        perror("WDIOC_KEEPALIVE");
        return -1;
    }
    return 0;
}

static int wdt_magic_close(int fd)
{
    if (write(fd, "V", 1) < 0)
        perror("watchdog magic close");
    if (close(fd) < 0)
        perror("close watchdog");
    return 0;
}

static int wdt_install_handlers(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = wdt_on_sigint;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction SIGINT");
        return -1;
    }

    sa.sa_handler = wdt_on_sigusr1;
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction SIGUSR1");
        return -1;
    }

    return 0;
}

static int wdt_run(int timeout)
{
    int fd;
    int feed_interval;
    int reboot_timeout = 1;

    if (timeout <= 0)
        timeout = IEPRO_WDT_DEFAULT_TIMEOUT;

    fd = open(IEPRO_WDT_DEV, O_RDWR);
    if (fd < 0) {
        perror("open watchdog");
        return -1;
    }

    if (wdt_install_handlers() < 0) {
        close(fd);
        return -1;
    }

    if (wdt_configure(fd, timeout) < 0) {
        close(fd);
        return -1;
    }

    if (wdt_write_pid() < 0) {
        close(fd);
        return -1;
    }

    feed_interval = timeout / 3;
    if (feed_interval < 1)
        feed_interval = 1;

    printf("Watchdog configured with %d seconds timeout.\n", timeout);
    printf("Feed interval: %d s. Keep this process running.\n", feed_interval);
    printf("Stop: Ctrl+C or `iepro_demo watchdog stop`\n");
    printf("Reboot: `iepro_demo watchdog reboot`\n");

    g_wdt_stop = 0;
    g_wdt_reboot = 0;
    menu_reset_stop();

    while (!g_wdt_stop && !g_wdt_reboot && !menu_stop_requested()) {
        if (wdt_keepalive(fd) < 0)
            break;
        sleep((unsigned int)feed_interval);
    }

    if (g_wdt_reboot) {
        printf("SIGUSR1 received: setting watchdog timeout to %d s and stopping feed.\n",
               reboot_timeout);
        if (ioctl(fd, WDIOC_SETTIMEOUT, &reboot_timeout) < 0)
            perror("WDIOC_SETTIMEOUT reboot");
        wdt_remove_pid();
        while (1)
            sleep(1);
    }

    wdt_magic_close(fd);
    wdt_remove_pid();
    menu_reset_stop();
    printf("Watchdog stopped gracefully.\n");
    return 0;
}

static int wdt_signal_feeder(int sig)
{
    pid_t pid;

    if (wdt_read_pid(&pid) < 0) {
        fprintf(stderr, "No watchdog feeder running (missing %s).\n",
                IEPRO_WDT_PID_FILE);
        return -1;
    }

    if (kill(pid, sig) < 0) {
        perror("kill watchdog feeder");
        wdt_remove_pid();
        return -1;
    }

    printf("Signal %d sent to watchdog feeder (PID %d).\n", sig, (int)pid);
    return 0;
}

static void wdt_show_menu(void)
{
    printf("\n--- Watchdog (%s) ---\n", IEPRO_WDT_DEV);
    printf(" 1) Start keepalive (foreground)\n");
    printf(" 2) Stop running feeder\n");
    printf(" 3) Reboot via watchdog timeout\n");
    printf(" 0) Back\n");
}

int watchdog_module_menu(void)
{
    for (;;) {
        int choice;
        int timeout;

        wdt_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK)
            return 0;

        switch (choice) {
        case 1:
            timeout = menu_read_int("Timeout in seconds (default 60): ",
                                    IEPRO_WDT_DEFAULT_TIMEOUT);
            if (timeout == MENU_CANCEL)
                break;
            if (timeout <= 0)
                timeout = IEPRO_WDT_DEFAULT_TIMEOUT;
            wdt_run(timeout);
            break;
        case 2:
            wdt_signal_feeder(SIGINT);
            menu_pause();
            break;
        case 3:
            wdt_signal_feeder(SIGUSR1);
            menu_pause();
            break;
        default:
            printf("Invalid choice.\n");
            menu_pause();
            break;
        }
    }
}

void watchdog_module_cli_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s watchdog <action> [options]\n"
            "  start          Open %s, enable card, and feed until Ctrl+C\n"
            "  stop           Signal a running feeder to stop gracefully\n"
            "  reboot         Signal a running feeder to trigger reboot\n"
            "Options:\n"
            "  --timeout N    Watchdog timeout in seconds (default %d)\n"
            "\n"
            "Examples:\n"
            "    %s watchdog start\n"
            "    %s watchdog start --timeout 120\n"
            "    %s watchdog stop\n"
            "    %s watchdog reboot\n",
            prog, IEPRO_WDT_DEV, IEPRO_WDT_DEFAULT_TIMEOUT,
            prog, prog, prog, prog);
}

int watchdog_module_cli(int argc, char **argv)
{
    const char *action = argv[1];
    int timeout = IEPRO_WDT_DEFAULT_TIMEOUT;
    int opt;

    static const struct option opts[] = {
        { "timeout", required_argument, NULL, 't' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    if (!action || !strcmp(action, "-h") || !strcmp(action, "--help")) {
        watchdog_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    optind = 2;
    while ((opt = getopt_long(argc, argv, "t:h", opts, NULL)) != -1) {
        switch (opt) {
        case 't':
            if (cli_parse_int(optarg, &timeout) < 0 || timeout <= 0) {
                fprintf(stderr, "Invalid --timeout value.\n");
                return CLI_EXIT_USAGE;
            }
            break;
        case 'h':
            watchdog_module_cli_usage(argv[0]);
            return CLI_EXIT_OK;
        default:
            watchdog_module_cli_usage(argv[0]);
            return CLI_EXIT_USAGE;
        }
    }

    if (!strcmp(action, "start"))
        return wdt_run(timeout) == 0 ? CLI_EXIT_OK : CLI_EXIT_FAIL;
    if (!strcmp(action, "stop"))
        return wdt_signal_feeder(SIGINT) == 0 ? CLI_EXIT_OK : CLI_EXIT_FAIL;
    if (!strcmp(action, "reboot"))
        return wdt_signal_feeder(SIGUSR1) == 0 ? CLI_EXIT_OK : CLI_EXIT_FAIL;

    fprintf(stderr, "Unknown watchdog action: %s\n", action);
    watchdog_module_cli_usage(argv[0]);
    return CLI_EXIT_USAGE;
}
