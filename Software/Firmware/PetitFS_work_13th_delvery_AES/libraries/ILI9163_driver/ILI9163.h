/* ********************************************************************************
  file:
   ILI9163.h
   
  purpose:
   ATMega LCD display driver for 128x128 12/16/18 bit color display, driver ILI9163

  author, copyright:
   Henryk Richter <bax@comlab.uni-rostock.de>

  version:
   0.1

  date:
   10-Apr-2016

  devices:
   cheap 1.44" LCDs

  acknowledgements:

  license:
   GPL v2

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  notes:
   - consult header-file for PIN assignments
   - you are free to choose any 3-5 digital output pins in software SPI mode
   - modify LCD_PORT_*, LCD_DDR_*, LCD_PIN_* to your liking
   - in hardware SPI mode, observe clock settings and connect CLK/DATA to SCK/MOSI
     pins of your device
*********************************************************************************** */
#ifndef	ILI9163_H
#define	ILI9163_H

#ifdef __cplusplus
#define PROTOHEADER "C"
#else
#define PROTOHEADER
#endif

/* ******************************************************************************** */
/* functional options of library: 
    Hardware / Software SPI
*/
#define _LCD_SOFT_SPI
                        /* software SPI implementation (slower but more flexible), 
                           else hardware SPI (faster, requires to set the correct 
                           SCK/MOSI pins -> requires MPU with hardware SPI, disable
			   for MCUs like ATTINY85                                   */
#define _LCD_STREAM_SUPPORT /* streaming data writes without setup phase per Byte,  */
                            /* faster but more code (about 20 Bytes)                */
/* LCD bit depth: choose one of the three, comment out the others */
#define _LCD_12BIT    /* 12 BPP - runtime efficient serial data transfer,         */
//#define _LCD_16BIT      /* default: 16 BPP (pixels directly addressable           */
//#define _LCD_18BIT      /* 18 BPP, very slow due to 24 BPP serial transfer format */

#define _LCD_ROTATION_SUPPORT /* optional: support LCD_Orientation(),               */
                              /*           rotation by n*90 degrees                 */

#define _LCD_PUTLINE_SUPPORT /* write a line of 8 Bit paletted pixels and convert   */
                             /* to actual display depth on the fly (based on        */
			     /* supplied color map (requires hardware SPI)          */
#define _LCD_SHOWIMAGE_SUPPORT /* optional: unscaled high / true color image support*/
//#define _LCD_SHOWIMAGESC_SUPPORT /* optional: scaled image support                  */
#define _LCD_SHOWIMAGE8_SUPPORT	/*  optional: bit chunky pixel image support        */ 
#define _LCD_FONT_SUPPORT    /* compile support for Putc(), Puts() ...              */
/* ******************************************************************************** */


