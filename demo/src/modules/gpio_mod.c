#include "demo.h"
#include "gpio_util.h"
#include "menu_util.h"

#include <stdio.h>
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
