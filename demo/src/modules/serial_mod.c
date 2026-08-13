#include "demo.h"
#include "iepro_hw.h"
#include "menu_util.h"

#include <asm/termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef BOTHER
#define BOTHER 0010000
#endif

static void serial_apply_common_flags(struct termios2 *tio)
{
    tio->c_cflag = (tio->c_cflag & ~CSIZE) | CS8;
    tio->c_iflag &= ~IGNBRK;
    tio->c_lflag = 0;
    tio->c_oflag = 0;
    tio->c_cc[VMIN] = 0;
    tio->c_cc[VTIME] = 50;
    tio->c_iflag &= ~(IXON | IXOFF | IXANY);
    tio->c_cflag |= (CLOCAL | CREAD);
    tio->c_cflag &= ~(PARENB | PARODD);
    tio->c_cflag &= ~CSTOPB;
    tio->c_cflag &= ~CRTSCTS;
}

static int serial_baud_needs_custom(int baud)
{
    return baud == 256000;
}

static int serial_set_baud(struct termios2 *tio, int baud)
{
    if (serial_baud_needs_custom(baud)) {
        tio->c_cflag &= ~CBAUD;
        tio->c_cflag |= BOTHER;
        tio->c_ispeed = (speed_t)baud;
        tio->c_ospeed = (speed_t)baud;
        return 0;
    }

    switch (baud) {
    case 600:
        tio->c_cflag &= ~CBAUD;
        tio->c_cflag |= B600;
        break;
    case 9600:
        tio->c_cflag &= ~CBAUD;
        tio->c_cflag |= B9600;
        break;
    case 19200:
        tio->c_cflag &= ~CBAUD;
        tio->c_cflag |= B19200;
        break;
    case 38400:
        tio->c_cflag &= ~CBAUD;
        tio->c_cflag |= B38400;
        break;
    case 57600:
        tio->c_cflag &= ~CBAUD;
        tio->c_cflag |= B57600;
        break;
    case 115200:
        tio->c_cflag &= ~CBAUD;
        tio->c_cflag |= B115200;
        break;
    default:
        fprintf(stderr, "Unsupported baud %d, using 9600.\n", baud);
        tio->c_cflag &= ~CBAUD;
        tio->c_cflag |= B9600;
        break;
    }

    tio->c_ispeed = 0;
    tio->c_ospeed = 0;
    return 0;
}

static int open_serial(const char *dev, int baud)
{
    struct termios2 tio;
    int fd;

    fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open serial");
        return -1;
    }

    memset(&tio, 0, sizeof(tio));
    if (ioctl(fd, TCGETS2, &tio) != 0) {
        perror("TCGETS2");
        close(fd);
        return -1;
    }

    serial_set_baud(&tio, baud);
    serial_apply_common_flags(&tio);

    if (ioctl(fd, TCSETS2, &tio) != 0) {
        perror("TCSETS2");
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
