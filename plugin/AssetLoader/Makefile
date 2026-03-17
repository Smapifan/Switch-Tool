BUILD   := ../../build/AssetLoader
TARGET  := $(BUILD)/AssetLoader
SRCDIR  := source

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILD)/%.o,$(SOURCES))

# Pfade für ARM64 Cross Curl richtig!
CXX      := aarch64-linux-gnu-g++
CXXFLAGS := -I/usr/aarch64-linux-gnu/include -I/usr/aarch64-linux-gnu/include/curl \
	-I$(SRCDIR) -O2 -Wall -std=c++17

LDFLAGS  := -L/usr/aarch64-linux-gnu/lib -lcurl

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
