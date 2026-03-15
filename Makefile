#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#-------------------------------------------------------------------------------
# TARGET   : Name of the output (.nro / .elf)
# BUILD    : Directory for intermediate build files
# SOURCES  : Directories containing C/C++/ASM source files
# DATA     : Directories containing binary data files
# INCLUDES : Additional include directories
# ROMFS    : Directory whose contents are embedded as RomFS (→ romfs:/)
#
# Note: Dear ImGui sources live in ./imgui/ which is NOT tracked by git.
#       Run `make fetch-imgui` or see README.md to obtain them before building.
#-------------------------------------------------------------------------------
TARGET   := PKMswitch
BUILD    := build
SOURCES  := source source/ui source/backends imgui
DATA     := data
INCLUDES := source imgui imgui/backends
ROMFS    := assets

#-------------------------------------------------------------------------------
# App metadata (written into .nacp)
#-------------------------------------------------------------------------------
APP_TITLE   := PKMswitch
APP_AUTHOR  := Smapifan
APP_VERSION := 1.0.0
ICON        := icon.png

#-------------------------------------------------------------------------------
# Compiler / linker flags
#-------------------------------------------------------------------------------
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS   := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
CFLAGS   += $(INCLUDE) -D__SWITCH__

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17

ASFLAGS  := -g $(ARCH)
LDFLAGS   = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
            -Wl,-Map,$(notdir $*.map)

LIBS := -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lSDL2 -lEGL -lglapi -ldrm_nouveau -lnx

#-------------------------------------------------------------------------------
# Library search paths
#-------------------------------------------------------------------------------
LIBDIRS := $(PORTLIBS) $(LIBNX)

#-------------------------------------------------------------------------------
# Guard: require imgui source before building
#-------------------------------------------------------------------------------
ifeq ($(wildcard imgui/imgui.h),)
$(error "Dear ImGui not found in ./imgui/. Run  make fetch-imgui  or:\n\
  git clone --depth=1 --branch v1.91.6 https://github.com/ocornut/imgui.git imgui")
endif

#-------------------------------------------------------------------------------
# Standard devkitPro build machinery (do not edit below this line)
#-------------------------------------------------------------------------------
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
                  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export BUILD_EXEFS_SRC := $(TOPDIR)/$(EXEFS_SRC)

ifeq ($(strip $(ICON)),)
    icons := $(wildcard *.jpg)
    ifneq (,$(findstring $(TARGET).jpg,$(icons)))
        export APP_ICON := $(TOPDIR)/$(TARGET).jpg
    else
        ifneq (,$(findstring icon.jpg,$(icons)))
            export APP_ICON := $(TOPDIR)/icon.jpg
        endif
    endif
else
    export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
    export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
    export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp
endif

ifneq ($(APP_TITLEID),)
    export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
    export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all fetch-imgui

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo Cleaning...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nacp $(TARGET).nro

## Convenience target: clone Dear ImGui at the required version
IMGUI_TAG ?= v1.91.6
fetch-imgui:
	@echo "Fetching Dear ImGui $(IMGUI_TAG)..."
	git clone --depth=1 --branch $(IMGUI_TAG) \
	    https://github.com/ocornut/imgui.git imgui
	@echo "Done. You can now run: make"

else

#-------------------------------------------------------------------------------
# Per-object build rules (inside the BUILD directory)
#-------------------------------------------------------------------------------
DEPENDS := $(OFILES:.o=.d)

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)

$(OFILES_SRC): $(HFILES_BIN)

%.bin.o %_bin.h: %.bin
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
