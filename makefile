MCU     = atmega32
F_CPU   = 1000000UL
CC      = avr-gcc
OBJCOPY = avr-objcopy

CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Iinclude -Os -Wall -std=c99

SRC_DIR  = src
INC_DIR  = include
OUT_DIR  = .firmware
TMP_DIR  = .temp

SRCS    = $(wildcard $(SRC_DIR)/*.c)
OBJS    = $(patsubst $(SRC_DIR)/%.c, $(TMP_DIR)/%.o, $(SRCS))

TARGET_ELF = $(OUT_DIR)/firmware.elf
TARGET_HEX = $(OUT_DIR)/firmware.hex

.PHONY: all clean

all: $(TARGET_HEX)
#  @$(MAKE) clean

$(TMP_DIR)/%.o: $(SRC_DIR)/%.c $(wildcard $(INC_DIR)/*.h) | $(TMP_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_ELF): $(OBJS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(OUT_DIR) $(TMP_DIR):
	mkdir -p $@

clean:
	rm -rf $(TMP_DIR)