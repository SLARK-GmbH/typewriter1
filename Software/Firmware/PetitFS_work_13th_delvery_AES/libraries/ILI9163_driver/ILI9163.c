/* ********************************************************************************
  file:
   ILI9163.c
   
  purpose:
   ATMega LCD display driver for 128x128 12/16/18 bit color display, driver ILI9163


  author, copyright:
   Henryk Richter <bax@comlab.uni-rostock.de>

  version:
   0.1

  date:
   10-Apr-2016

  devices:
   cheap 1.44"/1.8" LCDs 

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

  stats:

*********************************************************************************** */
#include <avr/pgmspace.h>
#include <util/delay.h> /* might be <avr/delay.h>, depending on toolchain */

#define  ILI9163_c /* make sure, globals in .h are not compiled twice... */
#include "ILI9163.h"

#ifdef _LCD_FONT_SUPPORT
#define FONT_MAIN
//#include "bax_font.h"
/* you should include at least 1 font directly here to be used as default font */
#include "font_screen_ascii_5x8v2_hor.h"
//#include "font_monocle_ascii_6x8.h"
//#include "font_Bitumen_ascii+dft_9x16.h"
//#include "font_Topaz8x16_hor.h"
#undef FONT_MAIN
#endif

/* ******************************************************************************** */
/* internal: end of initialization, implicit delay command */
#define LCD_CMD_END   0	   /* actually maps on NOP ... */
#define LCD_CMD_DELAY 0x80 /* delay command */
/* ******************************************************************************** */

