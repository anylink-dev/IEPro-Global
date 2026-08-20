#include "serial_port.h"
#include "iepro_hw.h"
#include "menu_util.h"

#include <stdio.h>

const char *iepro_serial_device_for_port(int port)
{
    switch (port) {
    case IEPRO_SERIAL_PORT_RS232:
        return IEPRO_DEV_RS232_1;
    case IEPRO_SERIAL_PORT_RS485_1:
        return IEPRO_DEV_RS485_1;
    case IEPRO_SERIAL_PORT_RS485_2:
        return IEPRO_DEV_RS485_2;
    default:
        return NULL;
    }
}

const char *iepro_serial_port_label(int port)
{
    switch (port) {
    case IEPRO_SERIAL_PORT_RS232:
        return "RS232";
    case IEPRO_SERIAL_PORT_RS485_1:
        return "RS485-1";
    case IEPRO_SERIAL_PORT_RS485_2:
        return "RS485-2";
    default:
        return "unknown";
    }
}

int iepro_serial_pick_port(int default_port)
{
    int port;

    if (default_port < IEPRO_SERIAL_PORT_RS232 ||
        default_port > IEPRO_SERIAL_PORT_RS485_2)
        default_port = IEPRO_SERIAL_PORT_RS232;

    port = menu_read_int(
        "Port [1=RS232 / 2=RS485-1 / 3=RS485-2]: ", default_port);
    if (port == MENU_CANCEL)
        return MENU_CANCEL;
    if (port < IEPRO_SERIAL_PORT_RS232 ||
        port > IEPRO_SERIAL_PORT_RS485_2) {
        printf("Invalid port, using %s.\n",
               iepro_serial_port_label(default_port));
        return default_port;
    }
    return port;
}
