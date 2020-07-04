/* Copyright (c) 1992 AT&T - All rights reserved. */
/* internal libg implementation file - include after libg */

/*
 * include defs of standard library routines, if possible,
 * and string routines
 */
#ifdef _POSIX_SOURCE
#include <stdlib.h>
#include <string.h>
#endif /* _POSIX_SOURCE */

/* Return a GCs for solid filling/strings/etc., segments/points, and tiling */
extern GC	_getfillgc(Fcode, Bitmap*, unsigned long);
extern GC	_getcopygc(Fcode, Bitmap*, Bitmap*, int*);
extern GC	_getgc(Bitmap*, unsigned long, XGCValues *);

/* convert between different bitmap depths */
extern void	_ldconvert(char *, int, char *, int, int, int);

/* balloc without zero init (which uses a gc!) */
extern Bitmap	*_balloc(Rectangle, int);

/* X Display for this application's connection */
extern Display	*_dpy;

/* Default visual, used in loadbitmap */
extern Visual	*_vis;

/* Default colormap, used in string */
extern Colormap	_cmap;

/* screen depth foreground and background for this application */
extern unsigned long	_fgpixel, _bgpixel;
extern XColor		_fgcolor, _bgcolor;

/* indexed by log depth (0 <= ld <= 5), to give depth and planemask */
extern int		_ld2d[];
extern unsigned long	_ld2dmask[];

/* libg.h defines:
 *   extern Bitmap screen;   -- Bitmap for application Window after xbinit()
 *   extern Font *font;      -- Font for application default font after xbinit()
 */

/*
 * Conventions:
 *   The .id field of a Bitmap is an X Pixmap unless the Bitmap is screen,
 *   in which case it is a Window.
 *   The .id field of a Cursor is set to the X xCursor the first time the
 *   cursor is used.
 *   The .id field of a Font is set to the X xFont.
 *
 *   Coordinate conventions: libg bitmaps can have non (0,0) origins,
 *   but not X Pixmaps, so we have to subtract the min point of a Bitmap
 *   from coords in the Bitmap before using the point in the corresponding Pixmap.
 *   The screen Bitmap, however, contains the rectangle in X coords of the
 *   widget in which the application is started, relative to the window.
 *   The origin may or may not be (0,0), but in any case, coordinates should
 *   NOT be translated before using in X calls on the Window.
 */

/* values for bitmap flag field (see _getcopygc if change first two vals) */
enum {
	DP1=	0x1,	/* depth == 1 (ldepth == 0) */
	BL1=	0x2,	/* black == 1 model */
	SCR=	0x4,	/* on screen */
	ZORG=	0x8,	/* r.min == Pt(0,0) */
	SHIFT= 0x20,	/* !SCR & !ZORG */
	CLIP=  0x40	/* r != clipr */
};

/* values for return bltfunc arg of _getcopygc */
enum {
	UseCopyArea,
	UseCopyPlane,
	UseFillRectangle
};

XRenderColor makecol(ulong);
