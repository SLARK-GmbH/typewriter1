/*
 ********************************************************************************
 * demoloop.c                                                                   *
 *                                                                              *
 * Author: Henryk Richter <bax@comlab.uni-rostock.de>                           *
 *                                                                              *
 * Purpose: ILI9163/ST7735 display control demo                                 *
 *                                                                              *
 * default: re-draw screen with some text, color bars and an image              *
 * control: scripted                                                            *
 *                                                                              *
 ********************************************************************************
*/
#include <avr/pgmspace.h>
#include <util/delay.h> /* might be <avr/delay.h>, depending on toolchain */

#include "ILI9163.h"

#define  DFTIMG_MAIN
#define  DFT32_MAIN
#include "DFT48x39.h"
#include "DFT32x32_32Col.h"
#undef   DFTIMG_MAIN
#undef   DFT32_MAIN

/* used fonts (MCUs with larger flash >8k can use the larger font as well) */
#if 0 
#define FONT_MAIN
#include "font_Bitumen_ascii+dft_9x16.h"
#else
#include "font_screen_ascii_5x8v2_hor.h"
#define font_9_16 font_5_8
#endif

#ifdef _LCD_12BIT
 #define BITSTR ",12 Bit"
#else
 #ifdef _LCD_16BIT
  #define BITSTR ",16 Bit"
 #else
  #define BITSTR ",18 Bit"
 #endif
#endif


#define SCRIPT_MAX 23
#ifdef SCRIPT_MAX
struct script {
	char mode;
	int  time;
};

/* r,t,p,f,g */
const struct script demoscript[SCRIPT_MAX] = {
 {  0,   100 },

 /* 16x rotation */
 {'r',   1},{'r',   1},{'r',   1},{'r',   1},
 {'r',   1},{'r',   1},{'r',   1},{'r',   1},
 {'r',   1},{'r',   1},{'r',   1},{'r',   1},
 {'r',   1},{'r',   1},{'r',   1},{'r',   1},

 {'z', 180}, /* image zoom      */
 {'d', 100}, /* 8 bit image write */

 {'f', 100}, /* area fill 128x1 */
 {'g', 100}, /* fill screen     */

 {'p',  10}, /* Put pixel */
 {'t',  20}, /* Putc()    */
};
#endif

const char PROGMEM bitstring[] = { BITSTR };
const char PROGMEM titlestring[] = { "Redraw text/img "BITSTR };

#if 1 
/*
  show status on LCD:
   - write counter (a,b are decimal counter = a*10+b)
   - str is the prefix string
   - suffixed by number of Bits per pixel
*/
void LCDShowStatus( char *str, char a, char b )
{
 char x = 0;
		x = LCD_Putc( '0'+a, x,120,LCD_WHITE,LCD_BLACK); 
		x = LCD_Putc( '0'+b, x,120,LCD_WHITE,LCD_BLACK); 
		if( str != (0) )
		{
			x = LCD_Puts( str,   x,120,LCD_RGB(127,127,127),LCD_WHITE);
			x = LCD_Putsf( bitstring,x,120,LCD_RGB(127,127,127),LCD_WHITE);
		}
}
#else
void LCDShowStatus( char *str, char a, char b ) { /* */ }
#endif

