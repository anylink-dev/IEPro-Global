# Makefile — build unified IE Pro demo console (build/iepro_demo).
#
# Layout:
#   src/                    application source
#   deps/arm-linux-gnueabihf/   DEPS_PREFIX (headers + static/shared libs)
#   build/                  output directory
#
# Prerequisites:
#   Prebuilt libs under deps/prebuilt/ are extracted automatically on first build
#   when deps/$(CROSS_HOST)/ is missing (tar from deps/prebuilt/$(CROSS_HOST).tar.gz)
#
# Usage:
#   make CROSS_COMPILE=arm-linux-gnueabihf-

CROSS_COMPILE ?=
CROSS_HOST    ?= arm-linux-gnueabihf
CC            = $(CROSS_COMPILE)gcc

SRC_DIR    = src
COMMON_DIR = $(SRC_DIR)/common
MOD_DIR    = $(SRC_DIR)/modules
BUILD_DIR  = build
DEPS_DIR   = deps
DEPS_PREFIX ?= $(DEPS_DIR)/$(CROSS_HOST)
DEPS_LIBDIR = $(DEPS_PREFIX)/lib
DEPS_INCDIR = $(DEPS_PREFIX)/include
PREBUILT_ARCHIVE = $(DEPS_DIR)/prebuilt/$(CROSS_HOST).tar.gz

CFLAGS  = -Wall -Wextra -O2 -I$(SRC_DIR) -I$(COMMON_DIR) -I$(MOD_DIR)
LDFLAGS =

SRCS = $(SRC_DIR)/main.c \
       $(wildcard $(COMMON_DIR)/*.c) \
       $(wildcard $(MOD_DIR)/*.c)

TARGET = $(BUILD_DIR)/iepro_demo

# Third-party libraries (static link from DEPS_PREFIX; all enabled by default)
CFLAGS += -DWITH_MQTT -DWITH_MODBUS -DWITH_CURL -DWITH_SSL
# Static link: libraries that need OpenSSL must appear before -lssl -lcrypto.
DEPS_LIBS = -Wl,--start-group -lmosquitto -lcurl -lmodbus -lssl -lcrypto -Wl,--end-group
SYS_LIBS  = -lpthread -ldl

CFLAGS  += -I$(DEPS_INCDIR)
LDFLAGS += -L$(DEPS_LIBDIR) -Wl,-Bstatic $(DEPS_LIBS) -Wl,-Bdynamic $(SYS_LIBS)

all: $(TARGET)

.PHONY: deps-prebuilt
deps-prebuilt:
	@if [ ! -d "$(DEPS_LIBDIR)" ]; then \
		echo ">> $(DEPS_PREFIX) not found, extracting prebuilt..."; \
		if [ ! -f "$(PREBUILT_ARCHIVE)" ]; then \
			echo "ERROR: missing $(PREBUILT_ARCHIVE)"; \
			exit 1; \
		fi; \
		tar -xzf "$(PREBUILT_ARCHIVE)" -C "$(DEPS_DIR)"; \
		if [ ! -d "$(DEPS_LIBDIR)" ]; then \
			echo "ERROR: extract failed, $(DEPS_LIBDIR) not found"; \
			exit 1; \
		fi; \
	fi

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRCS) | $(BUILD_DIR) deps-prebuilt
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean deps-prebuilt