/* ******************************************************************************** */
/* hardware  options of library: 

    LCD pins (1,2,3,4,5,6,7,8 -> VCC,GND,CS,RST,C/D,SDA,SCK,LED) mapped to ATMega 
    /CS   - B1 ( chip select low )
    /RES  - B4 ( reset low, MISO )
    C/D   - B2 ( command/data )
    SCK   - B5 ( SCK/SDA must be connected to the same port (!), clock signal, SCK ) 
    SDA   - B3 ( data, MOSI )

    other pins of module
    VDD   - 3.3V
    GND   - GND
    LED   - 10R -> VDD

    -> if hardware SPI is desired, map the CLK/DATA pins to SCK/MOSI
    -> if ATMega runs @5V, use 6.7k / 10k voltage divider for the 5 control pins
    -> if you use the Atmel MISO pin for /RES (like I did), apply a pullup resistor 
       to the pin (e.g. 6.7k->VCC)
    -> I didn't use the "minimal" wiring for a couple of reasons
       - in hardware SPI mode, MISO is unavailable as GPIO anyway -> /RES
       - i quite like proper /CS instead of hanging it low all the time
*/
#if 0 
/* Pinout for ATTINY85 - 3-wire SPI example
   (you might want to disable the defines for PIN_CS and PIN_RES, especially on ATtiny)
   - by disabling PIN_CS and/or PIN_RES it is assumed that PIN_RES has it's own power-on-reset and
     PIN_CS is held down to ground (the other defines regarding those pins are ignored)
*/
#define LCD_PORT_CS   PORTB
#define LCD_DDR_CS    DDRB
//#define LCD_PIN_CS    4 /* no chip select for 3-wire SPI */
#define LCD_PORT_RES  PORTB
#define LCD_DDR_RES   DDRB
//#define LCD_PIN_RES   1 /* no reset for 3-wire SPI */
#define LCD_PORT_CD   PORTB
#define LCD_DDR_CD    DDRB
#define LCD_PIN_CD    3 
#define LCD_PORT_CLK  PORTB
#define LCD_DDR_CLK   DDRB
#define LCD_PIN_CLK   2 
#define LCD_PORT_DATA PORTB
#define LCD_DDR_DATA  DDRB
#define LCD_PIN_DATA  0 
#else
/* suggested Atmega 328p pinout (default, matches my website, 5-wire SPI example) */
#define LCD_PORT_CS   PORTB
#define LCD_DDR_CS    DDRB
#define LCD_PIN_CS    1
#define LCD_PORT_RES  PORTB
#define LCD_DDR_RES   DDRB
#define LCD_PIN_RES   4
#define LCD_PORT_CD   PORTB
#define LCD_DDR_CD    DDRB
#define LCD_PIN_CD    2 
#define LCD_PORT_CLK  PORTB
#define LCD_DDR_CLK   DDRB
#define LCD_PIN_CLK   5
#define LCD_PORT_DATA PORTB
#define LCD_DDR_DATA  DDRB
#define LCD_PIN_DATA  3 
#endif
/* this #define, if not commented out enables the fast software SPI write routine 
   and means that CLK and DATA are on PORT B (change to PINA,PINC,PIND if appropriate) */
#define LCD_PIN_CLKDATA PINB

/* fast software SPI (when enabled with LCD_PIN_CLKDATA defined) runs at clock/4. If your
   display does not initialize, try disabling the LCD_PIN_CLKDATA define. That way, the
   SPI runs approximately at f/6 (2.6 MHz @ 16 MHz MCU). If that helps, check your 
   cabling afterwards (not too long, proper voltage dividers if any, etc.) You might also
   consider hardware SPI at reduced clock. */

/* ******************************************************************************** */


/* ******************************************************************************** */
/*
   initialization parameters
    - start with conservative values, tune them for your actual display after you
      get working results
*/
/* ******************************************************************************** */
#define LCD_RST_DELAY  100  /* conservative: 500                                    */
#define LCD_SLEEP_DELAY  1  /* 1 worked for me (and should not yield problems)      */
#define LCD_BOOST_DELAY 40  /* 30 worked for me, following spec: 40                 */



/* ******************************************************************************** */
/*
   useful parameters 
   (other displays than mine might have also 128x160 or other resolutions)
*/
/* ******************************************************************************** */
#define LCD_Width  128
#define LCD_Height 128 /* change to 160 for 1.8" displays */

#define LCD_ROT_0   0
#define LCD_ROT_90  1
#define LCD_ROT_180 2
#define LCD_ROT_270 3
/* ******************************************************************************** */

#if (defined _LCD_ROTATION_SUPPORT) && (defined ILI9163_c)
unsigned char LCD_orientation = LCD_ROT_0; /* LCD_ROT_90, LCD_ROT_180, LCD_ROT_270  */
#endif

