#include "demo.h"
#include "gpio_util.h"
#include "menu_util.h"
#include "cli_util.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    const char *name;
    int gpio;
} led_entry_t;

static const led_entry_t g_leds[] = {
    { "NET",   GPIO_LED_NET },
    { "RUN",   GPIO_LED_RUN },
    { "WARN", GPIO_LED_ALARM },
};

#define LED_COUNT ((int)(sizeof(g_leds) / sizeof(g_leds[0])))

static int gpio_pick_led(void)
{
    int pick = menu_read_int(
        "LED [1=NET / 2=RUN / 3=WARN / 4=All] (default 4): ", 4);

    if (pick == MENU_CANCEL)
        return MENU_CANCEL;
    if (pick < 1 || pick > 4) {
        printf("Invalid LED choice, using all.\n");
        return 4;
    }
    return pick;
}

static void gpio_led_set(int pick, int value)
{
    int i;
    int start = 0;
    int end = LED_COUNT;

    if (pick >= 1 && pick <= 3) {
        start = pick - 1;
        end = pick;
    }

    for (i = start; i < end; i++)
        gpio_write_value(g_leds[i].gpio, value);
}

static void gpio_led_set_verbose(int pick, int value)
{
    int i;
    int start = 0;
    int end = LED_COUNT;

    if (pick >= 1 && pick <= 3) {
        start = pick - 1;
        end = pick;
    }

    for (i = start; i < end; i++) {
        if (gpio_write_value(g_leds[i].gpio, value) == 0)
            printf("%s LED (GPIO %d) = %d\n",
                   g_leds[i].name, g_leds[i].gpio, value);
    }
}

static void gpio_led_test_menu(void)
{
    if (gpio_init_leds() < 0)
        printf("Warning: LED GPIO init failed (may need root).\n");

    for (;;) {
        int choice;
        int pick;

        printf("\n--- LED test ---\n");
        printf(" 1) Steady ON   (always on)\n");
        printf(" 2) Steady OFF  (always off)\n");
        printf(" 3) Fast blink  (Ctrl+C to stop)\n");
        printf(" 0) Back\n");

        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK)
            return;

        pick = gpio_pick_led();
        if (pick == MENU_CANCEL)
            continue;

        switch (choice) {
        case 1:
            gpio_led_set_verbose(pick, 1);
            break;
        case 2:
            gpio_led_set_verbose(pick, 0);
            break;
        case 3:
            menu_reset_stop();
            printf("Fast blink — press Ctrl+C to stop.\n");
            while (!menu_stop_requested()) {
                gpio_led_set(pick, 1);
                usleep(200000);
                gpio_led_set(pick, 0);
                usleep(200000);
            }
            gpio_led_set(pick, 0);
            menu_reset_stop();
            printf("\nBlink stopped.\n");
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}

static void gpio_monitor_inputs(void)
{
    menu_reset_stop();
    printf("Monitoring DI, DIP & Reset button — press Ctrl+C to stop.\n");
    while (!menu_stop_requested()) {
        printf("DI (GPIO %d) = %d | DIP1 (GPIO %d) = %d | DIP2 (GPIO %d) = %d | "
               "Reset (GPIO %d) = %d\n",
               GPIO_DI, gpio_read_di(),
               GPIO_DIP1, gpio_read_dip1(),
               GPIO_DIP2, gpio_read_dip2(),
               GPIO_RESET_BTN, gpio_read_reset_btn());
        fflush(stdout);
        sleep(1);
    }
    menu_reset_stop();
    printf("\nInput monitor stopped.\n");
}

static void gpio_show_menu(void)
{
    printf("\n--- GPIO module ---\n");
    printf(" 1) Init all board GPIO (DI/DO/DIP/Reset)\n");
    printf(" 2) Monitor DI, DIP & Reset button (Ctrl+C to stop)\n");
    printf(" 3) Set DO high (Y1, GPIO %d)\n", GPIO_DO);
    printf(" 4) Set DO low  (Y1, GPIO %d)\n", GPIO_DO);
    printf(" 5) Run demo pulse on DO (Ctrl+C to stop)\n");
    printf(" 6) LED test (steady ON / OFF / fast blink)\n");
    printf(" 0) Back to main menu (Ctrl+C)\n");
}

int gpio_module_menu(void)
{
    for (;;) {
        int choice;

        gpio_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK)
            return 0;

        switch (choice) {
        case 1:
            if (gpio_init_board_io() == 0)
                printf("GPIO initialized (DI/DO/DIP/Reset).\n");
            else
                printf("GPIO init completed with errors.\n");
            break;
        case 2:
            gpio_monitor_inputs();
            break;
        case 3:
            if (gpio_write_value(GPIO_DO, 1) == 0)
                printf("DO (GPIO %d) set HIGH\n", GPIO_DO);
            break;
        case 4:
            if (gpio_write_value(GPIO_DO, 0) == 0)
                printf("DO (GPIO %d) set LOW\n", GPIO_DO);
            break;
        case 5:
            menu_reset_stop();
            printf("DO pulse loop — press Ctrl+C to stop.\n");
            while (!menu_stop_requested()) {
                gpio_write_value(GPIO_DO, 1);
                sleep(1);
                gpio_write_value(GPIO_DO, 0);
                sleep(1);
            }
            gpio_write_value(GPIO_DO, 0);
            menu_reset_stop();
            printf("\nDO pulse stopped.\n");
            break;
        case 6:
            gpio_led_test_menu();
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}

void gpio_module_cli_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s gpio <action> [options]\n"
            "  init           Init board GPIO (menu 1)\n"
            "  monitor        Monitor DI/DIP/Reset until Ctrl+C (menu 2)\n"
            "  do-high        Set DO high (menu 3)\n"
            "  do-low         Set DO low (menu 4)\n"
            "  pulse          DO pulse loop until Ctrl+C (menu 5)\n"
            "  led on|off|blink  LED test (menu 6)\n"
            "Options:\n"
            "  --led N        1=NET, 2=RUN, 3=WARN, 4=All (default 4)\n"
            "\n"
            "Examples:\n"
            "    %s gpio init\n"
            "    %s gpio monitor\n"
            "    %s gpio do-high\n"
            "    %s gpio do-low\n"
            "    %s gpio led on --led 2\n"
            "    %s gpio led blink --led 4\n",
            prog, prog, prog, prog, prog, prog, prog);
}

