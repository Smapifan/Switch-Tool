BUILD    := build
TARGET   := $(BUILD)/AssetLoader
SRCDIR   := source

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(patsubst $(SRCDIR)/%.cpp, $(BUILD)/%.o, $(SOURCES))

CXXFLAGS ?= -O2 -Wall -std=c++17
LDFLAGS  ?= -lcurl

.PHONY: all clean

all: $(BUILD) $(TARGET).bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRCDIR)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TARGET).bin: $(TARGET).elf
	objcopy -O binary $< $@

clean:
	rm -rf $(BUILD)