/* Initialization list:
   - command byte (0=end of list)
   - number as byte:
          N of data bytes after the command (0 = cmd only)
       + DEL - a delay byte (in ms) follows after data (optional)
   - N data bytes
   - if( N & DEL ) delay byte
*/
const unsigned char PROGMEM LCD_init_seq[] = {
#if 0 
	ILI9163_CMD_EXIT_SLEEP_MODE,LCD_CMD_DELAY,150, /* wake up screen */
	ILI9163_CMD_SET_PIXEL_FORMAT,1,0x05,           /* 16 BPP */
	ILI9163_CMD_SET_GAMMA_CURVE, 1,0x04,           /* gamma curve 3  */
	ILI9163_CMD_GAM_R_SEL,1,0x01,                  /* gamma adjust enable */
	ILI9163_CMD_POSITIVE_GAMMA_CORRECT,15,         /* positive gamma correction parameters */
	 0x3f,0x25,0x1c,0x1e,0x20,0x12,0x2a,
	 0x90,0x24,0x11,0x00,0x00,0x00,0x00,0x00,
	ILI9163_CMD_NEGATIVE_GAMMA_CORRECT,15,       /* negative gamma correction parameters */
	 0x20,0x20,0x20,0x20,0x05,0x00,0x15,
	 0xa7,0x3d,0x18,0x25,0x2a,0x2b,0x2b,0x3a,
	ILI9163_CMD_FRAME_RATE_CONTROL1,3,1,0x2c,0x2d, //17,20, //0x8,0x8,
	ILI9163_CMD_FRAME_RATE_CONTROL2,3,1,0x2c,0x2d, //17,20, //0x8,0x8,
	ILI9163_CMD_FRAME_RATE_CONTROL3,6,1,0x2c,0x2d, //17,20, //0x8,0x8,
	                                  1,0x2c,0x2d,
	ILI9163_CMD_DISPLAY_INVERSION,1,0x7,    /* NLA = 1, NLB = 1, NLC = 1 (all on Frame Inversion) */
	ILI9163_CMD_POWER_CONTROL1,3,0xa2,0x2,0x84,   /* VRH = 10:  GVDD = 4.30, VC = 2: VCI1 = 2.65 */
	ILI9163_CMD_POWER_CONTROL2,1,0xC5,       /* BT = 2: AVDD = 2xVCI1, VCL = -1xVCI1, VGH = 5xVCI1, VGL = -2xVCI1 */
	ILI9163_CMD_POWER_CONTROL3,2,0xA,0x00,
	ILI9163_CMD_VCOM_CONTROL1,2,0x50,0x5b,  /* VMH = 80: VCOMH voltage = 4.5, VML = 91: VCOML voltage = -0.225 */
	ILI9163_CMD_VCOM_OFFSET_CONTROL,1,0x40, /* nVM = 0, VMF = 64: VCOMH output = VMH, VCOML output = VML */
	ILI9163_CMD_SET_COLUMN_ADDRESS,4,0,0,0,0x7f, /* XSH,XSL,XEH,XEL */
	ILI9163_CMD_SET_PAGE_ADDRESS,4,0,0,0,0x7f,   /* YSH,YSL,YEH,YEL */
	ILI9163_CMD_SET_ADDRESS_MODE,1,8+96,	/* rotation=0 (96,160,192), (BGR=8) */
	ST7735_DISSETS,2,0x15,2,
	ILI9163_CMD_SET_DISPLAY_ON,LCD_CMD_DELAY,10,
	ILI9163_CMD_ENTER_NORMAL_MODE,LCD_CMD_DELAY,10,
	ILI9163_CMD_WRITE_MEMORY_START,0,
#else
	ILI9163_CMD_EXIT_SLEEP_MODE,LCD_CMD_DELAY,5, /* wake up screen */
#ifdef _LCD_12BIT
	ILI9163_CMD_SET_PIXEL_FORMAT,1,0x03,         /* 12 BPP */
#else
#ifdef _LCD_16BIT
	ILI9163_CMD_SET_PIXEL_FORMAT,1,0x05,         /* 16 BPP */
#else
	ILI9163_CMD_SET_PIXEL_FORMAT,1,0x06,         /* 18 BPP */
#endif
#endif
	ILI9163_CMD_SET_GAMMA_CURVE, 1,0x04,         /* gamma curve 3  */
	ILI9163_CMD_GAM_R_SEL,1,0x01,                /* gamma adjust enable */
	ILI9163_CMD_POSITIVE_GAMMA_CORRECT,15,       /* positive gamma correction parameters */
	 0x3f,0x25,0x1c,0x1e,0x20,0x12,0x2a,
	 0x90,0x24,0x11,0x00,0x00,0x00,0x00,0x00,
	ILI9163_CMD_NEGATIVE_GAMMA_CORRECT,15,       /* negative gamma correction parameters */
	 0x20,0x20,0x20,0x20,0x05,0x00,0x15,
	 0xa7,0x3d,0x18,0x25,0x2a,0x2b,0x2b,0x3a,
	ILI9163_CMD_FRAME_RATE_CONTROL1,2,17,20, //0x8,0x8,
	ILI9163_CMD_DISPLAY_INVERSION,1,0x7,    /* NLA = 1, NLB = 1, NLC = 1 (all on Frame Inversion) */
	ILI9163_CMD_POWER_CONTROL1,2,0xa,0x2,   /* VRH = 10:  GVDD = 4.30, VC = 2: VCI1 = 2.65 */
	ILI9163_CMD_POWER_CONTROL2,1,0x2,       /* BT = 2: AVDD = 2xVCI1, VCL = -1xVCI1, VGH = 5xVCI1, VGL = -2xVCI1 */
	ILI9163_CMD_VCOM_CONTROL1,2,0x50,0x5b,  /* VMH = 80: VCOMH voltage = 4.5, VML = 91: VCOML voltage = -0.225 */
	ILI9163_CMD_VCOM_OFFSET_CONTROL,1,0x40, /* nVM = 0, VMF = 64: VCOMH output = VMH, VCOML output = VML */
	ILI9163_CMD_SET_COLUMN_ADDRESS,4,0,0,0,0x7f, /* XSH,XSL,XEH,XEL */
	ILI9163_CMD_SET_PAGE_ADDRESS,4,0,0,0,0x7f,   /* YSH,YSL,YEH,YEL */
	ILI9163_CMD_SET_ADDRESS_MODE,1,ILI9163_RGB,	/* rotation=0, (BGR=8) */
	ILI9163_CMD_SET_DISPLAY_ON,0,
	ILI9163_CMD_WRITE_MEMORY_START,0,
#endif
	LCD_CMD_END, 0                             /* end of list: no cmd, no data */
};

#ifdef LCD_PIN_CS
#define SETCS 	BitSet(LCD_PORT_CS,   LCD_PIN_CS   );
#define CLRCS   BitClr(LCD_PORT_CS,   LCD_PIN_CS   );
#else
#define SETCS
#define CLRCS
#endif

/* ******************************************************************************** */

