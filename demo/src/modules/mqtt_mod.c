#include "demo.h"
#include "gpio_util.h"
#include "menu_util.h"
#include "metrics.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef WITH_MQTT
#include <mosquitto.h>

#define MQTT_BROKER    "your-broker-host"
#define MQTT_PORT      1883
#define MQTT_DEVICE_ID "your-device-id"

static char topic_data[128];
static char topic_cmd[128];
static struct mosquitto *g_mosq;
static int g_mqtt_connected;

static void mqtt_build_payload(char *buf, size_t len)
{
    char metrics[192];

    metrics_build_json(metrics, sizeof(metrics));
    snprintf(buf, len,
             "{\"device_id\":\"%s\","
             "\"di\":{\"di1\":%d,\"dip1\":%d,\"dip2\":%d},"
             "\"metrics\":%s}",
             MQTT_DEVICE_ID,
             gpio_read_di(), gpio_read_dip1(), gpio_read_dip2(),
             metrics);
}

static void mqtt_on_message(struct mosquitto *mosq, void *obj,
                            const struct mosquitto_message *msg)
{
    (void)mosq;
    (void)obj;
    printf("MQTT received on %s: %.*s\n",
           msg->topic, msg->payloadlen, (char *)msg->payload);
}

static int mqtt_ensure_connected(void)
{
    if (g_mqtt_connected)
        return 0;

    mosquitto_lib_init();
    g_mosq = mosquitto_new(MQTT_DEVICE_ID, true, NULL);
    if (!g_mosq) {
        fprintf(stderr, "mosquitto_new failed\n");
        return -1;
    }

    snprintf(topic_data, sizeof(topic_data), "iepro/%s/data", MQTT_DEVICE_ID);
    snprintf(topic_cmd, sizeof(topic_cmd), "iepro/%s/cmd", MQTT_DEVICE_ID);
    mosquitto_message_callback_set(g_mosq, mqtt_on_message);

    if (mosquitto_connect(g_mosq, MQTT_BROKER, MQTT_PORT, 60) !=
        MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "connect to %s failed\n", MQTT_BROKER);
        mosquitto_destroy(g_mosq);
        g_mosq = NULL;
        mosquitto_lib_cleanup();
        return -1;
    }

    mosquitto_subscribe(g_mosq, NULL, topic_cmd, 0);
    mosquitto_loop_start(g_mosq);
    g_mqtt_connected = 1;
    printf("Connected to %s\n", MQTT_BROKER);
    return 0;
}

static void mqtt_disconnect(void)
{
    if (!g_mqtt_connected)
        return;
    mosquitto_loop_stop(g_mosq, true);
    mosquitto_disconnect(g_mosq);
    mosquitto_destroy(g_mosq);
    g_mosq = NULL;
    mosquitto_lib_cleanup();
    g_mqtt_connected = 0;
}

static void mqtt_show_menu(void)
{
    printf("\n--- MQTT module ---\n");
    printf("Broker : %s:%d\n", MQTT_BROKER, MQTT_PORT);
    printf("Device : %s\n", MQTT_DEVICE_ID);
    printf(" 1) Publish one sample message\n");
    printf(" 2) Run publish loop (10s interval, Ctrl+C to stop)\n");
    printf(" 0) Back to main menu (Ctrl+C)\n");
}

int mqtt_module_menu(void)
{
    char payload[512];

    if (gpio_init_board_io() < 0)
        printf("Warning: GPIO init failed, DI values may be zero.\n");

    for (;;) {
        int choice;

        mqtt_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK) {
            mqtt_disconnect();
            return 0;
        }
        if (mqtt_ensure_connected() < 0) {
            menu_pause();
            continue;
        }

        switch (choice) {
        case 1:
            mqtt_build_payload(payload, sizeof(payload));
            mosquitto_publish(g_mosq, NULL, topic_data,
                              (int)strlen(payload), payload, 1, false);
            printf("Published to %s\n%s\n", topic_data, payload);
            break;
        case 2:
            menu_reset_stop();
            printf("Publishing every 10s — press Ctrl+C to stop.\n");
            while (!menu_stop_requested()) {
                mqtt_build_payload(payload, sizeof(payload));
                mosquitto_publish(g_mosq, NULL, topic_data,
                                  (int)strlen(payload), payload, 1, false);
                printf("Published to %s\n", topic_data);
                sleep(10);
            }
            printf("\nMQTT loop stopped.\n");
            menu_reset_stop();
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}

#else

int mqtt_module_menu(void)
{
    printf("\nMQTT module was not built.\n");
    printf("Rebuild with: make WITH_MQTT=1 CROSS_COMPILE=...\n");
    menu_pause();
    return 0;
}

#endif
