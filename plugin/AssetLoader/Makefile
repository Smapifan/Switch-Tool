ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

include $(DEVKITPRO)/libnx/switch_rules

TARGET    := AssetLoader
OUTPUT    := $(CURDIR)/$(TARGET)
APP_TITLE := AssetLoader
APP_AUTHOR := Smapifan
APP_VERSION := 1.0.0

SOURCES   := source source/ui source/backends imgui
INCLUDES  := source imgui imgui/backends
ROMFS     := assets
ICON      := assets/icon.png

IMGUI_TAG ?= v1.91.6

.PHONY: fetch-imgui ensure-imgui check-icon all clean

check-icon:
	@echo "Checking for $(ICON)..."
	@if [ ! -f $(ICON) ]; then \
		echo "Icon fehlt, lade es von: https://github.com/Smapifan/Switch-Tool/raw/main/assets/icon.png"; \
		mkdir -p assets; \
		if command -v curl > /dev/null; then \
			curl -L --output $(ICON) https://github.com/Smapifan/Switch-Tool/raw/main/assets/icon.png || { echo "Fehler beim Download!"; exit 1; }; \
		elif command -v wget > /dev/null; then \
			wget -O $(ICON) https://github.com/Smapifan/Switch-Tool/raw/main/assets/icon.png || { echo "Fehler beim Download!"; exit 1; }; \
		else \
			echo "Bitte icon.png manuell in $(ICON) ablegen."; exit 1; \
		fi; \
	fi
	@if [ ! -f $(ICON) ]; then \
		echo "Missing icon file: $(ICON) (Download/source failed)"; \
		exit 2; \
	fi

ensure-imgui:
	@if [ ! -f imgui/imgui.h ]; then \
		echo "Fetching Dear ImGui $(IMGUI_TAG)..."; \
		rm -rf imgui; \
		git clone --depth=1 --branch $(IMGUI_TAG) https://github.com/ocornut/imgui.git imgui; \
	fi

fetch-imgui: ensure-imgui

all: check-icon ensure-imgui $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)

-include $(OFILES:.o=.d)

clean:
	rm -f $(OUTPUT).elf $(OUTPUT).nacp $(OUTPUT).nro *.o *.d
