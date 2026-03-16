ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

include $(DEVKITPRO)/libnx/switch_rules

TARGET    := build/PKMswitch
OUTPUT    := $(TARGET)
APP_TITLE := PKMswitch
APP_AUTHOR := Smapifan
APP_VERSION := 1.0.0

# ==== Abhängigkeiten automatisch holen (z.B. ImGui) ====
IMGUI_TAG ?= v1.91.6

# Hole ImGui, falls noch nicht da
ensure-imgui:
	@if [ ! -f imgui/imgui.h ]; then \
		echo "Cloning Dear ImGui $(IMGUI_TAG)..."; \
		git clone --depth=1 --branch $(IMGUI_TAG) https://github.com/ocornut/imgui.git imgui; \
	fi

# ==== Safety: Nur existierende Quellen-Verzeichnisse ====
SRC_DIRS := source source/ui source/backends imgui
EXISTING_DIRS := $(foreach dir,$(SRC_DIRS),$(if $(wildcard $(dir)),$(dir),))
SOURCES := $(shell find $(EXISTING_DIRS) -type f -name '*.cpp' 2>/dev/null)
INCLUDES := $(foreach dir,$(EXISTING_DIRS),-I$(dir)) -Iimgui/backends
ROMFS := assets
ICON := assets/icon.png

.PHONY: all ensure-imgui clean

all: ensure-imgui $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)

-include $(OFILES:.o=.d)

clean:
	rm -f $(OUTPUT).elf $(OUTPUT).nacp $(OUTPUT).nro *.o *.d