int gpio_module_cli(int argc, char **argv)
{
    const char *action = argv[1];
    int led_pick = 4;
    int opt;

    static const struct option opts[] = {
        { "led", required_argument, NULL, 'l' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    if (!action || !strcmp(action, "-h") || !strcmp(action, "--help")) {
        gpio_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    optind = 2;
    while ((opt = getopt_long(argc, argv, "l:h", opts, NULL)) != -1) {
        switch (opt) {
        case 'l':
            if (cli_parse_int(optarg, &led_pick) < 0 ||
                led_pick < 1 || led_pick > 4) {
                fprintf(stderr, "Invalid --led value (1-4).\n");
                return CLI_EXIT_USAGE;
            }
            break;
        case 'h':
            gpio_module_cli_usage(argv[0]);
            return CLI_EXIT_OK;
        default:
            gpio_module_cli_usage(argv[0]);
            return CLI_EXIT_USAGE;
        }
    }

    if (!strcmp(action, "init")) {
        if (gpio_init_board_io() == 0)
            printf("GPIO initialized (DI/DO/DIP/Reset).\n");
        else
            printf("GPIO init completed with errors.\n");
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "monitor")) {
        gpio_monitor_inputs();
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "do-high")) {
        if (gpio_write_value(GPIO_DO, 1) == 0)
            printf("DO (GPIO %d) set HIGH\n", GPIO_DO);
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "do-low")) {
        if (gpio_write_value(GPIO_DO, 0) == 0)
            printf("DO (GPIO %d) set LOW\n", GPIO_DO);
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "pulse")) {
        menu_reset_stop();
        printf("DO pulse loop — press Ctrl+C to stop.\n");
        while (!menu_stop_requested()) {
            gpio_write_value(GPIO_DO, 1);
            sleep(1);
            gpio_write_value(GPIO_DO, 0);
            sleep(1);
        }
        gpio_write_value(GPIO_DO, 0);
        menu_reset_stop();
        printf("\nDO pulse stopped.\n");
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "led")) {
        const char *led_action = argv[optind];

        if (gpio_init_leds() < 0)
            printf("Warning: LED GPIO init failed (may need root).\n");

        if (!led_action) {
            fprintf(stderr, "led requires on|off|blink.\n");
            return CLI_EXIT_USAGE;
        }
        if (!strcmp(led_action, "on")) {
            gpio_led_set_verbose(led_pick, 1);
            return CLI_EXIT_OK;
        }
        if (!strcmp(led_action, "off")) {
            gpio_led_set_verbose(led_pick, 0);
            return CLI_EXIT_OK;
        }
        if (!strcmp(led_action, "blink")) {
            menu_reset_stop();
            printf("Fast blink — press Ctrl+C to stop.\n");
            while (!menu_stop_requested()) {
                gpio_led_set(led_pick, 1);
                usleep(200000);
                gpio_led_set(led_pick, 0);
                usleep(200000);
            }
            gpio_led_set(led_pick, 0);
            menu_reset_stop();
            printf("\nBlink stopped.\n");
            return CLI_EXIT_OK;
        }
        fprintf(stderr, "Unknown led action: %s\n", led_action);
        return CLI_EXIT_USAGE;
    }

    fprintf(stderr, "Unknown gpio action: %s\n", action);
    gpio_module_cli_usage(argv[0]);
    return CLI_EXIT_USAGE;
}
