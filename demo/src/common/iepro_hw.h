#ifndef IEPRO_HW_H
#define IEPRO_HW_H

/* IE Pro 400 Global Standard — hardware interface map */

#define IEPRO_DEV_RS232_1   "/dev/ttymxc5"
#define IEPRO_DEV_RS485_1   "/dev/ttymxc1"
#define IEPRO_DEV_RS485_2   "/dev/ttymxc2"

/* CAN: can0, factory-installed module, default 250000 bps */
#define IEPRO_CAN_IFACE     "can0"
#define IEPRO_CAN_DEFAULT_BITRATE 250000

/* SIM7600G-H-PCIE cellular modem (NDIS dial-up via AT$QCRMCALL) */
#define IEPRO_MODEM_AT_DEV  "/dev/ttyUSB2"
#define IEPRO_CELL_IFACE    "wwan0"

#define GPIO_DI             117
#define GPIO_DO             118
#define GPIO_DIP1           124
#define GPIO_DIP2           121
#define GPIO_RESET_BTN      119
#define GPIO_LED_NET        122
#define GPIO_LED_RUN        71
#define GPIO_LED_ALARM      123

#endif
