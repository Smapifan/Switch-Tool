ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path-to>/devkitpro")
endif

export ARCH := aarch64
export CC   := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-gcc
export CXX  := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++
export AR   := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-ar

BUILD   := ../../build/AssetLoader
TARGET  := $(BUILD)/AssetLoader
SRCDIR  := source

# Sammelt alle .cpp im Plugin-Ordner
SOURCES := $(shell find $(SRCDIR) -type f -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILD)/%.o, $(SOURCES))

CXXFLAGS := -mcpu=cortex-a57+fp+simd+crypto -mtune=cortex-a57 \
  -I$(DEVKITPRO)/libnx/include \
  -I$(DEVKITPRO)/portlibs/switch/include \
  -I$(SRCDIR) \
  -O2 -Wall -std=c++17 -fPIC

LDFLAGS := -L$(DEVKITPRO)/libnx/lib -lnx -L$(DEVKITPRO)/portlibs/switch/lib \
           -lSDL2 -limgui -lcurl

.PHONY: all clean dirs

all: dirs $(TARGET).bin

dirs:
	@mkdir -p $(BUILD)

# Kompiliert alle Source-Files ins BUILD-Verzeichnis
$(BUILD)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Linkt alle Object-Files zu einem (dynamischen) .bin
$(TARGET).bin: $(OBJECTS)
	$(CXX) -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BUILD)