/* 
  main init call: initialize display 

  after this call, the display should be able to accept drawing commands and 
  show text/images
*/
void LCD_init( void )
{
	unsigned char ndat,cmd,delay;
	const unsigned char *ptr = LCD_init_seq;

	/* set pins to output: code could be simplied if same ports are used */
#ifdef LCD_PIN_CS
	BitSet( LCD_DDR_CS,   LCD_PIN_CS   );
#endif
#ifdef LCD_PIN_RES
	BitSet( LCD_DDR_RES,  LCD_PIN_RES  );
#endif
	BitSet( LCD_DDR_CLK,  LCD_PIN_CLK  );
	BitSet( LCD_DDR_DATA, LCD_PIN_DATA );
	BitSet( LCD_DDR_CD,   LCD_PIN_CD   );

	/* reset display and stay a while here */
	SETCS
#ifdef LCD_PIN_RES
	BitSet(LCD_PORT_RES,  LCD_PIN_RES  );
	_delay_ms(1);
	BitClr(LCD_PORT_RES,  LCD_PIN_RES  );
	_delay_ms(50);
	BitSet(LCD_PORT_RES,  LCD_PIN_RES  );
#endif
	_delay_ms(120);

#ifndef _LCD_SOFT_SPI
	BitClr(LCD_PORT_CLK,  LCD_PIN_CLK   ); /* clean start of HW SPI */
	BitClr(LCD_PORT_DATA, LCD_PIN_DATA  );
	/* double speed (SPSR|1), clock divider in SPR Bits 8 (01 -> clock*2/16=2 MHz @ clock=16MHz, 00=f*2/4 -> 8 MHz ) */
	SPSR |= 1;  /* enable double speed  */ // SPSR &= ~1; // disable double speed */
	SPCR  = (0<<SPIE)|(1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);	
#endif
	/* initialization sequence */
	while( (cmd = pgm_read_byte( ptr++ )) != LCD_CMD_END )
	{
		ndat   = pgm_read_byte( ptr++ );
		delay  = ndat & LCD_CMD_DELAY;
		ndat  -= delay;

		LCD_CMD(cmd);

		while( ndat-- ) /* optional: data for command */
		{
			LCD_DTA(pgm_read_byte( ptr++ ));
		}

		if( delay ) /* optional: delay after command+data */
		{
			delay = pgm_read_byte( ptr++ );
			while( delay-- )
				_delay_ms(1);
		}
	}
}


/* 
  one byte output via SPI

  note: 3 implementations in here
        - Software Slow ( #define _LCD_SOFT_SPI, #undef LCD_PIN_CLKDATA  )
	- Software Fast ( #define _LCD_SOFT_SPI, #define LCD_PIN_CLKDATA )
	- Hardware      ( #undef _LCD_SOFT_SPI)
        -> sorry for the whole lot of preprocessor macros, I wanted everything in one active
	   source code path, regardless of fast or slow SPI mode
	-> speed over size here, the loop is always unrolled
*/
#ifdef _LCD_SOFT_SPI
void LCD_SPI( unsigned char dta )
{
#ifndef LCD_PIN_CLKDATA 

#define SPI_BIT_START( _val_, _bit_, _portd_,_pind_,_portc_,_pinc_,_nbit_)
	/* this code supports different ports for CLK, DATA but is slower (~2.6 MHz @ 16 MHz MCU) */
#define SPI_BIT( _val_, _bit_, _portd_,_pind_,_portc_,_pinc_,_nbit_) \
		BitClr( _portd_, _pind_ ); \
		if( (_val_ & (1<<_bit_)) ) BitSet( _portd_, _pind_ ); \
		BitSet( _portc_, _pinc_);\
		BitClr( _portc_, _pinc_);

#else /* LCD_PIN_CLKDATA */
	/* this code is way faster (fastest SW SPI known to date) but requires CLK,DATA on the same port */

/* this lower code reqires clk/data on the same port and is a bit faster (test bench 11s) */
	register unsigned char regclr = LCD_PORT_DATA; /* CLK may be high, Data bit is copied anyway */
	register unsigned char regclk;
#define SPI_BIT_START( _val_, _bit_, _portd_,_pind_,_portc_,_pinc_,_nbit_) \
	asm volatile ( "bst %0,%1": : "r" (_val_), "I" (_bit_)   ); \
	asm volatile ( "ldi %0,%1": "=r" (regclk) : "I" (1<<_pinc_) ); \
	asm volatile ( "cbr %0,%1": "+r" (regclr) : "i" (1<<_pinc_) );

/* this one requires _portd_ == _portc_, at 16 MHz, the "nop" is required, though  */
#define SPI_BIT( _val_, _bit_, _portd_,_pind_,_portc_,_pinc_,_nbit_) \
	asm volatile ( "bld %0,%1": "+r" (regclr) : "I" (_pind_)  );\
	asm volatile ( "out %0,%1": : "I" (_SFR_IO_ADDR(_portd_)), "r" (regclr)  );\
	asm volatile ( "bst %0,%1": : "r" (_val_), "I" (_nbit_)   );\
	asm volatile ( "out %0,%1": : "I" (_SFR_IO_ADDR(_portd_)-2), "r" (regclk) );

#endif /* LCD_PIN_CLKDATA */

	SPI_BIT_START(dta,7,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,6);
	SPI_BIT(dta,7,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,6);
	SPI_BIT(dta,6,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,5);
	SPI_BIT(dta,5,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,4);
	SPI_BIT(dta,4,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,3);
	SPI_BIT(dta,3,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,2);
	SPI_BIT(dta,2,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,1);
	SPI_BIT(dta,1,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,0);
	SPI_BIT(dta,0,LCD_PORT_DATA,LCD_PIN_DATA,LCD_PORT_CLK,LCD_PIN_CLK,0);
	/* clock stays high */
}
#else /* _LCD_SOFT_SPI */
inline void LCD_SPI(unsigned char data)
{
	SPDR  = data;
	while(!(SPSR & (1<<SPIF)));
}
#endif /* _LCD_SOFT_SPI */


