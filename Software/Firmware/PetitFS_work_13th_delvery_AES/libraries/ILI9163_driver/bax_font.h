/* ********************************************************************************
  file:
   bax_font.h 
   
  purpose:
   structured font handling, providing a flexible interface to
   different active fonts in one program


  author, copyright:
   Henryk Richter <bax@comlab.uni-rostock.de>

  version:
   0.1

  date:
   30-Apr-2016

  devices:

  acknowledgements:

  notes:
  
  stats:


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

*********************************************************************************** */
#ifndef BAX_FONT
#define BAX_FONT

struct bax_font {
	unsigned char char_min;       /* smallest character in font (usually 32) */
	unsigned char char_max;       /* largest character in font (typ. 127 or 255) */
	unsigned char stride;         /* offset from one character to the next, set 0 for proportional fonts */
	unsigned char width;          /* width (or average width in case of proportional fonts) of characters) */ 
	unsigned char height;         /* height of font */
	const unsigned char *data;    /* font data */
	const unsigned char *offsets; /* width table (length char_max-char_min+1) for proportional fonts (with stride==0) */
};
typedef struct bax_font Bax_font;

#endif /* BAX_FONT */
