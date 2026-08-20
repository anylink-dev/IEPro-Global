#include "demo.h"
#include "menu_util.h"
#include "cli_util.h"
#include "metrics.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef WITH_MQTT
#include <mosquitto.h>

#ifndef MQTT_DEFAULT_BROKER
#define MQTT_DEFAULT_BROKER   "127.0.0.1"
#endif
#ifndef MQTT_DEFAULT_PORT
#define MQTT_DEFAULT_PORT     1883
#endif
#ifndef MQTT_DEFAULT_CLIENT_ID
#define MQTT_DEFAULT_CLIENT_ID "iepro-demo"
#endif
#ifndef MQTT_DEFAULT_SUB_TOPIC
#define MQTT_DEFAULT_SUB_TOPIC "iepro/demo/cmd"
#endif
#ifndef MQTT_DEFAULT_PUB_TOPIC
#define MQTT_DEFAULT_PUB_TOPIC "iepro/demo/data"
#endif

#define MQTT_BROKER_SIZE    256
#define MQTT_ID_SIZE        128
#define MQTT_USER_SIZE      128
#define MQTT_PASS_SIZE      128
#define MQTT_CA_SIZE        256
#define MQTT_TOPIC_SIZE     256
#define MQTT_PAYLOAD_SIZE   4096

struct mqtt_config {
    char broker[MQTT_BROKER_SIZE];
    int port;
    char client_id[MQTT_ID_SIZE];
    char username[MQTT_USER_SIZE];
    char password[MQTT_PASS_SIZE];
    int tls_enabled;
    char ca_path[MQTT_CA_SIZE];
    char sub_topic[MQTT_TOPIC_SIZE];
    char pub_topic[MQTT_TOPIC_SIZE];
};

static struct mqtt_config g_cfg;
static struct mosquitto *g_mosq;
static int g_mqtt_lib_inited;
static int g_mqtt_connected;
static int g_mqtt_loop_running;

static int mqtt_parse_bool(const char *line, int default_value)
{
    char tmp[32];

    if (!line || line[0] == '\0')
        return default_value;

    snprintf(tmp, sizeof(tmp), "%s", line);
    {
        char *p;

        for (p = tmp; *p; ++p) {
            if (*p >= 'A' && *p <= 'Z')
                *p = (char)(*p - 'A' + 'a');
        }
    }

    if (!strcmp(tmp, "1") || !strcmp(tmp, "true") || !strcmp(tmp, "yes") ||
        !strcmp(tmp, "on"))
        return 1;
    if (!strcmp(tmp, "0") || !strcmp(tmp, "false") || !strcmp(tmp, "no") ||
        !strcmp(tmp, "off"))
        return 0;
    return default_value;
}

static void mqtt_config_init(void)
{
    snprintf(g_cfg.broker, sizeof(g_cfg.broker), "%s", MQTT_DEFAULT_BROKER);
    g_cfg.port = MQTT_DEFAULT_PORT;
    snprintf(g_cfg.client_id, sizeof(g_cfg.client_id), "%s",
             MQTT_DEFAULT_CLIENT_ID);
    g_cfg.username[0] = '\0';
    g_cfg.password[0] = '\0';
    g_cfg.tls_enabled = 0;
    g_cfg.ca_path[0] = '\0';
    snprintf(g_cfg.sub_topic, sizeof(g_cfg.sub_topic), "%s",
             MQTT_DEFAULT_SUB_TOPIC);
    snprintf(g_cfg.pub_topic, sizeof(g_cfg.pub_topic), "%s",
             MQTT_DEFAULT_PUB_TOPIC);
}

static void mqtt_on_message(struct mosquitto *mosq, void *obj,
                            const struct mosquitto_message *msg)
{
    (void)mosq;
    (void)obj;
    printf("\n[MQTT] received on %s: %.*s\n",
           msg->topic, msg->payloadlen, (char *)msg->payload);
}

static int mqtt_lib_init_once(void)
{
    if (g_mqtt_lib_inited)
        return 0;
    if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "mosquitto_lib_init failed\n");
        return -1;
    }
    g_mqtt_lib_inited = 1;
    return 0;
}

static void mqtt_lib_cleanup(void)
{
    if (!g_mqtt_lib_inited)
        return;
    mosquitto_lib_cleanup();
    g_mqtt_lib_inited = 0;
}

