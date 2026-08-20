#include "demo.h"
#include "menu_util.h"
#include "cli_util.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef WITH_CURL
#include <curl/curl.h>

#ifndef HTTP_DEFAULT_URL
#define HTTP_DEFAULT_URL   "https://example.com"
#endif

#define HTTP_USER_AGENT    "IEPro-Demo/1.0"
#define HTTP_TIMEOUT_SEC   30L
#define HTTP_RESP_MAX      (64 * 1024)
#define HTTP_URL_SIZE      512
#define HTTP_CA_SIZE       256
#define HTTP_BODY_SIZE     4096

struct http_response {
    char body[HTTP_RESP_MAX + 1];
    size_t len;
};

static int g_curl_ready;
static char g_http_url[HTTP_URL_SIZE];
static char g_ca_path[HTTP_CA_SIZE];

static size_t http_write_cb(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct http_response *resp = userdata;
    size_t chunk = size * nmemb;
    size_t room;

    if (!resp || chunk == 0)
        return 0;

    room = HTTP_RESP_MAX - resp->len;
    if (room == 0)
        return chunk;

    if (chunk > room)
        chunk = room;

    memcpy(resp->body + resp->len, ptr, chunk);
    resp->len += chunk;
    resp->body[resp->len] = '\0';
    return size * nmemb;
}

static int http_curl_init(void)
{
    if (g_curl_ready)
        return 0;

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "curl_global_init failed\n");
        return -1;
    }
    g_curl_ready = 1;
    return 0;
}

static void http_curl_cleanup(void)
{
    if (!g_curl_ready)
        return;
    curl_global_cleanup();
    g_curl_ready = 0;
}

static void http_config_init(void)
{
    snprintf(g_http_url, sizeof(g_http_url), "%s", HTTP_DEFAULT_URL);
    g_ca_path[0] = '\0';
}

static void http_apply_tls_options(CURL *curl)
{
    if (g_ca_path[0] != '\0') {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_CAINFO, g_ca_path);
        return;
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

static void http_print_response(long http_code, const struct http_response *resp)
{
    printf("HTTP status: %ld\n", http_code);
    printf("Body (%zu bytes", resp->len);
    if (resp->len >= HTTP_RESP_MAX)
        printf(", truncated");
    printf("):\n");
    if (resp->len > 0)
        fwrite(resp->body, 1, resp->len, stdout);
    printf("\n");
}

static int http_request(const char *method, const char *url,
                        const char *post_body)
{
    CURL *curl;
    CURLcode res;
    long http_code = 0;
    struct http_response resp;

    if (!url || !url[0]) {
        fprintf(stderr, "empty URL\n");
        return -1;
    }
    if (http_curl_init() < 0)
        return -1;

    memset(&resp, 0, sizeof(resp));

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init failed\n");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, HTTP_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT_SEC);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    http_apply_tls_options(curl);

    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                         post_body ? post_body : "");
    }

    printf("HTTP %s %s\n", method, url);
    if (g_ca_path[0] != '\0')
        printf("TLS verify: on (CA: %s)\n", g_ca_path);
    else
        printf("TLS verify: off\n");

    if (post_body && post_body[0] != '\0')
        printf("Request body: %s\n", post_body);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl error: %s\n", curl_easy_strerror(res));
        if (g_ca_path[0] != '\0')
            printf("Hint: check CA path, or clear CA path to skip verification.\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    http_print_response(http_code, &resp);

    return (http_code >= 200 && http_code < 300) ? 0 : -1;
}

