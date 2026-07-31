#ifndef MENU_UTIL_H
#define MENU_UTIL_H

#include <stddef.h>

#define MENU_BACK    0
#define MENU_CANCEL  (-1)

void menu_init(void);
void menu_reset_stop(void);
int menu_stop_requested(void);

void menu_pause(void);
int menu_read_choice(const char *prompt);
int menu_read_int(const char *prompt, int default_value);
int menu_read_line(const char *prompt, char *buf, size_t len);

#endif
