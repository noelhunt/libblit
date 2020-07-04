/* Copyright (c) 1992 AT&T - All rights reserved. */
#ifndef _LIBG_H
#define _LIBG_H

#define Cursor	xCursor
#define	Event	xEvent
#define	String	xString
# include <X11/Xlib.h>
# include <X11/Xatom.h>
# include <X11/Xutil.h>
#undef	String
#undef	Event
#undef	Cursor

#define	Font	XFontStruct

#define fontheight(fp)	((fp)->max_bounds.ascent+(fp)->max_bounds.descent)
#define fontwidth(fp)	((fp)->max_bounds.width)
#define fontwidthc(fp,c) (XTextWidth((fp), &(c), 1))
#define fontnchars(fp)	((fp)->max_char_or_byte2+1)

#include <sys/time.h>

enum {
	RESHAPED  =  1,		/* window has been changed */
	KBD       =  2,		/* we have keyboard input */
	RCV       =  4,		/* recevied from "host" proc */
	MOUSE     =  8,		/* we always have the mouse */
	SEND      =  16,	/* for request compatability */
	CPU       =  32,
	ALARM     =  64,
	SELECT    =  128,	/* x11 selection */
	SNARF     =  256
};

#define nap(x)		Jnap(x)
#define wait(x)		Jwait(x)
#define sleep(x)	Jsleep(x)
#define alarm(x)	Jalarm(x)

#define own()		(P->state|MOUSE)

#define	min(x,y)	(((x) < (y)) ? (x) : (y))
#define	max(x,y)	(((x) > (y)) ? (x) : (y))

#define alloc(n)	calloc(n,1)
#define	muldiv(a,b,c)	((long)((a)*((long)b)/(c)))

#define button(i)	(mouse.buttons&(8>>i))
#define button1()	(mouse.buttons&4)
#define button2()	(mouse.buttons&2)
#define button3()	(mouse.buttons&1)
#define button12()	(mouse.buttons&6)
#define button13()	(mouse.buttons&5)
#define button23()	(mouse.buttons&3)
#define button123()	(mouse.buttons&7)

#define getrect1()	getrectb(4,1)
#define getrect2()	getrectb(2,1)
#define getrect3()	getrectb(1,1)
#define getrect12()	getrectb(6,1)
#define getrect13()	getrectb(5,1)
#define getrect23()	getrectb(3,1)
#define getrect123()	getrectb(7,1)

#define DBlack		0x000000
#define DWhite		0xFFFFFF
#define DRed		0xFF0000
#define DGreen		0x00FF00
#define DBlue		0x0000FF
#define DCyan		0x00FFFF
#define DMagenta	0xFF00FF
#define DYellow		0xFFFF00
#define DPaleyellow	0xFFFFAA
#define DDarkyellow	0xEEEE9E
#define DDarkgreen	0x448844
#define DPalegreen	0xAAFFAA
#define DMedgreen	0x88CC88
#define DDarkblue	0x000055
#define DPalebluegreen	0xAAFFFF
#define DPaleblue	0x0000BB
#define DBluegreen	0x008888
#define DGreygreen	0x55AAAA
#define DPalegreygreen	0x9EEEEE
#define DYellowgreen	0x99994C
#define DMedblue	0x000099
#define DGreyblue	0x005DBB
#define DPalegreyblue	0x4993DD
#define DPurpleblue	0x8888CC

/*
 * Types
 */
typedef unsigned char		uchar;

typedef struct Point Point;

struct	Point {
	int	x;
	int	y;
};

typedef struct Rectangle Rectangle;

struct Rectangle {
	Point min;
	Point max;
};

typedef	struct Bitmap Bitmap;

struct	Bitmap {
	Rectangle r;		/* rectangle in data area, local coords */
	Rectangle clipr;	/* clipping region */
	int	ldepth;
	int	id;		/* as known by the X server */
	Bitmap	*cache;		/* zero; distinguishes bitmap from layer */
	int	flag;		/* flag used by X implementation of libg */
};

typedef struct Mouse Mouse;

struct	Mouse {
	int	buttons; /* bit array: LMR=124 */
	Point	xy;
	ulong	msec;
};

typedef struct Cursor Cursor;

struct	Cursor {
	Point	offset;
	uchar	clr[2*16];
	uchar	set[2*16];
	int	id;	/* init to zero; used by library */
};

typedef struct Menu Menu;

struct Menu {
	char	**item;			/* string array, ending with 0	*/
	char	*(*generator)();	/* used if item == 0		*/
	short	prevhit;		/* private to menuhit()		*/
	short	prevtop;		/* private to menuhit() 	*/
};

typedef struct RGB RGB;

struct RGB {
	ulong	red;
	ulong	green;
	ulong	blue;
};

typedef struct JProc JProc;

struct JProc {
	int	state;
	Cursor	*cursor;
};