static void mqtt_teardown_client(void)
{
    struct mosquitto *mosq = g_mosq;
    int loop_running = g_mqtt_loop_running;
    int connected = g_mqtt_connected;

    if (!mosq)
        return;

    g_mosq = NULL;
    g_mqtt_connected = 0;
    g_mqtt_loop_running = 0;

    if (loop_running)
        mosquitto_loop_stop(mosq, true);

    if (connected)
        mosquitto_disconnect(mosq);

    mosquitto_destroy(mosq);
}

static int mqtt_apply_tls(struct mosquitto *mosq)
{
    int rc;

    if (!g_cfg.tls_enabled)
        return 0;

    if (g_cfg.ca_path[0] != '\0') {
        rc = mosquitto_tls_set(mosq, g_cfg.ca_path, NULL, NULL, NULL, NULL);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "mosquitto_tls_set failed: %s\n",
                    mosquitto_strerror(rc));
            return -1;
        }
        return 0;
    }

    rc = mosquitto_tls_insecure_set(mosq, true);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "mosquitto_tls_insecure_set failed: %s\n",
                mosquitto_strerror(rc));
        return -1;
    }
    printf("TLS enabled without CA verification (lab only).\n");
    return 0;
}

static int mqtt_start_loop(void)
{
    int rc;

    if (!g_mosq) {
        fprintf(stderr, "MQTT client not initialized.\n");
        return -1;
    }
    if (g_mqtt_loop_running)
        return 0;

    rc = mosquitto_loop_start(g_mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "mosquitto_loop_start failed: %s\n",
                mosquitto_strerror(rc));
        return -1;
    }

    g_mqtt_loop_running = 1;
    return 0;
}

static int mqtt_connect(void)
{
    int rc;

    if (g_mqtt_connected && g_mqtt_loop_running)
        return 0;
    if (g_mqtt_connected)
        return mqtt_start_loop();

    if (mqtt_lib_init_once() < 0)
        return -1;

    mqtt_teardown_client();

    g_mosq = mosquitto_new(g_cfg.client_id, true, NULL);
    if (!g_mosq) {
        fprintf(stderr, "mosquitto_new failed\n");
        return -1;
    }

    mosquitto_message_callback_set(g_mosq, mqtt_on_message);

    if (g_cfg.username[0] != '\0') {
        rc = mosquitto_username_pw_set(g_mosq, g_cfg.username,
                                       g_cfg.password[0] != '\0' ?
                                           g_cfg.password : NULL);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "mosquitto_username_pw_set failed: %s\n",
                    mosquitto_strerror(rc));
            mqtt_teardown_client();
            return -1;
        }
    }

    if (mqtt_apply_tls(g_mosq) < 0) {
        mqtt_teardown_client();
        return -1;
    }

    if (mqtt_start_loop() < 0) {
        mqtt_teardown_client();
        return -1;
    }

    rc = mosquitto_connect(g_mosq, g_cfg.broker, g_cfg.port, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "connect to %s:%d failed: %s\n",
                g_cfg.broker, g_cfg.port, mosquitto_strerror(rc));
        mqtt_teardown_client();
        return -1;
    }

    g_mqtt_connected = 1;
    printf("Connected to %s:%d\n", g_cfg.broker, g_cfg.port);

    if (g_cfg.sub_topic[0] != '\0') {
        rc = mosquitto_subscribe(g_mosq, NULL, g_cfg.sub_topic, 0);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "subscribe %s failed: %s\n",
                    g_cfg.sub_topic, mosquitto_strerror(rc));
        } else {
            printf("Subscribed to %s\n", g_cfg.sub_topic);
        }
    }

    return 0;
}

static void mqtt_disconnect(void)
{
    printf("Disconnecting...\n");
    fflush(stdout);
    mqtt_teardown_client();
    printf("Disconnected.\n");
}

static void mqtt_show_status(void)
{
    printf("\n--- MQTT status ---\n");
    printf("Broker       : %s:%d\n", g_cfg.broker, g_cfg.port);
    printf("Client ID    : %s\n", g_cfg.client_id);
    if (g_cfg.username[0] != '\0')
        printf("Username     : %s\n", g_cfg.username);
    else
        printf("Username     : (none)\n");
    if (g_cfg.password[0] != '\0')
        printf("Password     : ****\n");
    else
        printf("Password     : (none)\n");
    printf("TLS          : %s\n", g_cfg.tls_enabled ? "true" : "false");
    if (g_cfg.tls_enabled) {
        if (g_cfg.ca_path[0] != '\0')
            printf("CA path      : %s\n", g_cfg.ca_path);
        else
            printf("CA path      : (none, skip verify)\n");
    }
    printf("Subscribe    : %s\n",
           g_cfg.sub_topic[0] != '\0' ? g_cfg.sub_topic : "(none)");
    printf("Publish topic: %s\n", g_cfg.pub_topic);
    if (g_mqtt_connected && g_mqtt_loop_running)
        printf("Session      : connected\n");
    else
        printf("Session      : disconnected\n");
    printf("-------------------\n");
}