static void http_configure_url(void)
{
    char line[HTTP_URL_SIZE];

    printf("Current URL: %s\n", g_http_url);
    printf("Enter new URL (empty = keep current): ");
    fflush(stdout);
    if (menu_read_line("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (line[0] != '\0')
        snprintf(g_http_url, sizeof(g_http_url), "%s", line);
}

static void http_configure_ca(void)
{
    char line[HTTP_CA_SIZE];

    if (g_ca_path[0] != '\0')
        printf("Current CA path: %s\n", g_ca_path);
    else
        printf("Current CA path: (empty, TLS verification disabled)\n");

    printf("Enter CA certificate path (empty = disable verification): ");
    fflush(stdout);
    if (menu_read_line("", line, sizeof(line)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    snprintf(g_ca_path, sizeof(g_ca_path), "%s", line);
}

static void http_do_post(void)
{
    char body[HTTP_BODY_SIZE];
    const char *default_body = "{\"message\":\"hello from IEPro demo\"}";

    printf("POST body [%s]: ", default_body);
    fflush(stdout);
    if (menu_read_line("", body, sizeof(body)) < 0) {
        printf("Cancelled.\n");
        return;
    }
    if (body[0] == '\0')
        snprintf(body, sizeof(body), "%s", default_body);

    http_request("POST", g_http_url, body);
}

static void http_show_config(void)
{
    printf("\n--- Current HTTP configuration ---\n");
    printf("URL          : %s\n", g_http_url);
    printf("User-Agent   : %s\n", HTTP_USER_AGENT);
    printf("Timeout      : %ld s\n", HTTP_TIMEOUT_SEC);
    if (g_ca_path[0] != '\0') {
        printf("TLS verify   : on\n");
        printf("CA cert path : %s\n", g_ca_path);
    } else {
        printf("TLS verify   : off (no CA certificate)\n");
        printf("CA cert path : (not set)\n");
    }
    printf("----------------------------------\n");
}

static void http_show_menu(void)
{
    printf("\n--- HTTP module (GET / POST) ---\n");
    printf(" 1) Set URL\n");
    printf(" 2) Set CA certificate path\n");
    printf(" 3) Show current configuration\n");
    printf(" 4) GET\n");
    printf(" 5) POST\n");
    printf(" 0) Back to main menu (Ctrl+C)\n");
}

int http_module_menu(void)
{
    http_config_init();

    for (;;) {
        int choice;

        http_show_menu();
        choice = menu_read_choice("Select: ");
        if (choice == MENU_BACK) {
            http_curl_cleanup();
            return 0;
        }

        switch (choice) {
        case 1:
            http_configure_url();
            break;
        case 2:
            http_configure_ca();
            break;
        case 3:
            http_show_config();
            break;
        case 4:
            http_request("GET", g_http_url, NULL);
            break;
        case 5:
            http_do_post();
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
        menu_pause();
    }
}

void http_module_cli_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s http <get|post> [options]\n"
            "  get            HTTP GET (menu 4)\n"
            "  post           HTTP POST (menu 5)\n"
            "Options:\n"
            "  --url URL      Target URL (default: %s)\n"
            "  --ca PATH      CA certificate path (omit = skip TLS verify)\n"
            "  --body STR     POST body (default JSON hello message)\n"
            "\n"
            "Examples:\n"
            "    %s http get --url https://example.com\n"
            "    %s http post --url https://httpbin.org/post \\\n"
            "      --body '{\"message\":\"hello from IEPro demo\"}'\n"
            "    %s http get --url https://example.com --ca /etc/ssl/certs/ca.pem\n",
            prog, HTTP_DEFAULT_URL, prog, prog, prog);
}

int http_module_cli(int argc, char **argv)
{
    const char *action = argv[1];
    const char *url = NULL;
    const char *ca = NULL;
    const char *body = NULL;
    int opt;
    int rc;

    static const struct option opts[] = {
        { "url", required_argument, NULL, 'u' },
        { "ca", required_argument, NULL, 'c' },
        { "body", required_argument, NULL, 'b' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    http_config_init();

    if (!action || !strcmp(action, "-h") || !strcmp(action, "--help")) {
        http_module_cli_usage(argv[0]);
        return CLI_EXIT_USAGE;
    }

    optind = 2;
    while ((opt = getopt_long(argc, argv, "u:c:b:h", opts, NULL)) != -1) {
        switch (opt) {
        case 'u':
            url = optarg;
            break;
        case 'c':
            ca = optarg;
            break;
        case 'b':
            body = optarg;
            break;
        case 'h':
            http_module_cli_usage(argv[0]);
            return CLI_EXIT_OK;
        default:
            http_module_cli_usage(argv[0]);
            return CLI_EXIT_USAGE;
        }
    }

    if (url && url[0] != '\0')
        snprintf(g_http_url, sizeof(g_http_url), "%s", url);
    if (ca)
        snprintf(g_ca_path, sizeof(g_ca_path), "%s", ca);

    if (!strcmp(action, "get")) {
        rc = http_request("GET", g_http_url, NULL);
        http_curl_cleanup();
        return rc == 0 ? CLI_EXIT_OK : CLI_EXIT_FAIL;
    }
    if (!strcmp(action, "post")) {
        char post_body[HTTP_BODY_SIZE];
        const char *default_body = "{\"message\":\"hello from IEPro demo\"}";

        if (body && body[0] != '\0')
            snprintf(post_body, sizeof(post_body), "%s", body);
        else
            snprintf(post_body, sizeof(post_body), "%s", default_body);

        rc = http_request("POST", g_http_url, post_body);
        http_curl_cleanup();
        return rc == 0 ? CLI_EXIT_OK : CLI_EXIT_FAIL;
    }

    fprintf(stderr, "Unknown http action: %s\n", action);
    http_module_cli_usage(argv[0]);
    return CLI_EXIT_USAGE;
}

#else

int http_module_menu(void)
{
    printf("\nHTTP module was not built.\n");
    printf("Rebuild after extracting prebuilt deps (see deps/README.md).\n");
    menu_pause();
    return 0;
}

void http_module_cli_usage(const char *prog)
{
    fprintf(stderr, "HTTP module was not built (missing WITH_CURL).\n");
    (void)prog;
}

int http_module_cli(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("HTTP module was not built.\n");
    return CLI_EXIT_FAIL;
}

#endif
