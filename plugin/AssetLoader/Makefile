BUILD   := ../../build/AssetLoader
TARGET  := $(BUILD)/AssetLoader
SRCDIR  := source

SOURCES := $(shell find $(SRCDIR) -type f -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILD)/%.o, $(SOURCES))

# Benutze explizit devkitA64-Compiler, nicht das System-g++
ifeq ($(DEVKITPRO),)
$(error "Please source your devkitPro environment (z.B. source /opt/devkitpro/switchvars.sh)!")
endif

CXX     := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++
CC      := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-gcc
STRIP   := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-strip

CXXFLAGS := -mcpu=cortex-a57+fp+simd+crypto -mtune=cortex-a57 \
  -I$(DEVKITPRO)/portlibs/switch/include \
  -I$(SRCDIR) \
  -O2 -Wall -std=c++17 -fPIC

# Linker NUR curl & Portlibs, KEIN libnx, KEIN SDL2, KEIN ImGui!
LDFLAGS := -L$(DEVKITPRO)/portlibs/switch/lib -lcurl

.PHONY: all clean dirs

all: dirs $(TARGET).bin

dirs:
	@mkdir -p $(BUILD)

$(BUILD)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).bin: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BUILD)