/*
  SPI command: set RS bit aka C/D, then chip select 
*/
void LCD_CMD( unsigned char cmd )
{
	BitClr(  LCD_PORT_CLK, LCD_PIN_CLK  );
	BitClr(  LCD_PORT_CD,  LCD_PIN_CD   );
	CLRCS
	LCD_SPI( cmd );
	SETCS
	BitSet(  LCD_PORT_CD,  LCD_PIN_CD   );
}


/*
  SPI data: clear clock, then chip select 
*/
void LCD_DTA( unsigned char dta )
{
	BitClr(  LCD_PORT_CLK, LCD_PIN_CLK  );
	CLRCS
	LCD_SPI( dta );
	SETCS
}

/*
  SPI Data, clear clock, then chip select for 16 Bit data
*/
void LCD_DTA16( unsigned int dta )
{
	BitClr(  LCD_PORT_CLK, LCD_PIN_CLK  );
	CLRCS
	LCD_SPI( dta>>8 );
	LCD_SPI( dta    );
	SETCS
}

/* Stream Write support: initialize CS once, then write Pixel by Pixel 
   - if disabled, the "normal" LCD_DTA*() routines are used
*/
#ifndef _LCD_STREAM_SUPPORT
#define LCD_STREAM_START
#define LCD_STREAM_STOP
#define LCD_STREAM8(a)  LCD_DTA(a)
#define LCD_STREAM16(a) LCD_DTA16(a)
#else
#define LCD_STREAM_START \
	BitClr(  LCD_PORT_CLK, LCD_PIN_CLK  );\
	CLRCS
#define LCD_STREAM_STOP \
	SETCS
#define LCD_STREAM8( a )\
	LCD_SPI( a );
#define LCD_STREAM16(a ) \
	LCD_SPI( (a)>>8 );\
	LCD_SPI( (a) );
#endif



/*
  Fill screen with specified color
  - handles any rotation properly
*/
void LCD_FillScreen( color_t color )
{
#if (LCD_Height != 128 )
	/* swap width/height if rectangular display */
	if( lcd_orientation )
		LCD_FillRect( 0,0, LCD_Height, LCD_Width, color );
	else
#endif
		LCD_FillRect( 0,0, LCD_Width, LCD_Height, color );
}


/*
  fill rectangle
   - start at x,y coordinates and draw given number of pixels
   - width/height >=
   - color is either 8 or 12 bit, depending on operation mode
     (see LCD_RGB() macro)
   notes:
    you may use LCD_FillScreen(color) macro as fullscreen fill
*/
void LCD_FillRect( unsigned char x, unsigned char y, unsigned char w, unsigned char h, color_t color )
{
	unsigned int n;
	unsigned int c1;
#ifdef _LCD_12BIT
	unsigned char c2;
	c1 = (color<<4) | (color >> 8); // 16 bit RRRRGGGGBBBBRRRR
	c2 = color & 0xff;
	n     = (w*h)>>1;
#else
  #ifdef _LCD_16BIT
 	n  = w*h;
	c1 = color;
  #else
 	unsigned char c2;
 	n  = w*h;
	c1 = (color & 0xf800) | ((color&0x7E0)>>3); /* R8G8 */
	c2 = color<<3;
  #endif
#endif
	LCD_SetActiveArea( x, y, x+w-1, y+h-1 );

	LCD_STREAM_START
	while( n-- )
	{
		LCD_STREAM16( c1 ); /* write 16 bit in any case */
#ifndef _LCD_16BIT
		LCD_STREAM8( c2  ); /* write additional byte in 12/18 bit modes (2 byte per 2 pixel or 3 byte per pixel) */
#endif
	}
	LCD_STREAM_STOP
}


