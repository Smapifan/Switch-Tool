BUILD   := ../../build/AssetLoader
TARGET  := $(BUILD)/AssetLoader
SRCDIR  := source

SOURCES := $(shell find $(SRCDIR) -type f -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILD)/%.o, $(SOURCES))

CXX     := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++
CC      := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-gcc
STRIP   := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-strip

CXXFLAGS := -mcpu=cortex-a57+fp+simd+crypto -mtune=cortex-a57 \
  -I$(DEVKITPRO)/libnx/include \
  -I$(DEVKITPRO)/portlibs/switch/include \
  -I$(SRCDIR) \
  -O2 -Wall -std=c++17 -fPIC

LDFLAGS := -L$(DEVKITPRO)/libnx/lib -L$(DEVKITPRO)/portlibs/switch/lib -lcurl -lz -lnx \
  -Wl,-T$(DEVKITPRO)/libnx/switch.armv8.ld

.PHONY: all clean dirs

all: dirs $(TARGET).nro

dirs:
	@mkdir -p $(BUILD)

$(BUILD)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TARGET).nro: $(TARGET).elf
	nxlink -s 0x010000000000100D $< -o $@

clean:
	rm -rf $(BUILD)
