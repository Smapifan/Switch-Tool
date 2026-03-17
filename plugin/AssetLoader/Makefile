BUILD   := ../../build/AssetLoader
TARGET  := $(BUILD)/AssetLoader
SRCDIR  := source

SOURCES := $(shell find $(SRCDIR) -type f -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILD)/%.o, $(SOURCES))

CXX     := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++
CC      := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-gcc

CXXFLAGS := -mcpu=cortex-a57+fp+simd+crypto -mtune=cortex-a57 \
  -I$(DEVKITPRO)/portlibs/switch/include \
  -I$(SRCDIR) \
  -O2 -Wall -std=c++17 -fPIC

LDFLAGS := -L$(DEVKITPRO)/portlibs/switch/lib -lcurl -lz

.PHONY: all clean dirs

all: dirs $(TARGET).elf

dirs:
	@mkdir -p $(BUILD)

$(BUILD)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BUILD)
