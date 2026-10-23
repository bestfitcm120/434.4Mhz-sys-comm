MCU     = atmega32
# 8 MHz crystal. Override on the command line if needed: make F_CPU=1000000UL
F_CPU  ?= 8000000UL
CC      = avr-gcc
OBJCOPY = avr-objcopy

CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=gnu11 \
          -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS = -mmcu=$(MCU) -Wl,--gc-sections

SRC_DIR  = src
LIB_DIR  = lib
OUT_DIR  = .firmware
TMP_DIR  = .temp

# Firmware names. Each one has src/<NAME>/*.c, lib/<NAME>/*.h
# and builds to $(OUT_DIR)/Tx_firmware.elf/.hex or Rx_firmware.elf/.hex
FIRMWARES = TX RX
NAME_TX   = Tx
NAME_RX   = Rx

.PHONY: all clean $(FIRMWARES) tx rx

# Default: build both firmwares
all: $(FIRMWARES)

# Build one firmware only: make tx / make rx
tx: TX
rx: RX

# $(1) = TX or RX
define FIRMWARE_RULES

$(1)_SRCS = $$(wildcard $(SRC_DIR)/$(1)/*.c)
$(1)_OBJS = $$(patsubst $(SRC_DIR)/$(1)/%.c, $(TMP_DIR)/$(1)/%.o, $$($(1)_SRCS))
$(1)_ELF  = $(OUT_DIR)/$(NAME_$(1))_firmware.elf
$(1)_HEX  = $(OUT_DIR)/$(NAME_$(1))_firmware.hex

ifeq ($$(strip $$($(1)_SRCS)),)
# No sources yet: skip politely so "make" still builds the other firmware
$(1):
	@echo "$(1): no .c files in $(SRC_DIR)/$(1), skipped"
else
$(1): $$($(1)_HEX)
endif

$(TMP_DIR)/$(1)/%.o: $(SRC_DIR)/$(1)/%.c | $(TMP_DIR)/$(1)
	$$(CC) $$(CFLAGS) -I$(LIB_DIR)/$(1) -c $$< -o $$@

$$($(1)_ELF): $$($(1)_OBJS) | $(OUT_DIR)
	$$(CC) $$(LDFLAGS) -o $$@ $$^

$$($(1)_HEX): $$($(1)_ELF)
	$$(OBJCOPY) -O ihex -R .eeprom $$< $$@

$(TMP_DIR)/$(1):
	mkdir -p $$@

-include $$($(1)_OBJS:.o=.d)
endef

$(foreach fw,$(FIRMWARES),$(eval $(call FIRMWARE_RULES,$(fw))))

$(OUT_DIR) $(TMP_DIR):
	mkdir -p $@

clean:
	rm -rf $(TMP_DIR)