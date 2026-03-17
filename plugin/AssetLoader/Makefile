BUILD    := ../../build/AssetLoader
TARGET   := $(BUILD)/AssetLoader
SRCDIR   := source

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(patsubst $(SRCDIR)/%.cpp, $(BUILD)/%.o, $(SOURCES))

CXX      := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++
CC       := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-gcc

CXXFLAGS := -mcpu=cortex-a57+fp+simd+crypto -mtune=cortex-a57 \
  -I$(DEVKITPRO)/portlibs/switch/include \
  -I$(SRCDIR) \
  -O2 -Wall -std=c++17 -fPIC

LDFLAGS  := -L$(DEVKITPRO)/portlibs/switch/lib -lcurl -lz

.PHONY: all clean dirs

all: dirs $(TARGET).bin

dirs:
	@mkdir -p $(BUILD)

# Jetzt von source/*.cpp zu build/*.o
$(BUILD)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TARGET).bin: $(TARGET).elf
	aarch64-none-elf-objcopy -O binary $< $@

clean:
	rm -rf $(BUILD)
