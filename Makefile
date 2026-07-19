CC = gcc

PROJECT_DIR := $(shell pwd)

NORMAL_CFLAGS := -Wall -Wextra -I/usr/include/yaml
DEBUG_CFLAGS  := $(NORMAL_CFLAGS) -g -DDEBUG

GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0 gobject-2.0 glib-2.0 gdk-pixbuf-2.0)
GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0 gobject-2.0 glib-2.0 gdk-pixbuf-2.0) -lyaml

CLI_TARGET = start_sensors

CLI_SRC = src/app/ss_main.c src/backend/ss_can.c src/backend/ss_core.c src/controller/ss_controller.c \
		  src/infra/error/ss_error_message.c src/infra/utils/ss_utils.c src/infra/log/ss_logger.c \
		  src/ui/ss_app_context.c src/ui/ss_ui_error.c src/ui/ss_signal.c \
		  src/ui/ss_dashboard.c src/ui/ss_notify.c src/ui/ss_main.c src/ui/ss_style.c \
		  src/ui/ss_history_notify.c # src/ui/ss_header_settings.c
		  

CFLAGS_COMMON = $(GTK_CFLAGS) \
	-DSTART_SENSORS_PATH=\"$(PROJECT_DIR)\" \
	-DSS_PATH_GUI=\"$(PROJECT_DIR)/src/ui\"

LDFLAGS_COMMON = $(GTK_LIBS)

all: build/$(CLI_TARGET)

build-dir:
	mkdir -p build

build/$(CLI_TARGET): $(CLI_SRC) | build-dir
	$(CC) $(NORMAL_CFLAGS) $(CFLAGS_COMMON) \
	$(CLI_SRC) -o $@ $(LDFLAGS_COMMON)

debug: | build-dir
	$(CC) $(DEBUG_CFLAGS) $(CFLAGS_COMMON) \
	$(CLI_SRC) -o build/$(CLI_TARGET) $(LDFLAGS_COMMON)

clean:
	rm -f build/$(CLI_TARGET)

.PHONY: all clean debug build-dir
