#include "demo.h"
#include "gpio_util.h"
#include "menu_util.h"

#include <stdio.h>

#define DEMO_VERSION "1.0.0"

static void show_main_menu(void)
{
    printf("\n========================================\n");
    printf("  IE Pro 400 Global Standard — Demo\n");
    printf("  Version %s\n", DEMO_VERSION);
    printf("========================================\n");
    printf(" 1) Serial  (RS232 / RS485)\n");
    printf(" 2) CAN     (SocketCAN)\n");
    printf(" 3) GPIO    (DI / DO / DIP / LED / Reset button)\n");
    printf(" 4) MQTT    (northbound publish)\n");
    printf(" 5) Cellular (SIM7600G-H-PCIE 4G)\n");
    printf(" 6) HTTP    (GET / POST test)\n");
    printf(" 7) Modbus  (RTU / TCP, master / slave)\n");
    printf(" 8) Watchdog (hardware /dev/watchdog)\n");
    printf(" 0) Exit\n");
}

int main(int argc, char **argv)
{
    menu_init();

    if (gpio_init_board_io() < 0)
        printf("Note: GPIO init failed at startup (may need root).\n");

    if (argc > 1)
        return cli_dispatch(argc, argv);

    printf("IE Pro demo console — select a module from the menu.\n");
    printf("Press 0 or Ctrl+C at a menu to go back; Ctrl+C during a loop stops it.\n");

    for (;;) {
        int choice;

        show_main_menu();
        choice = menu_read_choice("Select module: ");
        if (choice == MENU_BACK) {
            printf("Bye.\n");
            return 0;
        }

        switch (choice) {
        case 1:
            serial_module_menu();
            break;
        case 2:
            can_module_menu();
            break;
        case 3:
            gpio_module_menu();
            break;
        case 4:
            mqtt_module_menu();
            break;
        case 5:
            cellular_module_menu();
            break;
        case 6:
            http_module_menu();
            break;
        case 7:
            modbus_module_menu();
            break;
        case 8:
            watchdog_module_menu();
            break;
        default:
            printf("Invalid choice.\n");
            menu_pause();
            break;
        }
    }
}
