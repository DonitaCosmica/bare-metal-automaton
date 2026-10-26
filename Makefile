CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
SIZE    = $(CROSS)size

CFLAGS  = -mcpu=cortex-m4 -mthumb -Os -g -Wall -Wextra -ffreestanding
LDFLAGS = -nostdlib -T linker/ra4m1.ld -Wl,--gc-sections -Wl,-Map=build/main.map

all: build/main.bin

build/main.elf: src/main.c linker/ra4m1.ld | build
	$(CC) $(CFLAGS) src/main.c $(LDFLAGS) -o $@
	$(SIZE) $@

build/main.bin: build/main.elf
	$(OBJCOPY) -O binary $< $@

build:
	mkdir -p build

flash: build/main.elf
	pyocd flash -t r7fa4m1ab --base-address 0x4000 $<

clean:
	rm -rf build

.PHONY: all flash clean