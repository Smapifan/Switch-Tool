.DEFAULT_GOAL := all

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

TARGET   := PKMswitch
BUILD    := build
SOURCES  := source source/ui source/backends imgui
DATA     := data
INCLUDES := source imgui imgui/backends
ROMFS    := assets

APP_TITLE   := PKMswitch
APP_AUTHOR  := Smapifan
APP_VERSION := 1.0.0

ICON := assets/icon.png
ICON_URL := https://github.com/Smapifan/Switch-Tool/raw/main/assets/icon.png

.PHONY: check-icon ensure-imgui fetch-imgui clean all

check-icon:
	@echo "Checking for $(ICON)..."
	@if [ ! -f $(ICON) ]; then \
		echo "Icon fehlt, lade es von: $(ICON_URL)"; \
		mkdir -p assets; \
		if command -v curl > /dev/null; then \
			curl -L --output $(ICON) $(ICON_URL) || { echo "Fehler beim Download!"; exit 1; }; \
		elif command -v wget > /dev/null; then \
			wget -O $(ICON) $(ICON_URL) || { echo "Fehler beim Download!"; exit 1; }; \
		else \
			echo "Bitte icon.png manuell in $(ICON) ablegen."; exit 1; \
		fi \
	fi
	@if [ ! -f $(ICON) ]; then \
		echo "Missing icon file: $(ICON) (Download/source failed)"; \
		exit 2; \
	fi

IMGUI_TAG ?= v1.91.6
ensure-imgui:
	@if [ ! -f imgui/imgui.h ]; then \
		echo "Fetching Dear ImGui $(IMGUI_TAG)..."; \
		rm -rf imgui; \
		git clone --depth=1 --branch $(IMGUI_TAG) https://github.com/ocornut/imgui.git imgui; \
	fi

all: check-icon ensure-imgui $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	$(MAKE) -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo Cleaning...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nacp $(TARGET).nro

fetch-imgui: ensure-imgui

else

DEPENDS := $(OFILES:.o=.d)

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
