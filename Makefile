.PHONY: all plugin clean

all: plugin

plugin:
	$(MAKE) -C plugin/AssetLoader

clean:
	$(MAKE) -C plugin/AssetLoader clean