/* ******************************************************************************** */
/*                                                                                  */
/*                       ILI9163 LCD Controller Commands                            */
/* (credit: #defines below copied from driver by Simon Inns)                        */
/* ******************************************************************************** */
#define ILI9163_CMD_NOP                     0x00
#define ILI9163_CMD_SOFT_RESET              0x01
#define ILI9163_CMD_GET_RED_CHANNEL         0x06
#define ILI9163_CMD_GET_GREEN_CHANNEL       0x07
#define ILI9163_CMD_GET_BLUE_CHANNEL        0x08
#define ILI9163_CMD_GET_PIXEL_FORMAT        0x0C
#define ILI9163_CMD_GET_POWER_MODE          0x0A
#define ILI9163_CMD_GET_ADDRESS_MODE        0x0B
#define ILI9163_CMD_GET_DISPLAY_MODE        0x0D
#define ILI9163_CMD_GET_SIGNAL_MODE         0x0E
#define ILI9163_CMD_GET_DIAGNOSTIC_RESULT   0x0F
#define ILI9163_CMD_ENTER_SLEEP_MODE        0x10
#define ILI9163_CMD_EXIT_SLEEP_MODE         0x11
#define ILI9163_CMD_ENTER_PARTIAL_MODE      0x12
#define ILI9163_CMD_ENTER_NORMAL_MODE       0x13
#define ILI9163_CMD_EXIT_INVERT_MODE        0x20
#define ILI9163_CMD_ENTER_INVERT_MODE       0x21
#define ILI9163_CMD_SET_GAMMA_CURVE         0x26
#define ILI9163_CMD_SET_DISPLAY_OFF         0x28
#define ILI9163_CMD_SET_DISPLAY_ON          0x29
#define ILI9163_CMD_SET_COLUMN_ADDRESS      0x2A
#define ILI9163_CMD_SET_PAGE_ADDRESS        0x2B
#define ILI9163_CMD_WRITE_MEMORY_START      0x2C
#define ILI9163_CMD_WRITE_LUT               0x2D
#define ILI9163_CMD_READ_MEMORY_START       0x2E
#define ILI9163_CMD_SET_PARTIAL_AREA        0x30
#define ILI9163_CMD_SET_SCROLL_AREA         0x33
#define ILI9163_CMD_SET_TEAR_OFF            0x34
#define ILI9163_CMD_SET_TEAR_ON             0x35
#define ILI9163_CMD_SET_ADDRESS_MODE        0x36
#define ILI9163_CMD_SET_SCROLL_START        0X37
#define ILI9163_CMD_EXIT_IDLE_MODE          0x38
#define ILI9163_CMD_ENTER_IDLE_MODE         0x39
#define ILI9163_CMD_SET_PIXEL_FORMAT        0x3A
#define ILI9163_CMD_WRITE_MEMORY_CONTINUE   0x3C
#define ILI9163_CMD_READ_MEMORY_CONTINUE    0x3E
#define ILI9163_CMD_SET_TEAR_SCANLINE       0x44
#define ILI9163_CMD_GET_SCANLINE            0x45
#define ILI9163_CMD_READ_ID1                0xDA
#define ILI9163_CMD_READ_ID2                0xDB
#define ILI9163_CMD_READ_ID3                0xDC
#define ILI9163_CMD_FRAME_RATE_CONTROL1     0xB1
#define ILI9163_CMD_FRAME_RATE_CONTROL2     0xB2
#define ILI9163_CMD_FRAME_RATE_CONTROL3     0xB3
#define ILI9163_CMD_DISPLAY_INVERSION       0xB4
#define ST7735_DISSETS                      0xB6
#define ILI9163_CMD_SOURCE_DRIVER_DIRECTION 0xB7
#define ILI9163_CMD_GATE_DRIVER_DIRECTION   0xB8
#define ILI9163_CMD_POWER_CONTROL1          0xC0
#define ILI9163_CMD_POWER_CONTROL2          0xC1
#define ILI9163_CMD_POWER_CONTROL3          0xC2
#define ILI9163_CMD_POWER_CONTROL4          0xC3
#define ILI9163_CMD_POWER_CONTROL5          0xC4
#define ILI9163_CMD_VCOM_CONTROL1           0xC5
#define ILI9163_CMD_VCOM_CONTROL2           0xC6
#define ILI9163_CMD_VCOM_OFFSET_CONTROL     0xC7
#define ILI9163_CMD_WRITE_ID4_VALUE         0xD3
#define ILI9163_CMD_NV_MEMORY_FUNCTION1     0xD7
#define ILI9163_CMD_NV_MEMORY_FUNCTION2     0xDE
#define ILI9163_CMD_POSITIVE_GAMMA_CORRECT  0xE0
#define ILI9163_CMD_NEGATIVE_GAMMA_CORRECT  0xE1
#define ILI9163_CMD_GAM_R_SEL               0xF2

