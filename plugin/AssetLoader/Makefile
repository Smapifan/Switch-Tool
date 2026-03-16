# AssetLoader Plugin Build Makefile für Nintendo Switch (libnx/devkitPro)

PLUGIN_NAME := AssetLoader
SRC_DIR := source
BUILD_DIR := build
OUT := $(BUILD_DIR)/$(PLUGIN_NAME).so

# --- devkitPro/libnx/Pfade ---
DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitA64
LIBNX     ?= $(DEVKITPRO)/libnx

# --- Quell- und Objektdateien ---
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# --- Externe Includes ---
INCLUDE_DIRS := -I$(SRC_DIR) -Iinclude
INCLUDE_DIRS += -I$(LIBNX)/include
INCLUDE_DIRS += -I/usr/include/SDL2
INCLUDE_DIRS += -Ithird-party/imgui
INCLUDE_DIRS += -Ithird-party/imgui/backends

# --- Compiler (für Nintendo Switch!) ---
CXX := $(DEVKITARM)/bin/aarch64-none-elf-g++
CXXFLAGS := -Wall -Werror -fPIC -std=gnu++17 -march=armv8-a+crc -mtune=cortex-a57 $(INCLUDE_DIRS)

# --- Linker-Flags (libnx, SDL2, curl, ImGui, etc.) ---
LDFLAGS := -shared -specs=$(LIBNX)/switch.specs -lnx -lSDL2 -lcurl -ldl

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