/* (optional) rotation support affects only the routine below and the ActiveArea call */
#ifdef _LCD_ROTATION_SUPPORT
const unsigned char PROGMEM LCD_rots[4]   = {0,0x60,0xC0,0xA0};
const unsigned char PROGMEM LCD_shifts[4] = {0,0,32,1};
#if (LCD_Height == 128 )
#define LCD_xcorr( a ) ( (a) + (LCD_orientation<<5) )
#define LCD_ycorr( a ) ( (a) + (LCD_orientation&32) )
#else
/* TODO: proper offsets for other display sizes (not needed for 128x160) */
const unsigned char PROGMEM LCD_shifts[4] = {0,1,0,1}; /* used as swap w/h flag for FillScreen() when hsize != 128 */
#define LCD_xcorr( a ) a
#define LCD_ycorr( a ) a
#endif
/* 
  optional:  set orientation (rotation) of display for text and graphics output
  arguments: rot = LCD_ROT_x, where x is 0,90,180,270 (e.g. LCD_ROT_90)
  note:     when rotated, keep in mind to swap LCD_Width/LCD_Height when limiting
            coordinates
*/
void LCD_Orientation( unsigned char rot )
{
	LCD_orientation = pgm_read_byte(&LCD_shifts[ rot ]); /* orientation encodes pixel shifts */
 
	LCD_CMD( ILI9163_CMD_SET_ADDRESS_MODE );
	LCD_DTA( ILI9163_RGB | pgm_read_byte( &LCD_rots[rot] ) ); 
}
#else
/* dummy rotation functions if commented out in header file */
#define LCD_xcorr( a ) (a)
#define LCD_ycorr( a ) (a)
void LCD_Orientation( unsigned char rot ){ }
#endif


/*
  set screen area for subsequent data writes
   - xstart,ystart are relative to upper left corner
   - xstop, ystop are x+write_width-1, y+write_height-1
   - even in rotated modes, no changes to coordinate handling are necessary in the calling code
   - write direction for subsequent data transfers is always in row-by-row order (natural raster scan)
*/
void LCD_SetActiveArea( unsigned char xstart, unsigned char ystart, unsigned char xstop, unsigned char ystop )
{
	LCD_CMD( ILI9163_CMD_SET_COLUMN_ADDRESS );
	LCD_DTA16( LCD_xcorr(xstart) );
	LCD_DTA16( LCD_xcorr(xstop)  );

	LCD_CMD( ILI9163_CMD_SET_PAGE_ADDRESS   );
	LCD_DTA16( LCD_ycorr(ystart)  );
	LCD_DTA16( LCD_ycorr(ystop)   );

	LCD_CMD( ILI9163_CMD_WRITE_MEMORY_START );
}

#ifdef _LCD_SHOWIMAGE_SUPPORT
/*
  display image on screen at coordinates x,y with size w,h
  pointer "img" must be in flash memory
  - if you need data in RAM, just clone this routine and
    remove the pgm_read_byte wrapper
*/
void LCD_ShowImage(unsigned char x, unsigned char y, unsigned char w, unsigned char h, const unsigned char *img )
{
	unsigned int pixels = w*h;

	pixels = PIXELS2BYTES( pixels ); /* *3/2 for 12 Bit, *2 for 16 Bit, *3 for 18 Bit */

	LCD_SetActiveArea( x, y, x+(w-1), y+(h-1) );

	LCD_STREAM_START
	while( pixels-- )
	{
		LCD_STREAM8( pgm_read_byte( img++ ) );
	}
	LCD_STREAM_STOP
} 
#endif /* _LCD_SHOWIMAGE_SUPPORT */


