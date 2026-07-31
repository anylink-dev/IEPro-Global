#ifndef GPIO_UTIL_H
#define GPIO_UTIL_H

#include "iepro_hw.h"

int gpio_export(int gpio);
int gpio_set_direction(int gpio, const char *dir);
int gpio_read_value(int gpio);
int gpio_write_value(int gpio, int value);
int gpio_init_input(int gpio);
int gpio_init_output(int gpio);

int gpio_init_board_io(void);
int gpio_init_leds(void);
int gpio_read_di(void);
int gpio_read_dip1(void);
int gpio_read_dip2(void);
int gpio_read_reset_btn(void);

#endif
