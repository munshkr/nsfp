.PHONY: install

all:
	@mkdir -p build/
	@cd build/ && cmake .. && make

install:
	$(MAKE) -C build/ install

clean:
	$(MAKE) -C build/ clean
