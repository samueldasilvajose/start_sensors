# -------------------------------
# Compilador
# -------------------------------
CC = gcc

# -------------------------------
# Diretório do projeto
# -------------------------------
PROJECT_DIR := $(shell pwd)

# -------------------------------
# Flags padrão do compilador
# -------------------------------
NORMAL_CFLAGS := -Wall -Wextra -I/usr/include/yaml
DEBUG_CFLAGS  := $(NORMAL_CFLAGS) -g -DDEBUG

# -------------------------------
# GTK 3 + GLib/GObject + YAML (pkg-config)
# -------------------------------
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0 gobject-2.0 glib-2.0 gdk-pixbuf-2.0)
GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0 gobject-2.0 glib-2.0 gdk-pixbuf-2.0) -lyaml

# -------------------------------
# Executável
# -------------------------------
CLI_TARGET = start_sensors

# -------------------------------
# Arquivos fonte
# -------------------------------
CLI_SRC = start_sensors_main.c start_sensors_can.c start_sensors_error.c start_sensors_utils.c start_sensors.c \
	gui/ss_app_context.c gui/ss_body.c gui/ss_controller.c gui/ss_error.c gui/ss_header.c gui/ss_main.c gui/ss_signal.c gui/ss_style.c

# -------------------------------
# Alvo padrão (compilação normal)
# -------------------------------
all: $(CLI_TARGET)

# -------------------------------
# Compila CLI normal
# -------------------------------
$(CLI_TARGET): $(CLI_SRC)
	$(CC) $(NORMAL_CFLAGS) $(GTK_CFLAGS) -DSTART_SENSORS_PATH=\"$(PROJECT_DIR)\" -DSS_PATH_GUI=\"$(PROJECT_DIR)/gui\" $(CLI_SRC) -o $@ $(GTK_LIBS)

# -------------------------------
# Compila em modo debug
# -------------------------------
debug: $(CLI_SRC)
	$(CC) $(DEBUG_CFLAGS) $(GTK_CFLAGS) -DSTART_SENSORS_PATH=\"$(PROJECT_DIR)\" -DSS_PATH_GUI=\"$(PROJECT_DIR)/gui\" $(CLI_SRC) -o $(CLI_TARGET) $(GTK_LIBS)

# -------------------------------
# Limpa executáveis
# -------------------------------
clean:
	rm -f $(CLI_TARGET)

# -------------------------------
# Recompila tudo
# -------------------------------
rebuild: clean all

# -------------------------------
# Alvos especiais
# -------------------------------
.PHONY: all clean rebuild debug