static int mqtt_read_line_keep(const char *prompt, char *buf, size_t len)
{
    if (menu_read_line(prompt, buf, len) < 0)
        return -1;
    return 0;
}


static void mqtt_configure_params(void)
{
    char line[MQTT_BROKER_SIZE];
    int port;

    printf("\n--- Configure MQTT connection ---\n");
    printf("(Press Enter on each field to keep the current value.)\n\n");

    printf("Broker [%s]: ", g_cfg.broker);
    fflush(stdout);
    if (mqtt_read_line_keep("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] != '\0')
        snprintf(g_cfg.broker, sizeof(g_cfg.broker), "%s", line);

    port = menu_read_int("Port: ", g_cfg.port);
    if (port == MENU_CANCEL) {
        printf("Cancelled.\n");
        return;
    }
    if (port > 0 && port <= 65535)
        g_cfg.port = port;
    else
        printf("Invalid port, keeping %d.\n", g_cfg.port);

    printf("Client ID [%s]: ", g_cfg.client_id);
    fflush(stdout);
    if (mqtt_read_line_keep("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] != '\0')
        snprintf(g_cfg.client_id, sizeof(g_cfg.client_id), "%s", line);

    printf("Username [%s] (- = clear): ",
           g_cfg.username[0] != '\0' ? g_cfg.username : "(none)");
    fflush(stdout);
    if (mqtt_read_line_keep("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] == '-') {
        g_cfg.username[0] = '\0';
        g_cfg.password[0] = '\0';
    } else if (line[0] != '\0') {
        snprintf(g_cfg.username, sizeof(g_cfg.username), "%s", line);
    }

    if (g_cfg.username[0] != '\0') {
        printf("Password [%s]: ",
               g_cfg.password[0] != '\0' ? "****" : "(none)");
        fflush(stdout);
        if (mqtt_read_line_keep("", line, sizeof(line)) < 0) {
            printf("Cancelled.\n");
            return;
        }
        if (line[0] != '\0')
            snprintf(g_cfg.password, sizeof(g_cfg.password), "%s", line);
    }

    printf("TLS (true/false) [%s]: ",
           g_cfg.tls_enabled ? "true" : "false");
    fflush(stdout);
    if (mqtt_read_line_keep("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] != '\0')
        g_cfg.tls_enabled = mqtt_parse_bool(line, g_cfg.tls_enabled);

    if (g_cfg.tls_enabled) {
        printf("CA path [%s]: ",
               g_cfg.ca_path[0] != '\0' ? g_cfg.ca_path : "(none, skip verify)");
        fflush(stdout);
        if (mqtt_read_line_keep("", line, sizeof(line)) < 0) {
            printf("Cancelled.\n");
            return;
        }
        if (line[0] != '\0')
            snprintf(g_cfg.ca_path, sizeof(g_cfg.ca_path), "%s", line);
    } else {
        g_cfg.ca_path[0] = '\0';
    }

    printf("Subscribe topic [%s] (- = clear): ",
           g_cfg.sub_topic[0] != '\0' ? g_cfg.sub_topic : "(none)");
    fflush(stdout);
    if (mqtt_read_line_keep("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] == '-')
        g_cfg.sub_topic[0] = '\0';
    else if (line[0] != '\0')
        snprintf(g_cfg.sub_topic, sizeof(g_cfg.sub_topic), "%s", line);

    printf("Default publish topic [%s]: ", g_cfg.pub_topic);
    fflush(stdout);
    if (mqtt_read_line_keep("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] != '\0')
        snprintf(g_cfg.pub_topic, sizeof(g_cfg.pub_topic), "%s", line);

    printf("Configuration updated.\n");
}

static int mqtt_fill_default_payload(char *payload, size_t len)
{
    int n;

    if (!payload || len == 0)
        return -1;

    n = metrics_build_json(payload, len);
    if (n < 0 || n >= (int)len || payload[0] == '\0')
        return -1;

    return 0;
}

static void mqtt_publish_message(void)
{
    char topic[MQTT_TOPIC_SIZE];
    char payload[MQTT_PAYLOAD_SIZE];
    int rc;

    if (!g_mqtt_connected || !g_mosq) {
        printf("Not connected. Use Connect first.\n");
        return;
    }

    printf("Publish topic [%s]: ", g_cfg.pub_topic);
    fflush(stdout);
    if (menu_read_line("", topic, sizeof(topic)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (topic[0] == '\0')
        snprintf(topic, sizeof(topic), "%s", g_cfg.pub_topic);

    printf("Message body (empty = sample metrics JSON): ");
    fflush(stdout);
    if (menu_read_line("", payload, sizeof(payload)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (payload[0] == '\0') {
        if (mqtt_fill_default_payload(payload, sizeof(payload)) < 0) {
            fprintf(stderr, "Failed to build default metrics payload.\n");
            return;
        }
        printf("Using default: %s\n", payload);
    }

    rc = mosquitto_publish(g_mosq, NULL, topic,
                           (int)strlen(payload), payload, 0, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "publish failed: %s\n", mosquitto_strerror(rc));
        return;
    }

    printf("Published to %s\n%s\n", topic, payload);
}

static void mqtt_show_menu(void)
{
    printf("\n--- MQTT module ---\n");
    printf(" 1) Configure MQTT connection parameters\n");
    printf(" 2) Connect (broker session + background loop)\n");
    printf(" 3) Disconnect\n");
    printf(" 4) Show current status\n");
    printf(" 5) Publish message\n");
    printf(" 0) Back to main menu (Ctrl+C)\n");
}

int mqtt_module_menu(void)
{
    mqtt_config_init();

    for (;;) {
        int choice;

        mqtt_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK) {
            mqtt_disconnect();
            mqtt_lib_cleanup();
            return 0;
        }

        switch (choice) {
        case 1:
            mqtt_configure_params();
            break;
        case 2:
            mqtt_connect();
            break;
        case 3:
            mqtt_disconnect();
            break;
        case 4:
            mqtt_show_status();
            break;
        case 5:
            mqtt_publish_message();
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}

void mqtt_module_cli_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s mqtt <connect|publish> [options]\n"
            "  connect        Connect/subscribe until Ctrl+C (menu 2)\n"
            "  publish        Connect, publish once, disconnect (menu 5)\n"
            "Options:\n"
            "  --broker HOST  --port N  --client-id ID\n"
            "  --username U   --password P\n"
            "  --tls true|false  --ca PATH\n"
            "  --sub-topic T  --pub-topic T\n"
            "  --topic T      Publish topic (publish action)\n"
            "  --message STR  Message body (publish; default: sample metrics JSON)\n"
            "\n"
            "Examples:\n"
            "  Publish once (custom message):\n"
            "    %s mqtt publish --broker 192.168.1.10 --port 1883 \\\n"
            "      --topic iepro/demo/data --message '{\"temp\":25}'\n"
            "  Publish once (default metrics JSON):\n"
            "    %s mqtt publish --broker 192.168.1.10 --port 1883 \\\n"
            "      --topic iepro/demo/data\n"
            "  Connect and subscribe until Ctrl+C:\n"
            "    %s mqtt connect --broker 192.168.1.10 \\\n"
            "      --sub-topic iepro/demo/cmd\n"
            "  TLS publish (skip verify, lab only):\n"
            "    %s mqtt publish --broker mqtt.example.com --port 8883 \\\n"
            "      --tls true --topic test --message hello\n",
            prog, prog, prog, prog, prog);
}

int mqtt_module_cli(int argc, char **argv)
{
    const char *action = argv[1];
    const char *message = NULL;
    const char *topic = NULL;
    int opt;
    int rc;

    static const struct option opts[] = {
        { "broker", required_argument, NULL, 'b' },
        { "port", required_argument, NULL, 'p' },
        { "client-id", required_argument, NULL, 'i' },
        { "username", required_argument, NULL, 'u' },
        { "password", required_argument, NULL, 'P' },
        { "tls", required_argument, NULL, 't' },
        { "ca", required_argument, NULL, 'c' },
        { "sub-topic", required_argument, NULL, 's' },
        { "pub-topic", required_argument, NULL, 'T' },
        { "topic", required_argument, NULL, 1000 },
        { "message", required_argument, NULL, 'm' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    mqtt_config_init();

    if (!action || !strcmp(action, "-h") || !strcmp(action, "--help")) {
        mqtt_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    optind = 2;
    while ((opt = getopt_long(argc, argv, "b:p:i:u:P:t:c:s:T:m:h", opts,
                              NULL)) != -1) {
        switch (opt) {
        case 'b':
            snprintf(g_cfg.broker, sizeof(g_cfg.broker), "%s", optarg);
            break;
        case 'p': {
            int port_val;
            if (cli_parse_int(optarg, &port_val) == 0 &&
                port_val > 0 && port_val <= 65535)
                g_cfg.port = port_val;
            break;
        }
        case 'i':
            snprintf(g_cfg.client_id, sizeof(g_cfg.client_id), "%s", optarg);
            break;
        case 'u':
            snprintf(g_cfg.username, sizeof(g_cfg.username), "%s", optarg);
            break;
        case 'P':
            snprintf(g_cfg.password, sizeof(g_cfg.password), "%s", optarg);
            break;
        case 't':
            g_cfg.tls_enabled = cli_parse_bool(optarg, g_cfg.tls_enabled);
            break;
        case 'c':
            snprintf(g_cfg.ca_path, sizeof(g_cfg.ca_path), "%s", optarg);
            break;
        case 's':
            if (!strcmp(optarg, "-"))
                g_cfg.sub_topic[0] = '\0';
            else
                snprintf(g_cfg.sub_topic, sizeof(g_cfg.sub_topic), "%s",
                         optarg);
            break;
        case 'T':
            snprintf(g_cfg.pub_topic, sizeof(g_cfg.pub_topic), "%s", optarg);
            break;
        case 1000:
            topic = optarg;
            break;
        case 'm':
            message = optarg;
            break;
        case 'h':
            mqtt_module_cli_usage(argv[0]);
            return CLI_EXIT_OK;
        default:
            mqtt_module_cli_usage(argv[0]);
            return CLI_EXIT_USAGE;
        }
    }

    if (!strcmp(action, "connect")) {
        if (mqtt_connect() < 0)
            return CLI_EXIT_FAIL;

        menu_reset_stop();
        printf("Connected. Press Ctrl+C to disconnect.\n");
        while (!menu_stop_requested())
            sleep(1);

        mqtt_disconnect();
        mqtt_lib_cleanup();
        return CLI_EXIT_OK;
    }
    if (!strcmp(action, "publish")) {
        char payload_buf[MQTT_PAYLOAD_SIZE];
        const char *pub_topic;
        const char *body;

        if (message && message[0] != '\0') {
            body = message;
        } else if (mqtt_fill_default_payload(payload_buf,
                                             sizeof(payload_buf)) == 0) {
            body = payload_buf;
        } else {
            fprintf(stderr, "Failed to build default metrics payload.\n");
            return CLI_EXIT_FAIL;
        }

        pub_topic = (topic && topic[0] != '\0') ? topic : g_cfg.pub_topic;

        if (!g_mqtt_connected) {
            if (mqtt_connect() < 0)
                return CLI_EXIT_FAIL;
        }

        rc = mosquitto_publish(g_mosq, NULL, pub_topic,
                               (int)strlen(body), body, 0, false);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "publish failed: %s\n", mosquitto_strerror(rc));
            mqtt_disconnect();
            mqtt_lib_cleanup();
            return CLI_EXIT_FAIL;
        }

        printf("Published to %s\n%s\n", pub_topic, body);
        mqtt_disconnect();
        mqtt_lib_cleanup();
        return CLI_EXIT_OK;
    }

    fprintf(stderr, "Unknown mqtt action: %s\n", action);
    mqtt_module_cli_usage(argv[0]);
    return CLI_EXIT_USAGE;
}

#else

int mqtt_module_menu(void)
{
    printf("\nMQTT module was not built.\n");
    printf("Rebuild after extracting prebuilt deps (see deps/README.md).\n");
    menu_pause();
    return 0;
}

void mqtt_module_cli_usage(const char *prog)
{
    fprintf(stderr, "MQTT module was not built (missing WITH_MQTT).\n");
    (void)prog;
}

int mqtt_module_cli(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("MQTT module was not built.\n");
    return CLI_EXIT_FAIL;
}

#endif

