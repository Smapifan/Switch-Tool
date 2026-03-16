ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

# Pfade
BUILD := build
SOURCES := source
TARGET := AssetLoader
OUTFILE := $(BUILD)/AssetLoader.bin

# Erzeuge Build-Ordner falls nötig
$(BUILD):
	@[ -d $@ ] || mkdir -p $@

# Baue aus allen Quellen eine Binärdatei (hier: einfach alle .o zusammenkopieren; passe bei Bedarf an!)
SRCS := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
OBJS := $(SRCS:.c=.o)

# Kompilieren (mit libnx, falls nötig, ansonsten für Data-Plugins meist normal)
# Füge hier deine Compiler-Flags/Linker-Flags ein, falls AssetLoader wirklich Code enthält
CFLAGS := -g -O2 -I$(DEVKITPRO)/libnx/include -D__SWITCH__

$(BUILD)/%.o: $(SOURCES)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTFILE): $(OBJS) | $(BUILD)
	# Für reines Data-Plugin: einfach zum Bin zusammenfassen
	cat $(OBJS) > $@

all: $(OUTFILE)

clean:
	@echo Cleaning plugin...
	@rm -rf $(BUILD)
