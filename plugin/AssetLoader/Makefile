PLUGIN_NAME := AssetLoader
SRC_DIR := source
BUILD_DIR := build
OUT := $(BUILD_DIR)/$(PLUGIN_NAME).so

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# =========== Zentrale Include-Dirs (für alle externen und system-Header) ==========
INCLUDE_DIRS := -I$(SRC_DIR) -Iinclude

# Für Switch Homebrew Entwickung (libnx, switch.h)
INCLUDE_DIRS += -I/opt/devkitpro/libnx/include

# Für SDL2
INCLUDE_DIRS += -I/usr/include/SDL2
# Falls Windows: INCLUDE_DIRS += -IC:/msys64/mingw64/include/SDL2

# Für ImGui und ImGui-Backends (Passe ggf. an: z.B. third-party/imgui)
INCLUDE_DIRS += -Ithird-party/imgui
INCLUDE_DIRS += -Ithird-party/imgui/backends

# Für libcurl
INCLUDE_DIRS += -I/usr/include

# =========== Linker-Flags für dynamische Libraries hinzufügen =============
LDLIBS := -lSDL2 -lcurl -ldl
LDLIBS += -lSDL2main            # Falls nötig, ggf. entfernen, wenn Problem
# Bei Link-Problemen mit ImGui, libnx, ... ggf. weitere -l Einträge!

CXXFLAGS := -Wall -Werror -fPIC -std=c++17 $(INCLUDE_DIRS)
LDFLAGS := -shared $(LDLIBS)

.PHONY: all clean check

ifeq ($(strip $(SRCS)),)
$(error Keine .cpp-Dateien in $(SRC_DIR) gefunden! Bitte mindestens eine C++-Datei anlegen.)
endif

all: $(OUT) check

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

check:
	@if [ ! -f $(OUT) ]; then \
		echo "Build fehlgeschlagen: $(OUT) fehlt!"; exit 1; \
	fi; \
	filesize=$$(stat -c%s "$(OUT)"); \
	if [ "$$filesize" -lt 128 ]; then \
		echo "Build fehlgeschlagen: $(OUT) zu klein!"; exit 1; \
	fi; \
	echo "Build erfolgreich: $(OUT), Größe: $$filesize Bytes."

clean:
	rm -rf $(BUILD_DIR)
