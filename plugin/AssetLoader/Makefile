ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

include $(DEVKITPRO)/libnx/switch_rules

TARGET := AssetLoader
BUILD := build
SOURCES := source
INCLUDES := include ../../imgui ../../imgui/backends

APP_TITLE := PKMswitch AssetLoader
APP_AUTHOR := PKMswitch Contributors
APP_VERSION := 1.0

ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

DEFINES := -DIMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS \
           -DIMGUI_DISABLE_DEFAULT_CLIPBOARD_FUNCTIONS

CFLAGS := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
CFLAGS += $(INCLUDE) -D__SWITCH__
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-rtti -fno-exceptions
ASFLAGS := -g $(ARCH)

LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS := -lcurl -lz -lmbedtls -lmbedx509 -lmbedcrypto \
        -lSDL2 -lEGL -lglapi -ldrm_nouveau -lnx

LIBDIRS := $(PORTLIBS) $(LIBNX)

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  -I$(DEVKITPRO)/libnx/include \
                  -I$(DEVKITPRO)/portlibs/switch/include \
                  -I$(DEVKITPRO)/portlibs/switch/include/SDL2 \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo Cleaning plugin...
	@rm -rf $(BUILD)
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
