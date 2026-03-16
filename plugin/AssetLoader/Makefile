ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path-to>/devkitpro")
endif

include $(DEVKITPRO)/libnx/switch_rules

TARGET := ../../build/AssetLoader/AssetLoader
OUTPUT := $(TARGET)
APP_TITLE := AssetLoader
APP_AUTHOR := Smapifan
APP_VERSION := 1.0.0

SOURCES := source/main.cpp  # ggf. weitere einfügen
INCLUDES := source

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)

-include $(OFILES:.o=.d)

clean:
	rm -f $(OUTPUT).* *.o *.d
