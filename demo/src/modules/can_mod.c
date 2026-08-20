#include "demo.h"
#include "iepro_hw.h"
#include "menu_util.h"
#include "cli_util.h"

#include <getopt.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

static int can_open_socket(const char *iface)
{
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("can socket");
        return -1;
    }

    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("can SIOCGIFINDEX");
        close(s);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("can bind");
        close(s);
        return -1;
    }
    return s;
}

static int can_bring_up(int bitrate)
{
    char cmd[128];

    snprintf(cmd, sizeof(cmd),
             "ip link set %s down 2>/dev/null; "
             "ip link set %s type can bitrate %d; "
             "ip link set %s up",
             IEPRO_CAN_IFACE, IEPRO_CAN_IFACE, bitrate, IEPRO_CAN_IFACE);
    printf("Running: %s\n", cmd);
    if (system(cmd) != 0) {
        printf("Failed to bring up %s (check wiring and bitrate).\n",
               IEPRO_CAN_IFACE);
        return -1;
    }
    printf("%s is up at %d bps.\n", IEPRO_CAN_IFACE, bitrate);
    return 0;
}

static void can_listen_once(void)
{
    int s = can_open_socket(IEPRO_CAN_IFACE);
    struct can_frame frame;
    fd_set rfds;
    struct timeval tv;
    int ret;

    if (s < 0)
        return;

    FD_ZERO(&rfds);
    FD_SET(s, &rfds);
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    printf("Waiting for CAN frame on %s (3s)...\n", IEPRO_CAN_IFACE);
    ret = select(s + 1, &rfds, NULL, NULL, &tv);
    if (ret > 0) {
        int nbytes = read(s, &frame, sizeof(frame));
        int i;

        if (nbytes > 0) {
            printf("CAN ID: 0x%X, DLC: %d, data:",
                   frame.can_id & CAN_EFF_MASK, frame.can_dlc);
            for (i = 0; i < frame.can_dlc; i++)
                printf(" %02X", frame.data[i]);
            printf("\n");
        }
    } else if (ret == 0) {
        printf("Timeout — no frame received.\n");
    } else {
        perror("can select");
    }
    close(s);
}

static void can_send_test_frame(void)
{
    int s = can_open_socket(IEPRO_CAN_IFACE);
    struct can_frame frame;

    if (s < 0)
        return;

    memset(&frame, 0, sizeof(frame));
    frame.can_id = 0x123;
    frame.can_dlc = 8;
    frame.data[0] = 0xDE;
    frame.data[1] = 0xAD;
    frame.data[2] = 0xBE;
    frame.data[3] = 0xEF;
    frame.data[4] = 0x00;
    frame.data[5] = 0x11;
    frame.data[6] = 0x22;
    frame.data[7] = 0x33;

    if (write(s, &frame, sizeof(frame)) < 0)
        perror("can write");
    else
        printf("Sent test frame ID=0x123 on %s\n", IEPRO_CAN_IFACE);
    close(s);
}

static void can_show_menu(void)
{
    printf("\n--- CAN module ---\n");
    printf(" 1) Bring up %s (default %d bps)\n",
           IEPRO_CAN_IFACE, IEPRO_CAN_DEFAULT_BITRATE);
    printf(" 2) Listen for one frame (3s timeout)\n");
    printf(" 3) Send test frame (ID 0x123)\n");
    printf(" 0) Back to main menu (Ctrl+C)\n");
}

int can_module_menu(void)
{
    for (;;) {
        int choice;
        int bitrate;

        can_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK)
            return 0;

        switch (choice) {
        case 1:
            bitrate = menu_read_int("Bitrate (default 250000): ",
                                    IEPRO_CAN_DEFAULT_BITRATE);
            if (bitrate == MENU_CANCEL)
                break;
            can_bring_up(bitrate);
            break;
        case 2:
            can_listen_once();
            break;
        case 3:
            can_send_test_frame();
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}

void can_module_cli_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s can <up|listen|send> [options]\n"
            "  up             Bring up %s (menu 1)\n"
            "  listen         Wait for one frame, 3s timeout (menu 2)\n"
            "  send           Send test frame ID 0x123 (menu 3)\n"
            "Options:\n"
            "  --bitrate N    Bitrate for up (default %d)\n"
            "\n"
            "Examples:\n"
            "    %s can up --bitrate 250000\n"
            "    %s can listen\n"
            "    %s can send\n",
            prog, IEPRO_CAN_IFACE, IEPRO_CAN_DEFAULT_BITRATE,
            prog, prog, prog);
}

int can_module_cli(int argc, char **argv)
{
    const char *action = argv[1];
    int bitrate = IEPRO_CAN_DEFAULT_BITRATE;
    int opt;

    static const struct option opts[] = {
        { "bitrate", required_argument, NULL, 'b' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    if (!action || !strcmp(action, "-h") || !strcmp(action, "--help")) {
        can_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    optind = 2;
    while ((opt = getopt_long(argc, argv, "b:h", opts, NULL)) != -1) {
        switch (opt) {
        case 'b':
            if (cli_parse_int(optarg, &bitrate) < 0 || bitrate <= 0) {
                fprintf(stderr, "Invalid --bitrate value.\n");
                return CLI_EXIT_USAGE;
            }
            break;
        case 'h':
            can_module_cli_usage(argv[0]);
            return CLI_EXIT_OK;
        default:
            can_module_cli_usage(argv[0]);
            return CLI_EXIT_USAGE;
        }
    }

    if (!strcmp(action, "up")) {
        return can_bring_up(bitrate) == 0 ? CLI_EXIT_OK : CLI_EXIT_FAIL;
    }
    if (!strcmp(action, "listen")) {
        can_listen_once();
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "send")) {
        can_send_test_frame();
        return CLI_EXIT_OK;
    }

    fprintf(stderr, "Unknown can action: %s\n", action);
    can_module_cli_usage(argv[0]);
    return CLI_EXIT_USAGE;
}
