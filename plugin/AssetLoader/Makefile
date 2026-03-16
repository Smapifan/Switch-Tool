PLUGIN_NAME := AssetLoader
PLUGIN_OUTDIR := $(BUILD_ROOT)/$(PLUGIN_NAME)
PLUGIN_OUTPUT := $(PLUGIN_OUTDIR)/$(PLUGIN_NAME).bin

SRC_DIR := .
SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp')
OBJS := $(patsubst ./%.cpp,$(PLUGIN_OUTDIR)/%.o,$(SRCS))

CXX := g++  # Oder Cross-Compiler entsprechend Zielplattform
CXXFLAGS := -Wall -Werror -fPIC -std=c++17 $(foreach dir,$(shell find . -type d),-I$(dir))

.PHONY: all clean

all: $(PLUGIN_OUTPUT)

$(PLUGIN_OUTDIR):
	@mkdir -p $(PLUGIN_OUTDIR)

# Kompiliere alle Files relativ ins Build/AssetLoader-Ordner
$(PLUGIN_OUTDIR)/%.o: %.cpp | $(PLUGIN_OUTDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PLUGIN_OUTPUT): $(OBJS)
	$(CXX) -shared -o $@ $^

clean:
	rm -rf $(PLUGIN_OUTDIR)
