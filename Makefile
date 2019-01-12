#
# Top makefile
#

.PHONY: all clean


all:
	@$(MAKE) -C src all

flash:
	../sunxi-tools/sunxi-fel -p spiflash-write 0 output/xboot.bin

clean:
	@$(MAKE) -C src clean
