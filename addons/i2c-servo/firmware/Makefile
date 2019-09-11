CC	:= avr-gcc
LD	:= avr-ld
OBJCOPY	:= avr-objcopy
OBJDUMP	:= avr-objdump
SIZE	:= avr-size

TARGET = i2c-servo
SOURCE = $(wildcard *.c)

VERSION=0x10   # V1.0

# select MCU
MCU = attiny25

AVRDUDE_PROG := -c usbasp

# ---------------------------------------------------------------------------

# 8 Mhz internal OSC, no brown out

# F_CPU=1000000
F_CPU=8000000
AVRDUDE_MCU=t25

# http://www.engbedded.com/fusecalc/
# AVRDUDE_FUSES=lfuse:w:0x62:m hfuse:w:0xdf:m efuse:w:0xff:m
AVRDUDE_FUSES=lfuse:w:0xe2:m hfuse:w:0xdf:m efuse:w:0xff:m

# ---------------------------------------------------------------------------

CFLAGS = -pipe -O2 -mmcu=$(MCU) -Wall -fdata-sections -ffunction-sections
CFLAGS += -Wa,-adhlns=$(*F).lst -DF_CPU=$(F_CPU) -DVERSION=$(VERSION)
LDFLAGS = # -Wl,-Map,$(@:.elf=.map),--cref,--relax,--gc-sections

# ---------------------------------------------------------------------------

$(TARGET): $(TARGET).elf
	@$(SIZE) -B -x --mcu=$(MCU) $<

$(TARGET).elf: $(SOURCE:.c=.o)
	@echo " Linking file:  $@"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(OBJDUMP) -h -S $@ > $(@:.elf=.lss)
	$(OBJCOPY) -j .text -j .data -O ihex $@ $(@:.elf=.hex)
	$(OBJCOPY) -j .text -j .data -O binary $@ $(@:.elf=.bin)

%.o: %.c $(MAKEFILE_LIST)
	@echo " Building file: $<"
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(SOURCE:.c=.o) $(SOURCE:.c=.lst) $(addprefix $(TARGET), .elf .map .lss .hex .bin)

install: $(TARGET).elf
	avrdude $(AVRDUDE_PROG) -p $(AVRDUDE_MCU) -e $(patsubst %,-U %, $(AVRDUDE_FUSES))
	avrdude $(AVRDUDE_PROG) -p $(AVRDUDE_MCU) -U flash:w:$(<:.elf=.hex)
