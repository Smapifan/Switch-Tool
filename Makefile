.PHONY: all plugins clean

BUILD_DIR := build

all: $(BUILD_DIR)/PKMswitch.nro plugins

$(BUILD_DIR)/PKMswitch.nro:
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -f Makefile.main PKMswitch_nro_outdir=$(BUILD_DIR) PKMswitch_nro_name=PKMswitch

plugins:
	$(MAKE) -C plugin/AssetLoader BUILD_ROOT=$(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
