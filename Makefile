CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
SIZE    = $(CROSS)size
PYOCD  ?= pyocd

TARGET     = main
BUILD      = build
SRC        = $(wildcard src/*.c)
OBJ        = $(SRC:src/%.c=$(BUILD)/%.o)
DEP        = $(OBJ:.o=.d)

CHIP       = r7fa4m1ab
FLASH_ADDR = 0x4000

CFLAGS  = -mcpu=cortex-m4 -mthumb -Os -g -Wall -Wextra -ffreestanding \
          -ffunction-sections -fdata-sections -fno-tree-loop-distribute-patterns \
          -Iinclude -MMD -MP

LDFLAGS = -mcpu=cortex-m4 -mthumb -nostdlib -T linker/ra4m1.ld \
          -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map

all: $(BUILD)/$(TARGET).bin

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJ) linker/ra4m1.ld
	$(CC) $(OBJ) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD):
	mkdir -p $(BUILD)

flash: $(BUILD)/$(TARGET).bin
	$(PYOCD) flash -t $(CHIP) --base-address $(FLASH_ADDR) $<

reset:
	$(PYOCD) reset -t $(CHIP)

size: $(BUILD)/$(TARGET).elf
	$(SIZE) $<

clean:
	rm -rf $(BUILD)

-include $(DEP)

.PHONY: all flash reset size clean