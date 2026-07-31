#include "demo.h"
#include "iepro_hw.h"
#include "menu_util.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static speed_t baud_to_flag(int baud)
{
    switch (baud) {
    case 600: return B600;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 256000: return B230400;
    default: return B9600;
    }
}

static int open_serial(const char *dev, int baud)
{
    int fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
    struct termios tty;

    if (fd < 0) {
        perror("open serial");
        return -1;
    }

    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, baud_to_flag(baud));
    cfsetispeed(&tty, baud_to_flag(baud));
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 50;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }
    return fd;
}

static const char *serial_device_for_choice(int choice)
{
    switch (choice) {
    case 1: return IEPRO_DEV_RS232_1;
    case 2: return IEPRO_DEV_RS485_1;
    case 3: return IEPRO_DEV_RS485_2;
    default: return NULL;
    }
}

static int serial_pick_port(void)
{
    int port = menu_read_int("Port [1=RS232 / 2=RS485-1 / 3=RS485-2] (default 1): ", 1);
    if (port == MENU_CANCEL)
        return MENU_CANCEL;
    if (port < 1 || port > 3) {
        printf("Invalid port, using RS232-1.\n");
        return 1;
    }
    return port;
}

static int serial_pick_baud(void)
{
    int baud = menu_read_int("Baud rate (default 9600): ", 9600);
    if (baud == MENU_CANCEL)
        return MENU_CANCEL;
    if (baud <= 0)
        return 9600;
    return baud;
}

static int serial_pick_port_baud(const char **dev, int *baud)
{
    int port;

    port = serial_pick_port();
    if (port == MENU_CANCEL)
        return MENU_CANCEL;
    *baud = serial_pick_baud();
    if (*baud == MENU_CANCEL)
        return MENU_CANCEL;
    *dev = serial_device_for_choice(port);
    return 0;
}

static void serial_receive_loop(const char *dev, int baud)
{
    unsigned char buf[256];
    int fd = open_serial(dev, baud);
    int n;

    if (fd < 0)
        return;

    menu_reset_stop();
    printf("Receiving from %s (%d baud) — press Ctrl+C to stop.\n", dev, baud);
    while (!menu_stop_requested()) {
        n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("Received (%d bytes): %s\n", n, buf);
            fflush(stdout);
        } else if (n < 0) {
            perror("serial read");
            break;
        }
    }
    close(fd);
    menu_reset_stop();
    printf("\nReceive loop stopped.\n");
}

static void serial_send_loop(const char *dev, int baud)
{
    char text[256];
    int fd;
    size_t len;

    if (menu_read_line("Text to send (repeated): ", text, sizeof(text)) < 0)
        return;
    if (text[0] == '\0') {
        printf("Empty input, cancelled.\n");
        return;
    }

    fd = open_serial(dev, baud);
    if (fd < 0)
        return;

    len = strlen(text);
    menu_reset_stop();
    printf("Sending to %s (%d baud) — press Ctrl+C to stop.\n", dev, baud);
    while (!menu_stop_requested()) {
        if (write(fd, text, len) < 0) {
            perror("serial write");
            break;
        }
        printf("Sent %zu bytes\n", len);
        fflush(stdout);
        sleep(1);
    }
    close(fd);
    menu_reset_stop();
    printf("\nSend loop stopped.\n");
}

static void serial_echo_loop(const char *dev, int baud)
{
    unsigned char buf[256];
    int fd = open_serial(dev, baud);
    int n;

    if (fd < 0)
        return;

    menu_reset_stop();
    printf("Echo on %s (%d baud) — press Ctrl+C to stop.\n", dev, baud);
    while (!menu_stop_requested()) {
        n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            if (write(fd, buf, (size_t)n) < 0) {
                perror("serial write");
                break;
            }
            buf[n] = '\0';
            printf("Echoed (%d bytes): %s\n", n, (char *)buf);
            fflush(stdout);
        } else if (n < 0) {
            perror("serial read");
            break;
        }
    }
    close(fd);
    menu_reset_stop();
    printf("\nEcho loop stopped.\n");
}

static void serial_show_menu(void)
{
    printf("\n--- Serial module ---\n");
    printf(" 1) Loop receive (Ctrl+C to stop)\n");
    printf(" 2) Loop send (Ctrl+C to stop)\n");
    printf(" 3) Loop echo (receive & reply, Ctrl+C to stop)\n");
    printf(" 0) Back to main menu (Ctrl+C)\n");
}

int serial_module_menu(void)
{
    for (;;) {
        int choice;
        int baud;
        const char *dev;

        serial_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK)
            return 0;

        switch (choice) {
        case 1:
            if (serial_pick_port_baud(&dev, &baud) == 0 && dev)
                serial_receive_loop(dev, baud);
            break;
        case 2:
            if (serial_pick_port_baud(&dev, &baud) == 0 && dev)
                serial_send_loop(dev, baud);
            break;
        case 3:
            if (serial_pick_port_baud(&dev, &baud) == 0 && dev)
                serial_echo_loop(dev, baud);
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}
