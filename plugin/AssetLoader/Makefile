# C++ Plugin-Build Makefile für AssetLoader

PLUGIN_NAME := AssetLoader
SRC_DIR := source
BUILD_DIR := build
OUT := $(BUILD_DIR)/$(PLUGIN_NAME).so

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

CXXFLAGS := -Wall -Werror -fPIC -std=c++17
LDFLAGS := -shared

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
	if [ "$$filesize" -lt 1024 ]; then \
		echo "Build fehlgeschlagen: $(OUT) zu klein!"; exit 1; \
	fi; \
	echo "Build erfolgreich: $(OUT), Größe: $$filesize Bytes."

clean:
	rm -rf $(BUILD_DIR)
