#include "gpio_util.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int gpio_export(int gpio)
{
    char path[64];

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", gpio);
    if (access(path, F_OK) == 0)
        return 0;

    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("gpio export");
        return -1;
    }

    char buf[8];
    snprintf(buf, sizeof(buf), "%d", gpio);
    if (write(fd, buf, strlen(buf)) < 0) {
        perror("gpio export write");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int gpio_set_direction(int gpio, const char *dir)
{
    char path[64];
    int fd;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("gpio direction");
        return -1;
    }
    if (write(fd, dir, strlen(dir)) < 0) {
        perror("gpio direction write");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int gpio_read_value(int gpio)
{
    char path[64];
    char val[4];
    int fd;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("gpio read");
        return -1;
    }
    if (read(fd, val, sizeof(val)) < 0) {
        perror("gpio read value");
        close(fd);
        return -1;
    }
    close(fd);
    return val[0] == '1' ? 1 : 0;
}

int gpio_write_value(int gpio, int value)
{
    char path[64];
    char val[2];
    int fd;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("gpio write");
        return -1;
    }
    val[0] = value ? '1' : '0';
    val[1] = '\0';
    if (write(fd, val, 1) < 0) {
        perror("gpio write value");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int gpio_init_input(int gpio)
{
    if (gpio_export(gpio) < 0)
        return -1;
    return gpio_set_direction(gpio, "in");
}

int gpio_init_output(int gpio)
{
    if (gpio_export(gpio) < 0)
        return -1;
    return gpio_set_direction(gpio, "out");
}

int gpio_init_board_io(void)
{
    int rc = 0;

    if (gpio_init_input(GPIO_DI) < 0)
        rc = -1;
    if (gpio_init_input(GPIO_DIP1) < 0)
        rc = -1;
    if (gpio_init_input(GPIO_DIP2) < 0)
        rc = -1;
    if (gpio_init_input(GPIO_RESET_BTN) < 0)
        rc = -1;
    if (gpio_init_output(GPIO_DO) < 0)
        rc = -1;
    return rc;
}

int gpio_init_leds(void)
{
    int rc = 0;

    if (gpio_init_output(GPIO_LED_NET) < 0)
        rc = -1;
    if (gpio_init_output(GPIO_LED_RUN) < 0)
        rc = -1;
    if (gpio_init_output(GPIO_LED_ALARM) < 0)
        rc = -1;
    return rc;
}

int gpio_read_di(void)
{
    int val = gpio_read_value(GPIO_DI);
    return val < 0 ? 0 : val;
}

int gpio_read_dip1(void)
{
    int val = gpio_read_value(GPIO_DIP1);
    return val < 0 ? 0 : val;
}

int gpio_read_dip2(void)
{
    int val = gpio_read_value(GPIO_DIP2);
    return val < 0 ? 0 : val;
}

int gpio_read_reset_btn(void)
{
    int val = gpio_read_value(GPIO_RESET_BTN);
    return val < 0 ? 0 : val;
}
