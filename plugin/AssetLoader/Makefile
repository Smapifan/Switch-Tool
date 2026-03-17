BUILD   := ../../build/AssetLoader
TARGET  := $(BUILD)/AssetLoader
SRCDIR  := source

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILD)/%.o,$(SOURCES))

CXX     := aarch64-linux-gnu-g++
CC      := aarch64-linux-gnu-gcc

CXXFLAGS := -I/usr/include \
  -I/usr/include/curl \
  -I$(SRCDIR) \
  -O2 -Wall -std=c++17

LDFLAGS := -lcurl

.PHONY: all clean dirs

all: dirs $(TARGET).bin

dirs:
	@mkdir -p $(BUILD)

$(BUILD)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TARGET).bin: $(TARGET).elf
	aarch64-linux-gnu-objcopy -O binary $< $@

clean:
	rm -rf $(BUILD)
