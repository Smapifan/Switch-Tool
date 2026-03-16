ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

BUILD := build
TARGET := AssetLoader
SRC := source/AssetLoader.c
OUT := $(BUILD)/AssetLoader.bin

CC := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-gcc
CFLAGS := -g -O2 -I$(DEVKITPRO)/libnx/include -D__SWITCH__

.PHONY: all clean

# Baue build-Ordner falls nicht vorhanden
$(BUILD):
	@[ -d $@ ] || mkdir -p $@

# Kompiliere AssetLoader.c und mache daraus AssetLoader.bin
$(OUT): $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $(BUILD)/AssetLoader.o $<
	# Konvertiere das Objekt zu Binärfile (kann z.B. raw Data sein oder einfach kopiert werden)
	cp $(BUILD)/AssetLoader.o $(OUT)

all: $(OUT)

clean:
	@echo Cleaning AssetLoader plugin...
	@rm -rf $(BUILD)
