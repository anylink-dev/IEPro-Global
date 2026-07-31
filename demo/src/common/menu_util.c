#include "menu_util.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t g_stop_loop;

static void on_sigint(int sig)
{
    (void)sig;
    g_stop_loop = 1;
}

void menu_init(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}

void menu_reset_stop(void)
{
    g_stop_loop = 0;
}

int menu_stop_requested(void)
{
    return g_stop_loop != 0;
}

static int menu_input_interrupted(void)
{
    if (!menu_stop_requested())
        return 0;
    menu_reset_stop();
    return 1;
}

void menu_pause(void)
{
    /* no-op: return to submenu immediately */
}

int menu_read_choice(const char *prompt)
{
    char line[32];
    int value;
    char *end;

    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
        if (menu_input_interrupted())
            return MENU_BACK;
        return -1;
    }
    if (menu_input_interrupted())
        return MENU_BACK;

    value = (int)strtol(line, &end, 10);
    if (end == line)
        return -1;
    return value;
}

int menu_read_int(const char *prompt, int default_value)
{
    char line[32];
    int value;
    char *end;

    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
        if (menu_input_interrupted())
            return MENU_CANCEL;
        return default_value;
    }
    if (menu_input_interrupted())
        return MENU_CANCEL;

    value = (int)strtol(line, &end, 10);
    if (end == line)
        return default_value;
    return value;
}

int menu_read_line(const char *prompt, char *buf, size_t len)
{
    if (len == 0)
        return -1;

    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, len, stdin)) {
        if (menu_input_interrupted())
            return -1;
        return -1;
    }
    if (menu_input_interrupted())
        return -1;

    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}
