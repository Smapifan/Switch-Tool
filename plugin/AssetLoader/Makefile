#---------------------------------------------------------------------------------
# PKMswitch AssetLoader Plugin — Makefile (Switch/libnx)
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

TARGET   := AssetLoader
BUILD    := build
SOURCES  := source
INCLUDES := include ../../imgui ../../imgui/backends

APP_TITLE   := PKMswitch AssetLoader
APP_AUTHOR  := PKMswitch Contributors
APP_VERSION := 1.0

ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

# Keep POSIX-y helpers disabled
DEFINES := -DIMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS \
           -DIMGUI_DISABLE_DEFAULT_CLIPBOARD_FUNCTIONS

CFLAGS   := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
CFLAGS   += $(INCLUDE) -D__SWITCH__
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-rtti -fno-exceptions
ASFLAGS  := -g $(ARCH)

LDFLAGS  = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

# IMPORTANT:
# - no -limgui (we compile imgui sources directly)
# - add -lz for curl
LIBS := -lcurl -lz -lmbedtls -lmbedx509 -lmbedcrypto \
        -lSDL2 -lEGL -lglapi -ldrm_nouveau -lnx

LIBDIRS := $(PORTLIBS) $(LIBNX)

#-------------------------------------------------------------------------------
# devkitPro build machinery
#-------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(BUILD)/$(TARGET)
export TOPDIR := $(CURDIR)

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                $(CURDIR)/../../imgui \
                $(CURDIR)/../../imgui/backends

export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(notdir $(wildcard $(CURDIR)/source/*.c))
CPPFILES := $(notdir $(wildcard $(CURDIR)/source/*.cpp)) \
            imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp \
            imgui_impl_sdl2.cpp imgui_impl_sdlrenderer2.cpp

SFILES   := $(notdir $(wildcard $(CURDIR)/source/*.s))

ifeq ($(strip $(CPPFILES)),)
  export LD := $(CC)
else
  export LD := $(CXX)
endif

export OFILES_SRC := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES     := $(OFILES_SRC)

# Include paths (make switch.h resolvable ALWAYS)
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

else

DEPENDS := $(OFILES:.o=.d)

.PHONY: all

# Build an NRO for now (your dist script copies/renames to AssetLoader.bin).
# If you truly need a raw .bin later, we can add an objcopy step.
all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp

$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
