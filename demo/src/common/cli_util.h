#ifndef CLI_UTIL_H
#define CLI_UTIL_H

#define CLI_EXIT_OK     0
#define CLI_EXIT_USAGE  1
#define CLI_EXIT_FAIL   2

void cli_print_main_usage(const char *prog);

/* Returns 1..3 (RS232/RS485-1/RS485-2), or -1 on invalid input. */
int cli_parse_serial_port(const char *s, int default_port);

int cli_parse_bool(const char *s, int default_val);
int cli_parse_int(const char *s, int *out);

int cli_dispatch(int argc, char **argv);

#endif
