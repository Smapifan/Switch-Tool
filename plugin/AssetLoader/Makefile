PLUGIN_NAME := AssetLoader
PLUGIN_OUTDIR := $(BUILD_ROOT)/$(PLUGIN_NAME)
PLUGIN_OUTPUT := $(PLUGIN_OUTDIR)/$(PLUGIN_NAME).bin

SRC_DIR := source
# explizit .cpp-Dateien listen:
SRCS := $(SRC_DIR)/main.cpp $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(PLUGIN_OUTDIR)/%.o,$(SRCS))

CXX := g++  # Anpassbar für Ziel
CXXFLAGS := -Wall -Werror -fPIC -std=c++17 -I$(SRC_DIR)

.PHONY: all clean

all: $(PLUGIN_OUTPUT)

$(PLUGIN_OUTDIR):
	@mkdir -p $(PLUGIN_OUTDIR)

$(PLUGIN_OUTDIR)/%.o: $(SRC_DIR)/%.cpp | $(PLUGIN_OUTDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PLUGIN_OUTPUT): $(OBJS)
	$(CXX) -shared -o $@ $^

clean:
	rm -rf $(PLUGIN_OUTDIR)
