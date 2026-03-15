#---------------------------------------------------------------------------------
# PKMswitch AssetLoader Plugin — Makefile
# Requires devkitPro with libnx, SDL2 portlibs, and curl portlib.
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
  $(error "Please set DEVKITPRO in your environment.")
endif

include $(DEVKITPRO)/libnx/switch_rules

APP_TITLE   := PKMswitch AssetLoader
APP_AUTHOR  := PKMswitch Contributors
APP_VERSION := 1.0

TARGET  := AssetLoader
SOURCES := source
DATA    :=
INCLUDES:= include

#---------------------------------------------------------------------------------
# Compiler / linker flags
#---------------------------------------------------------------------------------
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS   := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
CFLAGS   += $(INCLUDE) -D__SWITCH__
CXXFLAGS := $(CFLAGS) -std=c++17 -fno-rtti -fno-exceptions

ASFLAGS  := -g $(ARCH)

LDFLAGS  = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS     := -lcurl -lmbedtls -lmbedx509 -lmbedcrypto \
            `sdl2-config --libs` \
            -limgui \
            -lEGL -lglapi -ldrm_nouveau \
            -lnx

#---------------------------------------------------------------------------------
# Source / object lists (auto-discovered + ImGui)
#---------------------------------------------------------------------------------
IMGUI_DIR := $(CURDIR)/../../imgui

CFILES   := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
CPPFILES := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp)) \
            $(IMGUI_DIR)/imgui.cpp \
            $(IMGUI_DIR)/imgui_draw.cpp \
            $(IMGUI_DIR)/imgui_tables.cpp \
            $(IMGUI_DIR)/imgui_widgets.cpp \
            $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
            $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp
OFILES   := $(addprefix build/,$(notdir $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)))

INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
            -I$(DEVKITPRO)/portlibs/switch/include \
            -I$(DEVKITPRO)/portlibs/switch/include/SDL2 \
            -I$(DEVKITPRO)/libnx/include \
            -I$(IMGUI_DIR) \
            -I$(IMGUI_DIR)/backends

LIBDIRS  := $(DEVKITPRO)/portlibs/switch \
            $(DEVKITPRO)/libnx

#---------------------------------------------------------------------------------
.PHONY: all clean

all: build/$(TARGET).nro

build/$(TARGET).nro: build/$(TARGET).elf
	@elf2nro $< $@ --icon=$(DEVKITPRO)/libnx/default_icon.jpg \
	              --nacp=$(TARGET).nacp
	@echo "Built $@"

build/$(TARGET).elf: $(OFILES) | build
	@$(CXX) $(LDFLAGS) -o $@ $^ $(foreach dir,$(LIBDIRS),-L$(dir)/lib) $(LIBS)

build/%.o: source/%.cpp | build
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: source/%.c | build
	@$(CC) $(CFLAGS) -c $< -o $@

build/%.o: $(IMGUI_DIR)/%.cpp | build
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: $(IMGUI_DIR)/backends/%.cpp | build
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	@mkdir -p $@

clean:
	@rm -rf build $(TARGET).nacp
