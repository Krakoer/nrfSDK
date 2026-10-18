CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
NRFUTIL = nrfutil

CFLAGS  = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
          -O2 -ffreestanding -fno-builtin -nostdlib -Wall -Wextra \
		  -Werror=implicit-function-declaration -Werror=implicit-int \
          -I$(SDK)/include
LDFLAGS = -T $(SDK)/boot/nrf52840.ld -nostdlib -Wl,--gc-sections

SRCS += $(SDK)/boot/startup.c $(wildcard $(SDK)/src/*.c)

NRFFLAGS    = pkg generate --hw-version 52 --sd-req 0x00 \
              --application-version 1 --application $(TARGET).hex
UPLOADFLAGS = dfu usb-serial -pkg $(TARGET)_dfu.zip -p /dev/ttyACM0 -b 115200

all: $(TARGET)_dfu.zip

$(TARGET).elf: $(SRCS) $(SDK)/boot/nrf52840.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(TARGET).elf

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET)_dfu.zip: $(TARGET).hex
	$(NRFUTIL) $(NRFFLAGS) $(TARGET)_dfu.zip

send:
	$(NRFUTIL) $(UPLOADFLAGS)

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET)_dfu.zip

.PHONY: all upload clean