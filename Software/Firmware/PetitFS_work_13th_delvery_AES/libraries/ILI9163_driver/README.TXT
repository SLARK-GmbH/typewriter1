------------------ ILI9163 / ST7735 driver for Atmel MCUs ---------------------
- author: Henryk Richter <henryk.richter@gmx.net>                             -
-------------------------------------------------------------------------------

This is a quite fast driver for cheap 1.44" (128x128) or 1.8" (128x160)
LCDs. Most of these LCDs come either with an ILI9163 or ST7735 controller
chip. 

Interestingly for me, the seller insisted on using an ILI9163 driver for
the display (which I did at first) but on the other hand, the packaging of
my particular display said "ST7735". Hmpf. So in essence, one can safely
assume that the ST7735 chips are quite versatile in their command handling.

This driver is named ILI9163 but should support both types.

The main reason for me to implement an own driver was the desire to support
all three color depths of the display controller. Especially the 12 Bit mode
is of interest for fast applications (like demos) with real-time graphics.
The 18 Bit mode (which was to my knowledge not implemented before in open
source code) on the other hand comes in handy for natural images.


Features (usual = - , special features = !!):
-  Initialization / SPI Write
   - either HW SPI at up to f/2 (f=MCU clock)  _or_
   - fast Software SPI at f/4, i.e. 4 MHz SPI with 16 MHz MCU
-  modular, just disable #defines for functionality you don't need
!! 12/16/18 Bit bit depths supported
-  Display orientation change in 90 degree units
-  Pixel setting
-  Area Fill
!! High/True Color image display (see included image converter for 12/16/18 Bit)
!! High/True Color scaled image display (bresenham based nearest neighbor)
!! 8 Bit image display (with individual color table)
   - low Flash requirements with just 8 Bit per pixel instead of 12/16/18
   - automatic conversion to active display mode (12/16/18 Bit)
!! PutLine function, which shows a single line (or column when changing
   the display orientation) from RAM on screen, useful for demo effects
   (input line in RAM as indexed color, output in current display mode)
!! Text functions with multiple font support
   - foreground/background color selectable
   - more than one font at once

Other primitives like arbitrary lines (horizontal/vertical lines can be drawn
by LCD_FillRect() ) and circles are not included here but can be easily copied
from other implementations if necessary.

Quick start:
- see ILI9163.h for PIN assigment Atmel->LCD, modify the LCD_PORT_*,LCD_DDR_*,
  LCD_PIN_* defines if necessary, the default is suitable for hardware SPI
  on Atmega328 (or software SPI on any Atmega with a usable Port B)
- initialization: call LCD_init(), afterwards the display is ready for drawing
- a quick first test for proper settings and connections would be
  { color_t a; while(1){ LCD_FillScreen( a );a ^= 0xffff; } }
- colors can be specified with the LCD_RGB() macro which converts the
  color to the active bit depth at compile-time for static tables
  or at runtime for dynamically specified colors
- see demoloop.c for an overview of the functionality
