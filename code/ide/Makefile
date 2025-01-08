CC=m68k-elf-rosco-gcc
OBJCOPY=m68k-elf-rosco-objcopy
OBJDUMP=m68k-elf-rosco-objdump
CFLAGS?=-g -O3
LDFLAGS?=-g -mgdb

SOURCES=main.c ide.c
OBJECTS=$(SOURCES:.c=.o)
PROGRAM=idetest

all: $(PROGRAM).bin $(PROGRAM).dis

$(PROGRAM).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -Wl,-Map=$(PROGRAM).map -Wl,--warn-common -o $@ $^

$(PROGRAM).dis: $(PROGRAM).elf
	$(OBJDUMP) -d $< > $@

$(PROGRAM).bin: $(PROGRAM).elf
	$(OBJCOPY) -O binary $< $@

clean:
	$(RM) $(OBJECTS) $(PROGRAM).bin $(PROGRAM).map $(PROGRAM).elf $(PROGRAM).dis

.PHONY: all clean