#ifdef _LCD_SHOWIMAGESC_SUPPORT
/*
  display image on screen at coordinates x,y with size dw,dh, from an image of size w,h
  pointer "img" must be in flash memory
  - if you need data in RAM, just clone this routine and
    remove the pgm_read_byte wrapper
  - classic bresenham
*/
void LCD_ShowImageSC(unsigned char x, unsigned char y, 
                        unsigned char dw, unsigned char dh,
			unsigned char w,  unsigned char h,
			const unsigned char *img)
{
	unsigned char i,j;
	int err,erry;
	int iy;
	const unsigned char *imgp;
#ifdef _LCD_12BIT
	unsigned char p0=0,icount=0,ocount=0,lcount=0;
	unsigned int  p;
#define STRIDE  icount+1
#define TOGGLEI icount ^= 1;
#else
#define TOGGLEI 
#ifdef _LCD_16BIT
#define STRIDE 2
#else
#define STRIDE 3
#endif
#endif /* _LCD_12BIT */

	LCD_SetActiveArea( x, y, x+(dw-1), y+(dh-1) );
	erry = dh>>1;
	for( j=0,iy=h ; j < dh; j++ )
	{
		err  = dw>>1;
		imgp = img;
		for( i=0 ; i< dw ; i++ )
		{
#ifdef _LCD_12BIT
			p = (((unsigned int)pgm_read_byte( imgp  ))<<8) | (unsigned int)pgm_read_byte( imgp+1 );
			if( icount )
				p <<= 4;
			/* p: RRRRGGGGBBBB0000 */
			if( ocount )
			{
				p0 |=  (p>>12 ); /* BBBBRRRR */
				LCD_DTA( p0   );
				LCD_DTA( p>>4 ); /* GGGGBBBB */
			}
			else
			{
				p0 = p; /* BBBB0000 */
				LCD_DTA( (unsigned char)(p>>8) );
			}
			ocount ^= 1;
#else /* _LCD_12BIT */
			LCD_DTA( pgm_read_byte( imgp   ) );
			LCD_DTA( pgm_read_byte( imgp+1 ) );
#ifdef _LCD_18BIT
			LCD_DTA( pgm_read_byte( imgp+2 ) );
#endif
#endif /* _LCD_12BIT */
			err += w;
			while( err >= dw )
			{
				imgp   += STRIDE;
				TOGGLEI /* 12 bit only */
				err -= dw;
			}
		}
		erry += h;
		while( erry >= dh ) /* works but not too efficient when h much larger than dh */
		{
			erry -= dh;
			iy--;
			if( iy > 0 )
			{
#ifdef _LCD_12BIT
				img    += w + (w>>1) + lcount;
				lcount ^= (w&1);
				icount  = lcount;
#else
				img  += w*STRIDE;
#endif
			}
		}
	}
} 
#endif /* _LCD_SHOWIMAGESC_SUPPORT */



#ifdef _LCD_FONT_SUPPORT
/* helper macros for writing one pixel in Putc and SetPixel, depending on active color mode 
   (didn't want to pollute the main calls with more ifdefs than necessary) 
*/
#ifdef _LCD_12BIT

  #define LCD_PP_TMPVARS unsigned int store=0;
  #define LCD_PP_SETUP( arg, _f_color,_b_color )
  #define LCD_PUTPIXEL( color, idx,shift ) \
  	if( shift ) \
	{	LCD_DTA16(color<<4);}\
	else { \
	if( idx & 1)\
	{     LCD_DTA16(store|color);}\
	else\
	{     LCD_DTA( (color)>>4 ); store = color<<12; }}

#else
  #ifdef _LCD_16BIT

	#define LCD_PP_TMPVARS
	#define LCD_PP_SETUP( arg,_f_color,_b_color )
	#define LCD_PUTPIXEL( color, idx, shift ) \
		LCD_DTA16(color);

  #else

	#define LCD_PP_TMPVARS unsigned int true_color[4];
	#define LCD_PP_SETUP(arg,_f_color,_b_color) \
		true_color[0] = (_f_color & 0xf800) | ((_f_color>>3)&0xfc);\
		true_color[1] = (_f_color & 0x1f)<<3;\
		if( arg == 2 ) \
		{\
		 true_color[2] = (_b_color & 0xf800) | ((_b_color>>3)&0xfc);\
		 true_color[3] = (_b_color & 0x1f)<<3;\
		 _b_color = 2;\
		}\
		_f_color = 0;
	#define LCD_PUTPIXEL( color, idx, shift ) \
		LCD_DTA16(true_color[color]);\
		LCD_DTA(true_color[color+1]);

  #endif
#endif