void demoloop(void)
{
  unsigned char i,j;
  char r=0,r2=0,r3=0,r4=0,o=0,t; // BCD frame counter (yes, I could define nibbles...)
  unsigned int totaltime=0;              // total frame counter
  unsigned int nexttime=0;
  unsigned char   modeidx =0;
  char mode = 0;
  unsigned int time=0;

  /* init display, set default orientation (optional) */
  LCD_init();
  o = LCD_ROT_270;
  LCD_Orientation(o);

  LCD_FillScreen( LCD_RGB(255,255,255) ); /* white where R,G,B are at max. value 255 */

#ifndef SCRIPT_MAX
  mode     = 0;
#else
  mode     = demoscript[modeidx].mode;
  nexttime = demoscript[modeidx].time + totaltime;
#endif
  while(1)
  {
  	switch( mode )
	{
	 case 0:
		LCD_PutcF( '0'+r, 29,40,LCD_BLACK,LCD_WHITE,&font_9_16);  // double size char
		LCD_PutcF( '0'+r2, 20,40,LCD_BLACK,LCD_WHITE,&font_9_16); // double size char
		LCD_PutsF("ILI9163",40,40,LCD_BLUE,LCD_WHITE,&font_9_16);
		LCD_Putsf(titlestring,0,120,LCD_RGB(127,127,127),LCD_WHITE);

		/* color bars */
		for( i=0 ; i<16; i++ )
		{
			LCD_Putc('r',i*5,56,0,LCD_RGB(i<<4,0,0));
			LCD_Putc('g',i*5,64,0,LCD_RGB(0,i<<4,0));
			LCD_Putc('b',i*5,72,0,LCD_RGB(0,0,i<<4));
			LCD_Putc('w',i*5,80,0,LCD_RGB((i<<4),(i<<4),(i<<4)));
		}
#ifdef DFTIMG_WIDTH
		LCD_ShowImage( 0,0,DFTIMG_WIDTH,DFTIMG_HEIGHT,(const unsigned char*)DFTIMG_PTR);/* ShowImage is for high/true color */
#endif
		break;
	  /* 8 bit indexed color image display test */
	 case 1:
#if (defined _LCD_SHOWIMAGE8_SUPPORT) && (defined DFT32_WIDTH)
		{
		 unsigned char k=time & 1;

	 	 for(j=0; j < LCD_Height ; j+= DFT32_HEIGHT )
		 {
			for( i=0; i < LCD_Width ; i+= DFT32_WIDTH )
			{
				k ^= 1;
				LCD_ShowImage8( i,j,DFT32_WIDTH,DFT32_HEIGHT,DFT32_PTR,DFT32_COLTAB + (k<<5) ); /* indexed color image routine */
			}
			k ^= 1;
		 }
		 time++;
		 LCDShowStatus( "LCD_ShowImage8()",r2,r );
		}
#else
		totaltime=nexttime;
#endif
	 	break;
	 /* image scaling test */
	 case 2:
#if (defined DFTIMG_WIDTH) && (defined _LCD_SHOWIMAGESC_SUPPORT)
	 	{
		#define sc_maxt 180
		 unsigned int t0;
		 unsigned char x,y,w,h;

	 	 if( time > sc_maxt*2 )
			 time = 0;
		 t0=time;
		 if( t0 > sc_maxt )
		 {
			t0=(sc_maxt*2)-t0;
		 }
	 	 //if( t0 < 250 )
		 x  = (LCD_Width>>1);
		 y  = (LCD_Height>>1);
		 w  = (LCD_Width*(t0+6))  >> 8;
		 x -= (w>>1);
		 h  = (LCD_Height*(t0+6)) >> 8;
		 y -= h>>1;
		 LCD_ShowImageSC(x,y,w,h,DFTIMG_WIDTH,DFTIMG_HEIGHT,(const unsigned char*)DFTIMG_PTR);
		}
		time+=2;
#else
		totaltime=nexttime;
#endif
	 	break;
	 case 3:
		/* pixel draw test */
		for( j = 0 ; j < LCD_Height ; j++ )
		{
		 for( i=0 ;  i < LCD_Width  ; i++ )
		 {
			LCD_SetPixel( i, j, LCD_RGB( i<<1, j<<1, (r+r2*10) )); 
		 }
		}
		LCDShowStatus( "SetPixel()",r2,r );
		break;
	 case 4:
		/* pixel draw test */
		for( j = 0 ; j < LCD_Height ; j++ )
		{
			LCD_FillRect( 0, j, LCD_Width,1, LCD_RGB(j<<1,j<<1,j<<1)); 
		}
		LCDShowStatus( "FillRect() 128x1",r2,r );
		break;
	 case 5:
		LCD_FillRect( 0, 0, LCD_Width,LCD_Height, LCD_RGB(r<<4,r<<4,r2<<4)); 
		LCDShowStatus( "FillScreen()",r2,r );
		break;
	 case 6:
	        t=32+r+r2*10;
	 	for( j=0 ; j < (LCD_Height) ; j+= 8 )
		{
		 	for( i=0 ; i < (LCD_Width-5) ; i += 5 )
			{
				LCD_Putc( t ,i,j,LCD_WHITE,LCD_BLACK);
				t++;
				if( t < 0 )
				 t=32+r;
			}
		}
	 default:
	 	break;
	}
	totaltime++;
#ifdef SCRIPT_MAX
	t = 0;
	if( totaltime >= nexttime )
	{
		modeidx++;
		if( modeidx >= SCRIPT_MAX )
			modeidx = 0;

  		t        = demoscript[modeidx].mode;
  		nexttime = demoscript[modeidx].time + totaltime;
	}
#endif
	r++;
	if( r > 9 )
	{
		r=0;
		r2++;
		if( r2 > 9 )
		{
		 r3++;
		 r2=0;
		 if( r3 > 9 )
		 {
			r4++;
			r3=0;
			if( r4 > 9 )
				r4=0;
		 }
		}
	}
#ifdef SCRIPT_MAX
	if( t )
	{
#else
#endif		
		switch( t )
		{
			case 'r':
				o++;o&=3;
				//LCD_Orientation( o );
				LCD_FillScreen( LCD_WHITE );
				mode = 0;
				break;
			case 't':
				mode = 6;
				LCD_FillScreen( LCD_BLACK );
				break;
			case 'p':
				mode = 3;
				break;
			case 'f':
				mode = 4;
				break;
			case 'g':
				mode = 5;
				break;
			case 'd':
				mode = 1;
				break;
			case 'z':
				mode = 2;
				time = 0;
				LCD_FillScreen( LCD_BLACK );
				break;
			default:
				if( t<32 )
					mode = t;
				break;
		}
		LCD_Orientation( o );
	}

//	_delay_ms(100);
 }

}


