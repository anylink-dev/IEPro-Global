#ifndef IEPRO_DEMO_H

#define IEPRO_DEMO_H



int cli_dispatch(int argc, char **argv);



int serial_module_menu(void);

int serial_module_cli(int argc, char **argv);

void serial_module_cli_usage(const char *prog);



int gpio_module_menu(void);

int gpio_module_cli(int argc, char **argv);

void gpio_module_cli_usage(const char *prog);



int can_module_menu(void);

int can_module_cli(int argc, char **argv);

void can_module_cli_usage(const char *prog);



int cellular_module_menu(void);

int cellular_module_cli(int argc, char **argv);

void cellular_module_cli_usage(const char *prog);



int mqtt_module_menu(void);

int mqtt_module_cli(int argc, char **argv);

void mqtt_module_cli_usage(const char *prog);



int http_module_menu(void);

int http_module_cli(int argc, char **argv);

void http_module_cli_usage(const char *prog);



int modbus_module_menu(void);

int modbus_module_cli(int argc, char **argv);

void modbus_module_cli_usage(const char *prog);



int watchdog_module_menu(void);

int watchdog_module_cli(int argc, char **argv);

void watchdog_module_cli_usage(const char *prog);



#endif

