PLUGIN_NAME := AssetLoader
PLUGIN_OUTDIR := ../../build/$(PLUGIN_NAME)
PLUGIN_OUTPUT := $(PLUGIN_OUTDIR)/$(PLUGIN_NAME).bin

# Finde alle .cpp rekursiv im Plugin
SRCS := $(shell find . -type f -name '*.cpp')
OBJS := $(patsubst ./%.cpp,$(PLUGIN_OUTDIR)/%.o,$(SRCS))

CXX := g++
CXXFLAGS := -Wall -Werror -fPIC -std=c++17 $(foreach dir,$(shell find . -type d),-I$(dir))

.PHONY: all clean

all: $(PLUGIN_OUTPUT)

$(PLUGIN_OUTDIR):
	@mkdir -p $(PLUGIN_OUTDIR)

# Kompiliert alle .cpp im Plugin (inkl. dedizierte main.cpp falls vorhanden)
$(PLUGIN_OUTDIR)/%.o: %.cpp | $(PLUGIN_OUTDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link zu .bin
$(PLUGIN_OUTPUT): $(OBJS)
	$(CXX) -shared -o $@ $^

clean:
	rm -rf $(PLUGIN_OUTDIR)
