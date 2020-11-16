# Name: Makefile
# Author: <insert your name here>
# Copyright: <insert your copyright message here>
# License: <insert your license reference here>

# This is a prototype Makefile. Modify it according to your needs.
# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected. We recommend that you leave it undefined and
#                add settings like this to your ~/.avrduderc file:
#                   default_programmer = "stk500v2"
#                   default_serial = "avrdoper"
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

#DEVICE = attiny85
#CLOCK  = 8000000
DEVICE     = atmega328p
CLOCK      = 16000000
PROGRAMMER = -c avrispmkii 
#-P avrdoper
OBJECTS    = ILI9163.o main.o demoloop.o 
HEADERS	   = ILI9163.h
FUSES      = -U hfuse:w:0xda:m -U lfuse:w:0xff:m
# DA/FF were what my Atmega 328p's had as default...

# ATMega328p fuse bits used above (fuse bits for other devices are different!):
# Example for 16 MHz external oscillator
# Fuse high byte:
# 0xda = 1 1 0 1   1 0 1 0 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   +---------- EESAVE (set to 0 to preserve EEPROM over chip erase)
#        | | | +-------------- WDTON    (watchdog, 1=unprogrammed or not on)
#        | | +---------------- SPIEN    (if set to 1, serial programming is disabled)
#        | +------------------ DWEN     (debug wire)
#        +-------------------- RSTDISBL (if set to 0, RESET pin is disabled)
# Fuse low byte:
# 0x87 = 1 0 0 0   0 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (8M internal RC, 0111 external quartz)
#        | |  +--------------- SUT 1..0 (slowly rising power)
#        | +------------------ CKOUT(def 1 unprogrammed)
#        +-------------------- CKDIV8 (def 0 programmed = clock divide by 8, want 1 = no clock div 8)
#
# For computing fuse byte values for other devices and options see
# the fuse bit calculator at http://www.engbedded.com/fusecalc/


# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)
COMPILEPP = avr-g++ -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)


# symbolic targets:
all:	AtmegaBoard.hex

.c.o:
	$(COMPILE) -c $< -o $@

.cpp.o:
	$(COMPILEPP) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:AtmegaBoard.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID AtmegaBoard.hex

clean:
	rm -f AtmegaBoard.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

AtmegaBoard.hex: main.elf
	rm -f AtmegaBoard.hex
	avr-objcopy -j .text -j .data -O ihex main.elf AtmegaBoard.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
