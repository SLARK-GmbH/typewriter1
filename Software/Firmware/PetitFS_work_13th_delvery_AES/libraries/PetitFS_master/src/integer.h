/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _FF_INTEGER
#define _FF_INTEGER

#ifdef _WIN32	/* FatFs development platform */

#include <windows.h>
#include <tchar.h>

#else			/* Embedded platform */

/* This type MUST be 8 bit */
typedef unsigned char	PFS_BYTE;

/* These types MUST be 16 bit */
typedef short		PFS_SHORT;
typedef unsigned short	PFS_WORD;
typedef unsigned short	PFS_WCHAR;

/* These types MUST be 16 bit or 32 bit */
typedef int		PFS_INT;
typedef unsigned int	PFS_UINT;

/* These types MUST be 32 bit */
typedef long		PFS_LONG;
typedef unsigned long	PFS_DWORD;

#endif

#endif
