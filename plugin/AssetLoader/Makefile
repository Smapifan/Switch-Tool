# Plugin-Build Makefile für AssetLoader

PLUGIN_NAME := AssetLoader
SRC_DIR := source
BUILD_DIR := build
OUT := $(BUILD_DIR)/$(PLUGIN_NAME).so

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

CFLAGS := -Wall -Werror -fPIC
LDFLAGS := -shared

.PHONY: all clean check

# Fehler abfangen: keine Quellen vorhanden
ifeq ($(strip $(SRCS)),)
$(error Keine .c-Dateien in $(SRC_DIR) gefunden! Bitte mindestens eine C-Datei anlegen.)
endif

all: $(OUT) check

# Erstelle Build-Verzeichnis falls es fehlt
$(BUILD_DIR):
	mkdir -p $@

# Kompiliere jede .c-Datei zu .o ins build/-Verzeichnis
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Linke alle Objekte zu AssetLoader.so
$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Sicherheitscheck nach Build: Existenz und Mindestgröße
check:
	@if [ ! -f $(OUT) ]; then \
		echo "Build fehlgeschlagen: $(OUT) fehlt!"; exit 1; \
	fi; \
	filesize=$$(stat -c%s "$(OUT)"); \
	if [ "$$filesize" -lt 1000 ]; then \
		echo "Build fehlgeschlagen: $(OUT) zu klein!"; exit 1; \
	fi; \
	echo "Build erfolgreich: $(OUT), Größe: $$filesize Bytes."

clean:
	rm -rf $(BUILD_DIR)
