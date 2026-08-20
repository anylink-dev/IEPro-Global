#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#define IEPRO_SERIAL_PORT_RS232    1
#define IEPRO_SERIAL_PORT_RS485_1  2
#define IEPRO_SERIAL_PORT_RS485_2  3

const char *iepro_serial_device_for_port(int port);
const char *iepro_serial_port_label(int port);

/* Returns 1..3, MENU_CANCEL, or clamps invalid input to RS232. */
int iepro_serial_pick_port(int default_port);

#endif
