# Makefile — build unified IE Pro demo console (build/iepro_demo).
#
# Layout:
#   src/          application source
#   deps/         third-party headers and prebuilt libs (mosquitto, modbus, curl, openssl)
#   build/        output directory
#
# Usage:
#   make CROSS_COMPILE=arm-linux-gnueabihf-
#   make WITH_MQTT=1 CROSS_COMPILE=arm-linux-gnueabihf-
#   make WITH_MQTT=1 WITH_MODBUS=1 WITH_CURL=1 WITH_SSL=1 CROSS_COMPILE=...

CROSS_COMPILE ?=
CC      = $(CROSS_COMPILE)gcc

SRC_DIR    = src
COMMON_DIR = $(SRC_DIR)/common
MOD_DIR    = $(SRC_DIR)/modules
BUILD_DIR  = build
DEPS_DIR   = deps

CFLAGS  = -Wall -Wextra -O2 -I$(SRC_DIR) -I$(COMMON_DIR) -I$(MOD_DIR)
LDFLAGS =

SRCS = $(SRC_DIR)/main.c \
       $(wildcard $(COMMON_DIR)/*.c) \
       $(wildcard $(MOD_DIR)/*.c)

TARGET = $(BUILD_DIR)/iepro_demo

ifdef WITH_MQTT
CFLAGS  += -DWITH_MQTT -I$(DEPS_DIR)/mosquitto/include
LDFLAGS += -L$(DEPS_DIR)/mosquitto/lib -lmosquitto
endif

ifdef WITH_MODBUS
CFLAGS  += -DWITH_MODBUS -I$(DEPS_DIR)/modbus/include
LDFLAGS += -L$(DEPS_DIR)/modbus/lib -lmodbus
endif

ifdef WITH_CURL
CFLAGS  += -DWITH_CURL -I$(DEPS_DIR)/curl/include
LDFLAGS += -L$(DEPS_DIR)/curl/lib -lcurl
endif

ifdef WITH_SSL
CFLAGS  += -DWITH_SSL -I$(DEPS_DIR)/openssl/include
LDFLAGS += -L$(DEPS_DIR)/openssl/lib -lssl -lcrypto
endif

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
