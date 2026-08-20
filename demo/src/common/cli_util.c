#include "cli_util.h"
#include "demo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void cli_print_main_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s [module action [options]]\n"
            "       %s                         (interactive menu)\n\n"
            "Modules (actions mirror the interactive menus):\n"
            "  serial   recv|send|echo       --port 1|2|3 --baud N [--text STR]\n"
            "  can      up|listen|send       [--bitrate N]\n"
            "  gpio     init|monitor|do-high|do-low|pulse|led\n"
            "           led: on|off|blink [--led 1|2|3|4]\n"
            "  mqtt     connect|publish\n"
            "  cellular version|firmware|imei|iccid|imsi|sim|csq|operator|\n"
            "           netmode|reg|dial-status|cell|connect|disconnect|\n"
            "           dhcp|ping|help|at [--cmd STR] [--apn A] [--user U] [--pass P]\n"
            "  http     get|post [--url U] [--ca PATH] [--body STR]\n"
            "  modbus   run|read|write\n"
            "  watchdog start|stop|reboot [--timeout N]\n"
            "\n"
            "Loop actions run until Ctrl+C.\n"
            "\n"
            "Run \"%s <module>\" without action to see options and examples.\n",
            prog, prog, prog);
}

int cli_parse_serial_port(const char *s, int default_port)
{
    if (!s || s[0] == '\0')
        return default_port;

    if (!strcasecmp(s, "1") || !strcasecmp(s, "rs232"))
        return 1;
    if (!strcasecmp(s, "2") || !strcasecmp(s, "rs485-1") ||
        !strcasecmp(s, "rs485_1"))
        return 2;
    if (!strcasecmp(s, "3") || !strcasecmp(s, "rs485-2") ||
        !strcasecmp(s, "rs485_2"))
        return 3;

    return -1;
}

int cli_parse_bool(const char *s, int default_val)
{
    if (!s || s[0] == '\0')
        return default_val;

    if (!strcasecmp(s, "1") || !strcasecmp(s, "true") ||
        !strcasecmp(s, "yes") || !strcasecmp(s, "on"))
        return 1;
    if (!strcasecmp(s, "0") || !strcasecmp(s, "false") ||
        !strcasecmp(s, "no") || !strcasecmp(s, "off"))
        return 0;

    return default_val;
}

int cli_parse_int(const char *s, int *out)
{
    char *end;
    long v;

    if (!s || s[0] == '\0' || !out)
        return -1;

    v = strtol(s, &end, 10);
    if (end == s)
        return -1;

    *out = (int)v;
    return 0;
}

typedef int (*cli_handler_t)(int argc, char **argv);

struct cli_module {
    const char *name;
    cli_handler_t handler;
    void (*usage)(const char *prog);
};

static const struct cli_module g_modules[] = {
    { "serial",   serial_module_cli,   serial_module_cli_usage },
    { "can",      can_module_cli,      can_module_cli_usage },
    { "gpio",     gpio_module_cli,     gpio_module_cli_usage },
    { "mqtt",     mqtt_module_cli,     mqtt_module_cli_usage },
    { "cellular", cellular_module_cli, cellular_module_cli_usage },
    { "http",     http_module_cli,     http_module_cli_usage },
    { "modbus",   modbus_module_cli,   modbus_module_cli_usage },
    { "watchdog", watchdog_module_cli, watchdog_module_cli_usage },
    { NULL, NULL, NULL }
};

int cli_dispatch(int argc, char **argv)
{
    size_t i;
    const char *prog = argv[0];

    if (argc < 2) {
        cli_print_main_usage(prog);
        return CLI_EXIT_USAGE;
    }

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") ||
        !strcmp(argv[1], "help")) {
        cli_print_main_usage(prog);
        return CLI_EXIT_OK;
    }

    for (i = 0; g_modules[i].name; i++) {
        if (strcmp(argv[1], g_modules[i].name) != 0)
            continue;

        if (argc < 3) {
            g_modules[i].usage(prog);
            return CLI_EXIT_USAGE;
        }

        return g_modules[i].handler(argc - 1, argv + 1);
    }

    fprintf(stderr, "Unknown module: %s\n", argv[1]);
    cli_print_main_usage(prog);
    return CLI_EXIT_USAGE;
}
