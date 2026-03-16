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

# Der korrekte Download-Link ("raw")
ICON := assets/icon.png
ICON_URL := https://github.com/Smapifan/Switch-Tool/raw/main/assets/icon.png

.PHONY: check-icon
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

ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

DEFINES  := -DIMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS \
            -DIMGUI_DISABLE_DEFAULT_CLIPBOARD_FUNCTIONS

CFLAGS   := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
CFLAGS   += $(INCLUDE) -D__SWITCH__
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
ASFLAGS  := -g $(ARCH)
LDFLAGS   = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
            -Wl,-Map,$(notdir $*.map)

LIBS := -lcurl -lz -lmbedtls -lmbedx509 -lmbedcrypto \
        -lSDL2 -lEGL -lglapi -ldrm_nouveau -lnx

LIBDIRS := $(PORTLIBS) $(LIBNX)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(CURDIR)/$(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(CURDIR)/$(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(CURDIR)/$(dir)/*.s)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(CURDIR)/$(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
    export LD := $(CC)
else
    export LD := $(CXX)
endif

export OFILES_BIN   := $(addsuffix .o,$(BINFILES))
export OFILES_SRC   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES       := $(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN   := $(patsubst %.bin.o,%.bin.h,$(filter %.bin.o,$(OFILES_BIN)))

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include)     \
                  -I$(DEVKITPRO)/portlibs/switch/include         \
                  -I$(DEVKITPRO)/portlibs/switch/include/SDL2    \
                  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export BUILD_EXEFS_SRC := $(TOPDIR)/$(EXEFS_SRC)

export APP_ICON := $(TOPDIR)/$(ICON)
export NROFLAGS += --icon=$(APP_ICON)

ifeq ($(strip $(NO_NACP)),)
    export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp
endif

ifneq ($(APP_TITLEID),)
    export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
    export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all fetch-imgui ensure-imgui

IMGUI_TAG ?= v1.91.6
ensure-imgui:
	@if [ ! -f imgui/imgui.h ]; then \
		echo "Fetching Dear ImGui $(IMGUI_TAG)..."; \
		rm -rf imgui; \
		git clone --depth=1 --branch $(IMGUI_TAG) https://github.com/ocornut/imgui.git imgui; \
		echo "Done. You can now run: make"; \
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