#define ILI9163_RGB 8 /* RGB=0, BGR=8 */


/* 16 BPP depth for color specifications (even in 18 bit mode, to avoid 32 bit variables, appropriate scaling in the driver) */
typedef unsigned int  color_t;
#ifdef _LCD_12BIT
#define LCD_RGB( _r_,_g_,_b_) (((((unsigned int)(_r_))&0xf0)<<4)|(((unsigned int)(_g_))&0xf0)|(((unsigned int)(_b_))>>4))
#else
/* color specification is 8 bit for red, green and blue, scaled down to 565 */
//#define LCD_RGB( _r_,_g_,_b_) (((((unsigned int)(_r_))&0x1f)<<11)|((((unsigned int)(_g_))&63)<<5)|(((unsigned int)(_b_))&0x1f))
#define LCD_RGB( _r_,_g_,_b_) (((((unsigned int)(_r_))&0xF8)<<8)|((((unsigned int)(_g_))&0xFC)<<3)|(((unsigned int)(_b_))>>3))
#endif

/* some default colors */
#define LCD_WHITE   LCD_RGB(255,255,255)
#define LCD_BLACK   LCD_RGB(  0,  0,  0)
#define LCD_GRAY    LCD_RGB(127,127,127)
#define LCD_GREEN   LCD_RGB(  0,255,  0)
#define LCD_BLUE    LCD_RGB(  0,  0,255)
#define LCD_RED     LCD_RGB(255,  0,  0)
#define LCD_MAGENTA LCD_RGB(255,  0,255)
#define LCD_YELLOW  LCD_RGB(255,255,  0)
#define LCD_CYAN    LCD_RGB(  0,255,255)


/* ******************************************************************************** */
/*
   Functions
*/
/* ******************************************************************************** */

/* set/clear Bit in register or port */
#define BitSet( _port_, _bit_ ) _port_ |=   (1 << (_bit_) )
#define BitClr( _port_, _bit_ ) _port_ &= (~(1 << (_bit_) ) )
/* copy bit from source to destination (2 clocks via T Bit) */
#define BitCpy( _out_, _obit_, _in_, _ibit_ ) \
	asm volatile ( "bst %2,%3" "\n\t" \
	               "bld %0,%1" : "+r" (_out_) : "I" (_obit_) , "r" (_in_) , "I" (_ibit_) );


#ifdef _LCD_12BIT
#define PIXELS2BYTES( pix ) ((pix)+((pix)>>1))
#endif
#ifdef _LCD_16BIT
#define PIXELS2BYTES( pix ) ((pix)<<1)
#endif
#ifdef _LCD_18BIT
#define PIXELS2BYTES( pix ) ((pix)*3)
#endif


/*
  initialization: activate display
*/
extern PROTOHEADER void LCD_init( void );

#ifdef _LCD_ROTATION_SUPPORT
/* 
  optional:  set orientation (rotation) of display for text and graphics output
  arguments: rot = LCD_ROT_x, where x is 0,90,180,270 (e.g. LCD_ROT_90)
  note:     when rotated, keep in mind to swap LCD_Width/LCD_Height when limiting
            coordinates
*/
extern PROTOHEADER void LCD_Orientation( unsigned char rot );
#endif

/*
  write 8 Bit via SPI, MSB first, transfer bit on rising clock edge
  probably not needed outside the LCD driver
*/
extern PROTOHEADER void LCD_SPI( unsigned char dta );

/*
  write command or data to LCD
  probably not needed outside the LCD driver
*/
extern PROTOHEADER void LCD_CMD( unsigned char cmd );
extern PROTOHEADER void LCD_DTA( unsigned char dta );
extern PROTOHEADER void LCD_DTA16( unsigned int dta );


