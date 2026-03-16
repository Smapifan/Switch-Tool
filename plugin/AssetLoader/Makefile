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

# Baue den build-Ordner, falls nicht vorhanden
$(BUILD):
	@[ -d $@ ] || mkdir -p $@

# Kompiliere und erzeuge AssetLoader.bin
$(OUT): $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $(BUILD)/AssetLoader.o $<
	# Kopiere das Objekt oder erzeuge ein reines Binärfile (je nach Bedarf)
	cp $(BUILD)/AssetLoader.o $(OUT)

all: $(OUT)

clean:
	@echo Cleaning AssetLoader plugin...
	@rm -rf $(BUILD)
