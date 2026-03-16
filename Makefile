ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif
include $(DEVKITPRO)/libnx/switch_rules

TARGET    := build/PKMswitch
OUTPUT    := $(TARGET)
APP_TITLE := PKMswitch
APP_AUTHOR := Smapifan
APP_VERSION := 1.0.0

# Explizit main.cpp als Source - das ist der EINTRAG, der alles robust macht!
SOURCES   := source/main.cpp             # <-- Die wichtigste Zeile!
INCLUDES  := source imgui imgui/backends
ROMFS     := assets
ICON      := assets/icon.png

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)

-include $(OFILES:.o=.d)

clean:
	rm -f $(OUTPUT).elf $(OUTPUT).nacp $(OUTPUT).nro *.o *.d