/*
  rectangle fill
*/
extern PROTOHEADER void LCD_FillScreen( color_t color ); /* clear screen with color 0=black,1=white */
extern PROTOHEADER void LCD_FillRect( unsigned char x, unsigned char y, unsigned char w, unsigned char h, color_t color);

/*
 set drawing area, relative to upper left corner of rotated coordinates (intuitive)
 - e.g. ...( 1,1,8,16 ); will establish a drawing area of 8x16 at up/left coordinates 1,1;
   where the up/left coordinate will be the rotated 0,90,180,270 degrees position
   of the display's chosen orientation
 - keep in mind that for rotations of 90 or 270 degrees, displays with 128x160
   will be 160x128 in size, so adapt your screen layout accordingly in these cases
 - see LCD_FillRect() for proper application of start coordinates+width/height
*/
extern PROTOHEADER void LCD_SetActiveArea( unsigned char xstart, unsigned char ystart, unsigned char xstop, unsigned char ystop );

/*
  draw pixel at x,y with specified color, use 0=black,1=white as color spec
*/
extern PROTOHEADER void LCD_SetPixel( unsigned char x, unsigned char y, color_t color );

#ifdef _LCD_SHOWIMAGE_SUPPORT
/* 
  Show Image of width/height at specified coordinates, image pointer assumed to be in progmem
*/
extern PROTOHEADER void LCD_ShowImage (unsigned char x, unsigned char y, unsigned char w, unsigned char h, const unsigned char *img); 
#endif

#ifdef _LCD_SHOWIMAGESC_SUPPORT
extern PROTOHEADER void LCD_ShowImageSC(unsigned char  x, unsigned char y, 
                                        unsigned char dw, unsigned char dh,
					unsigned char w,  unsigned char h,
					const unsigned char *img); 
#endif

#if (defined _LCD_PUTLINE_SUPPORT) && !(defined _LCD_SOFT_SPI)
/*
  Show single line of an image (from RAM)
  note: use LCD_ShowImage() when an image row from flash is to be shown 
*/
extern PROTOHEADER void LCD_PutLine(unsigned char x, unsigned char y, unsigned char w, char *img, const color_t *colormap );
#endif

#ifdef _LCD_SHOWIMAGE8_SUPPORT 
/* show 8 Bit chunky pixel image in current mode, apply supplied color map (requires putline */
void LCD_ShowImage8(unsigned char x, unsigned char y, unsigned char w, unsigned char h, const unsigned char *img, const color_t *colormap );
#endif

#ifdef _LCD_FONT_SUPPORT
/*
  Show single character at location x,y
  - negative x and/or y will force double width/height at pos abs(x),abs(y)
*/
extern PROTOHEADER unsigned char LCD_Putc(char c, unsigned char x, unsigned char y, color_t f_color, color_t b_color );
/* 
 Show 0-terminated string, starting at location x,y (newline \r or \n)
  - negative x and/or y will force double width/height at pos abs(x),abs(y)
*/
extern PROTOHEADER unsigned char LCD_Puts(char *str, unsigned char x, unsigned char y, color_t f_color, color_t b_color );
extern PROTOHEADER unsigned char LCD_Putsf(const char *str, unsigned char x, unsigned char y,color_t f_color, color_t b_color );

#include "bax_font.h"
/* same as above, just with a given font instead of the default one */
extern PROTOHEADER unsigned char LCD_PutcF(char c, unsigned char x, unsigned char y, color_t f_color, color_t b_color,   const Bax_font*f );
extern PROTOHEADER unsigned char LCD_PutsF(char *str, unsigned char x, unsigned char y, color_t f_color, color_t b_color,const Bax_font*f );
extern PROTOHEADER unsigned char LCD_PutsfF(const char *str, unsigned char x, unsigned char y,color_t f_color, color_t b_color,const Bax_font*f );
#endif

#endif	/* ILI9163_H */
