BUILD   := build
TARGET  := $(BUILD)/pokemon-id-exporter
SRCDIR  := source

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILD)/%.o,$(SOURCES))

CXX      := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++
CXXFLAGS := -mcpu=cortex-a57+fp+simd+crypto -mtune=cortex-a57 \
  -I$(DEVKITPRO)/libnx/include \
  -I$(SRCDIR) \
  -O2 -Wall -std=c++17

LDFLAGS := -L$(DEVKITPRO)/libnx/lib -lnx

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
	elf2nro $< $@

clean:
	rm -rf $(BUILD)
