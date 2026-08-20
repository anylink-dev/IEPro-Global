#include "demo.h"
#include "iepro_hw.h"
#include "menu_util.h"
#include "serial_port.h"
#include "cli_util.h"

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <asm/termbits.h>

#ifndef TCGETS2
#define TCGETS2 _IOR('T', 0x2A, struct termios2)
#endif
#ifndef TCSETS2
#define TCSETS2 _IOW('T', 0x2B, struct termios2)
#endif

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

    port = iepro_serial_pick_port(IEPRO_SERIAL_PORT_RS232);
    if (port == MENU_CANCEL)
        return MENU_CANCEL;
    *baud = serial_pick_baud();
    if (*baud == MENU_CANCEL)
        return MENU_CANCEL;
    *dev = iepro_serial_device_for_port(port);
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

static void serial_send_loop(const char *dev, int baud, const char *preset_text)
{
    char text[256];
    int fd;
    size_t len;

    if (preset_text) {
        snprintf(text, sizeof(text), "%s", preset_text);
    } else {
        if (menu_read_line("Text to send (repeated): ", text, sizeof(text)) < 0)
            return;
        if (text[0] == '\0') {
            printf("Empty input, cancelled.\n");
            return;
        }
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
                serial_send_loop(dev, baud, NULL);
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

void serial_module_cli_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s serial <recv|send|echo> [options]\n"
            "  recv           Loop receive until Ctrl+C (menu 1)\n"
            "  send           Loop send until Ctrl+C (menu 2)\n"
            "  echo           Loop echo until Ctrl+C (menu 3)\n"
            "Options:\n"
            "  --port 1|2|3   1=RS232, 2=RS485-1, 3=RS485-2 (default 1)\n"
            "  --baud N       Baud rate (default 9600)\n"
            "  --text STR     Text to send (required for send)\n"
            "\n"
            "Examples:\n"
            "  Receive on RS485-1:\n"
            "    %s serial recv --port 2 --baud 9600\n"
            "  Send on RS232:\n"
            "    %s serial send --port 1 --baud 9600 --text \"hello\"\n"
            "  Echo on RS485-2:\n"
            "    %s serial echo --port 3 --baud 115200\n",
            prog, prog, prog, prog);
}

int serial_module_cli(int argc, char **argv)
{
    const char *action = argv[1];
    const char *dev;
    int port = IEPRO_SERIAL_PORT_RS232;
    int baud = 9600;
    const char *text = NULL;
    int opt;

    static const struct option opts[] = {
        { "port", required_argument, NULL, 'p' },
        { "baud", required_argument, NULL, 'b' },
        { "text", required_argument, NULL, 't' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    if (!action || !strcmp(action, "-h") || !strcmp(action, "--help")) {
        serial_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    optind = 2;
    while ((opt = getopt_long(argc, argv, "p:b:t:h", opts, NULL)) != -1) {
        switch (opt) {
        case 'p':
            port = cli_parse_serial_port(optarg, port);
            if (port < 0) {
                fprintf(stderr, "Invalid --port value.\n");
                return CLI_EXIT_USAGE;
            }
            break;
        case 'b':
            if (cli_parse_int(optarg, &baud) < 0 || baud <= 0) {
                fprintf(stderr, "Invalid --baud value.\n");
                return CLI_EXIT_USAGE;
            }
            break;
        case 't':
            text = optarg;
            break;
        case 'h':
            serial_module_cli_usage(argv[0]);
            return CLI_EXIT_OK;
        default:
            serial_module_cli_usage(argv[0]);
            return CLI_EXIT_USAGE;
        }
    }

    dev = iepro_serial_device_for_port(port);
    if (!dev) {
        fprintf(stderr, "Invalid serial port.\n");
        return CLI_EXIT_FAIL;
    }

    if (!strcmp(action, "recv")) {
        serial_receive_loop(dev, baud);
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "send")) {
        if (!text || text[0] == '\0') {
            fprintf(stderr, "send requires --text.\n");
            return CLI_EXIT_USAGE;
        }
        serial_send_loop(dev, baud, text);
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "echo")) {
        serial_echo_loop(dev, baud);
        return CLI_EXIT_OK;
    }

    fprintf(stderr, "Unknown serial action: %s\n", action);
    serial_module_cli_usage(argv[0]);
    return CLI_EXIT_USAGE;
}