/*
 * Codes for bitblt etc.
 *
 *	       D
 *	     0   1
 *         ---------
 *	 0 | 1 | 2 |
 *     S   |---|---|
 * 	 1 | 4 | 8 |
 *         ---------
 *
 *	Usually used as D|S; DorS is so tracebacks are readable.
 */
typedef enum Fcode {
	Zero		= 0x0,
	DnorS		= 0x1,
	DandnotS	= 0x2,
	notS		= 0x3,
	notDandS	= 0x4,
	notD		= 0x5,
	DxorS		= 0x6,
	DnandS		= 0x7,
	DandS		= 0x8,
	DxnorS		= 0x9,
	D		= 0xA,
	DornotS		= 0xB,
	S		= 0xC,
	notDorS		= 0xD,
	DorS		= 0xE,
	F		= 0xF
} Fcode;

/*
 * Miscellany
 */

int	abs(int);
Point	addpt(Point, Point);
Point	subpt(Point, Point);
Point	mulpt(Point, int);
Point	divpt(Point, int);
Rectangle rectsubpt(Rectangle, Point);
Rectangle rectaddpt(Rectangle, Point);
Rectangle insetrect(Rectangle, int);
Rectangle rectmul(Rectangle, int);
Rectangle rectdiv(Rectangle, int);
Rectangle rectshift(Rectangle, int);
Rectangle canonrect(Rectangle);
void	combinerect(Rectangle*, Rectangle);
Bitmap*	balloc(Rectangle, int);
void	bfree(Bitmap*);
int	rectclip(Rectangle*, Rectangle);
void	fatal(char*);
void	bitblt(Bitmap*, Point, Bitmap*, Rectangle, Fcode);
void	rectf(Bitmap*, Rectangle, int, Fcode);
void	copymasked(Bitmap*, Point, Bitmap*, Bitmap*, Rectangle);
int	bitbltclip(void*);
Point	string(Bitmap*, Point, Font*, char*, int, Fcode);
void	segment(Bitmap*, Point, Point, int, Fcode);
long	strwidth(Font*, char*);
Point	strsize(Font*, char*);
void	texture(Bitmap*, Rectangle, Bitmap*, Fcode);
void	wrbitmap(Bitmap*, int, int, uchar*);
void	loadbitmap(Bitmap*, int, int, uchar*);
int	ptinrect(Point, Rectangle);
int	rectXrect(Rectangle, Rectangle);
int	eqpt(Point, Point);
int	eqrect(Rectangle, Rectangle);
void	outline(Bitmap*, Rectangle, int, Fcode);
void	border(Bitmap*, Rectangle, int, int);
void	cursorswitch(Cursor*);
void	cursorset(Point);
Rectangle bscreenrect(Rectangle*);
int	clipline(Rectangle, Point*, Point*);
int	clipr(Bitmap*, Rectangle);
int	menuhit(int, Menu*);
ulong	rgbpix(Bitmap*, RGB);
ulong	pixval(ulong, ulong);
void	rdcolmap(Bitmap*, RGB*);
void	wrcolmap(Bitmap*, RGB*);
int	snarfswap(char*, int, char**);
Point	 Pt(int, int);
Rectangle Rect(int, int, int, int);
Rectangle Rpt(Point, Point);

#define	Dx(r)	((r).max.x-(r).min.x)
#define	Dy(r)	((r).max.y-(r).min.y)

extern Bitmap	screen;
extern Font	*font;
extern Font	defont;
extern Point	ZPoint;
extern Rectangle ZRectangle;
extern Rectangle Drect;
extern Bitmap	Jfscreen;
extern Point	Joffset;
extern int	dpyfd;
extern Cursor	normalcursor;
extern Mouse	mouse;
extern JProc	*P;
extern uchar	*muxbuf;

void	initdisplay(int, char**);
Font	getfont(char*);
void	request(int);
char	*gcalloc(ulong, char**);
void	gcfree(char*);
void	handleinput(void);
void	Jscreengrab(void);
void	Jscreenrelease(void);
void	ringbell(void);
void	cursinhibit(void);
void	cursallow(void);
void	setAllHints(int, char**, XSizeHints*);
void	buttons(int);
void	borders(Rectangle, int);
Rectangle getrect(int, int);
Rectangle getrectb(int, int);
int	ngetrect(Rectangle*,Rectangle*,int,int,int,int);
Cursor	*cursswitch(Cursor*);
void	cursset(Point);
int	realtime();
int	wait(int);
void	alarm(int);
void	sleep(int);
void	nap(int);
void	sendnchars(int, uchar*);
void	sendchar(char);
int	rcvchar(void);
int	kbdchar(void);
int	getmuxbuf(void);

void	printgc(char*,GC);

#define	BGSHORT(p)	(((p)[0]<<0) | ((p)[1]<<8))
#define	BGLONG(p)	((BGSHORT(p)<<0) | (BGSHORT(p+2)<<16))
#define	BPSHORT(p, v)	((p)[0]=(v), (p)[1]=((v)>>8))
#define	BPLONG(p, v)	(BPSHORT(p, (v)), BPSHORT(p+2, (v)>>16))

#endif