/* "convenience" functions for text display with default font */
unsigned char LCD_Putc(char c, unsigned char x, unsigned char y, color_t f_color, color_t b_color )
{
	return LCD_PutcF(c,x,y,f_color,b_color,DEF_FONT);
}
unsigned char LCD_Puts(char *str, unsigned char x, unsigned char y, color_t f_color, color_t b_color)
{
	return LCD_PutsF(str,x,y,f_color,b_color,DEF_FONT);
}
unsigned char LCD_Putsf(const char *str, unsigned char x, unsigned char y,color_t f_color, color_t b_color )
{
	return LCD_PutsfF( str,x,y,f_color,b_color,DEF_FONT);
}


/*
  write character on LCD
  - use LCD_RGB( r,g,b ) to obtain f_color and b_color (where r,g,b are in 4 bit)
*/
unsigned char LCD_PutcF(char c, unsigned char x, unsigned char y, color_t f_color, color_t b_color, const Bax_font*f )
{
    char z=0;
    unsigned char pos,max;
    unsigned char i;
    const unsigned char *fntchar;
    unsigned int col;
    LCD_PP_TMPVARS
#define FGETB(a) pgm_read_byte(&a)
#define FGETW(a) pgm_read_word(&a)


    LCD_SetActiveArea( x, y, x-1+FGETB(f->width), y-1+FGETB(f->height) ); /* 1-zoom = 1 or 2 */

    c -= FGETB(f->char_min);
    if( c < 0 ) c=0; /* space */
    if( c > (FGETB(f->char_max)-FGETB(f->char_min))) c=0; /* space */
    fntchar = (const unsigned char*)FGETW(f->data) + FGETB(f->stride) * c;

    LCD_PP_SETUP(2,f_color,b_color)

    pos = 0;
    max = FGETB(f->width) * FGETB(f->height);
    for( i=0; i < max; i++ )
    {
    	if( pos == 0 )
	{
    		z  = pgm_read_byte( fntchar++ );
		pos = 128;
	}
	col = b_color;
	if( z & pos )
	{
		col = f_color;
	}
	LCD_PUTPIXEL( col, i, 0 ); /* this is not SetPixel() ! */
	pos >>= 1;
    }
 
    return x+FGETB(f->width);
}


/*
  write 0-terminated String, \r or \n will force a newline (start at x=0 again)
*/
unsigned char LCD_PutsF(char *str, unsigned char x, unsigned char y, color_t f_color, color_t b_color, const Bax_font *f )
{
    unsigned char c;
              
    while(1)
    {
    	c=*str++;
	if( !c )
		break;

	if( (c=='\r') || (c=='\n') )
	{
		x = 0;
		y += FGETB(f->height);
		if( y > (LCD_Height-FGETB(f->height)) )
			y = 0;

		continue;
	}

        x = LCD_PutcF( c, x, y, f_color, b_color,f);
    }  

    return x;
}

/*
  write 0-terminated String from flash memory, \r or \n will force a newline (start at x=0 again)
*/
unsigned char LCD_PutsfF(const char *str, unsigned char x, unsigned char y, color_t f_color, color_t b_color, const Bax_font *f)
{
    unsigned char c;
              
    while(1)
    {
    	c=pgm_read_byte(str++);
	if( !c )
		break;

	if( (c=='\r') || (c=='\n') )
	{
		x = 0;
		y += FGETB(f->height);
		if( y > (LCD_Height-FGETB(f->height)) )
			y = 0;

		continue;
	}

        x = LCD_PutcF( c, x, y, f_color, b_color,f );
    }  

    return x;
}
#endif /* _LCD_FONT_SUPPORT */


/*
  Set a single pixel to specified color

  note: don't use this to draw hor/ver lines,
        use LCD_FillRect() with width/height 1
	instead
*/
void LCD_SetPixel( unsigned char x, unsigned char y, color_t f_color )
{
    LCD_PP_TMPVARS

    LCD_SetActiveArea( x, y, x, y ); /* this area call makes "SetPixel" quite inefficient */

    LCD_PP_SETUP(1,f_color,f_color)

    LCD_PUTPIXEL( f_color, 1, 1 );
}



