PLUGIN_NAME := AssetLoader
PLUGIN_OUTDIR := ../../build/$(PLUGIN_NAME)
PLUGIN_OUTPUT := $(PLUGIN_OUTDIR)/$(PLUGIN_NAME).bin

# Finde alle .cpp-Dateien REKURSIV in diesem Plugin-Verzeichnis
SRC_DIR := .
SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp')
OBJS := $(patsubst ./%.cpp,$(PLUGIN_OUTDIR)/%.o,$(SRCS))

CXX := g++ # Für native Tools, ggf. anpassen für Cross-Compile
CXXFLAGS := -Wall -Werror -fPIC -std=c++17 $(foreach dir,$(shell find . -type d),-I$(dir))

.PHONY: all clean

all: $(PLUGIN_OUTPUT)

$(PLUGIN_OUTDIR):
	@mkdir -p $(PLUGIN_OUTDIR)

$(PLUGIN_OUTDIR)/%.o: %.cpp | $(PLUGIN_OUTDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PLUGIN_OUTPUT): $(OBJS)
	$(CXX) -shared -o $@ $^

clean:
	rm -rf $(PLUGIN_OUTDIR)
