ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<pfad-zu>/devkitpro")
endif

include $(DEVKITPRO)/libnx/switch_rules

TARGET := ../../build/AssetLoader/AssetLoader
OUTPUT := $(TARGET)

SOURCES := source/main.cpp  # ggf. alle plugin sources aufführen
INCLUDES := source
APP_TITLE := AssetLoader
APP_AUTHOR := Smapifan
APP_VERSION := 1.0.0

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)

-include $(OFILES:.o=.d)

clean:
	rm -f $(OUTPUT).* *.o *.d