#if (defined _LCD_PUTLINE_SUPPORT) && !(defined _LCD_SOFT_SPI)
/*
  display line on screen at coordinates x,y with size w
  pointer "img" must be in RAM
  color map: 
   - PROGMEM
   - 256 entries
   - 12 bit RRRRGGGGBBBB in 12 bit mode
   - 16 bit RGB565 in 16/18 bit mode
*/
void LCD_PutLine(unsigned char x, unsigned char y, unsigned char w, char *img, const color_t *colormap )
{
	unsigned char a;
	unsigned int  b;
#ifdef _LCD_12BIT
	unsigned char c;
	w>>=1;
#endif
	LCD_SetActiveArea( x, y, x+(w-1), y );

	LCD_STREAM_START

	a   = *img++;
	b   = pgm_read_word(&colormap[ a ]); /* colormap is lower 12 bit in 12 bit mode */
#ifdef _LCD_12BIT
	c   = b << 4;             /* lower 4 bit of first 12 bit color word into next byte */
#endif
	b >>= 4;                  /* get upper 8 bit */
	while( w-- )
	{
#ifdef _LCD_12BIT
		SPDR  = b;         //LCD_STREAM8( b>>4 ); /* upper 8 bit */

		a   = *img++;
		c  |= pgm_read_byte(1+(unsigned char*)&colormap[ a ]); /* colormap is lower 12 bit in 12 bit mode */

		while(!(SPSR & (1<<SPIF)));
		SPDR = c;

		c  = pgm_read_byte(0+(unsigned char*)&colormap[a]);
	
		while(!(SPSR & (1<<SPIF)));
		SPDR = c;

		a  =  *img++;
		b  =  pgm_read_word(&colormap[ a ]); /* colormap is lower 12 bit in 12 bit mode */
		c  =  b<<4;
		b >>= 4;

		while(!(SPSR & (1<<SPIF)));
#endif
#ifdef _LCD_16BIT
		a = *img++;
		b = pgm_read_word(&colormap[ a ]);
		LCD_STREAM16( b );   
#endif
#ifdef _LCD_18BIT
		a = *img++;
		b = pgm_read_word(&colormap[ a ]); /* 16 bit color -> convert to 18 Bit */
		
		LCD_STREAM8( (b>>8) & 0xf8 );
		LCD_STREAM8( (b>>3) & 0xfc );
		LCD_STREAM8( (b<<3) );
#endif
	}
	LCD_STREAM_STOP
} 

#endif /* _LCD_PUTLINE_SUPPORT */

#ifdef _LCD_SHOWIMAGE8_SUPPORT 
/* show 8 Bit chunky pixel image in current mode, apply supplied color map 
   inputs:
    - coordinates as usual
    - image pointer to 8 bit image in flash memory
    - color map (e.g. 
       const color_t ex_ctab[4] PROGMEM={LCD_RGB(255,255,255),LCD_RGB(0,0,0),LCD_RGB(0,0,255),LCD_RGB(255,0,0)}; 
      )
    - the color map should cover all used colors (of course) but doesn't need to be 8 bit deep if the number of
      actually used indices is lower

    - Note: slow in 12 bit mode
*/
void LCD_ShowImage8(unsigned char x, unsigned char y, unsigned char w, unsigned char h, const unsigned char *img, const color_t *colormap )
{
 unsigned char i,a;
 unsigned int  b;
#ifdef _LCD_12BIT
 unsigned char c=0;
 unsigned int  ct=0;
#endif

	LCD_SetActiveArea( x, y, x+(w-1), y+(h-1) );

	LCD_STREAM_START

	while( h-- )
	{
		for( i=0; i<w ; i++ )
		{
#ifdef _LCD_12BIT
			if( ct&1 )
			{
				a      = pgm_read_byte(img++);
				b      = pgm_read_word(&colormap[ a ]);
				c     |= (b>>8);
				LCD_STREAM8(c);
				LCD_STREAM8(b);
			}
			else
			{
				a     = pgm_read_byte(img++);
				b     = pgm_read_word(&colormap[ a ]); /* colormap is lower 12 bit in 12 bit mode */
				c     = b << 4;             /* lower 4 bit of first 12 bit color word into next byte */
				b   >>= 4;                  /* get upper 8 bit */
				LCD_STREAM8(b);
			}
			ct++;
#endif
#ifdef _LCD_16BIT
			a = pgm_read_byte(img++);
			b = pgm_read_word(&colormap[ a ]);
			LCD_STREAM16( b );   
#endif
#ifdef _LCD_18BIT
			a = pgm_read_byte(img++);
			b = pgm_read_word(&colormap[ a ]); /* 16 bit color -> convert to 18 Bit */
			
			LCD_STREAM8( (b>>8) & 0xf8 );
			LCD_STREAM8( (b>>3) & 0xfc );
			LCD_STREAM8( (b<<3) );
#endif
		}
	}
	LCD_STREAM_STOP
}
#endif

#undef ILI9163_c /* make sure, globals in ILI9163.h are not compiled twice... */

