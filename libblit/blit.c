#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#define FNDELAY	O_NDELAY
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <blit.h>
#include "libgint.h"

/* NOTE:
 *	if ((fgpix^bgpix)&fgpix) {
 *		F_OR = GXor;
 *		F_CLR = GXandInverted;
 *		F_XOR = GXxor;
 *	} else {
 *		F_OR = GXand;
 *		F_CLR = GXorInverted;
 *		F_XOR = GXequiv;
 *	}
 */

/* Common */
GC		gc;
Display		*_dpy;
Visual		*_vis;
Colormap	_cmap;
Rectangle	Drect;
Bitmap		screen, Jfscreen;
Point		Joffset;
Mouse		mouse;
static JProc	sP;
JProc		*P;
Point		ZPoint = {0,0};
Rectangle	ZRectangle = {0,0,0,0};
Font		defont, *font;
int		rcvmask = 1;
int		dpyfd;
int		curStack = 0;
Cursor		*curSave  = 0;
Cursor		normalcursor;
Cursor		nocursor;
static int	Jlocklevel;
static int	hintwidth, hintheight, hintflags;

ulong	_tmppix;
uchar	*muxbuf;
ulong	_fgpixel, _bgpixel;
XColor	_fgcolor, _bgcolor;
int	_ld2d[6] = { 1, 2, 4, 8, 16, 24 };
ulong	_ld2dmask[6] = { 0x1, 0x3, 0xF, 0xFF, 0xFFFF, 0x00FFFFFF };

static unsigned long inputmask =
	ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|StructureNotifyMask|ExposureMask|KeyPressMask|PointerMotionHintMask;

static Cursor bigarrow = {
	{-7, -7},
	{ 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xf8,
	  0xff, 0xf8, 0xff, 0xf0, 0xff, 0xf8, 0xff, 0xfc,
	  0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
	  0xfb, 0xfc, 0xe1, 0xf8, 0x00, 0xf0, 0x00, 0x60, },
	{ 0x00, 0x00, 0x7f, 0xf8, 0x7f, 0xf0, 0x7f, 0xe0,
	  0x7f, 0xc0, 0x7f, 0xe0, 0x7f, 0xf0, 0x7f, 0xf8,
	  0x7f, 0xfc, 0x7f, 0xfe, 0x77, 0xfe, 0x63, 0xfc,
	  0x41, 0xf8, 0x00, 0xf0, 0x00, 0x60, 0x00, 0x00, }
};

static Cursor arrow = {
	{-7, -7},
	{ 0x00, 0x00, 0x7f, 0xf8, 0x7f, 0xf0, 0x7f, 0xe0,
	  0x7f, 0xc0, 0x7f, 0xe0, 0x7f, 0xf0, 0x7f, 0xf8,
	  0x7f, 0xfc, 0x7f, 0xfe, 0x77, 0xfe, 0x63, 0xfc,
	  0x41, 0xf8, 0x00, 0xf0, 0x00, 0x60, 0x00, 0x00 },
	{ 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xf8,
	  0xff, 0xf8, 0xff, 0xf0, 0xff, 0xf8, 0xff, 0xfc,
	  0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
	  0xfb, 0xfc, 0xe1, 0xf8, 0x00, 0xf0, 0x00, 0x60 }
};

void kbdread(uchar*);
void setcliprect(Rectangle);
void unsetcliprect();
void paste(Window, Atom);

Window root;

void initdisplay(int argc, char *argv[]){
	int i, rootid, defscreen;
	Screen *xscreen;
	XSizeHints sizehints;
	XWMHints xwmhints;
	XWindowAttributes wattr;
	XSetWindowAttributes xswa;
	XColor ccolor, tcolor;
	char *fgcname = 0, *bgcname = 0;
	ulong planes;
	char *stdfont = "pelm.latin1.8";
	char *geom = 0;
	int flags, depth;
	int width, height, x, y;
	char **ap;
	Font *fp;
	Rectangle r;
	int log2(int);

	P = &sP;
	if(!(_dpy= XOpenDisplay(NULL))){
		perror("Cannot open screen\n");
		exit(-1);
	}
#ifdef SYNCH
	XSynchronize(_dpy, True);
#endif
	dpyfd = ConnectionNumber(_dpy);

	rootid = DefaultScreen(_dpy);
	root = DefaultRootWindow(_dpy);
	depth =	DefaultDepth(_dpy, rootid);
	defscreen = DefaultScreen(_dpy);
	xscreen = DefaultScreenOfDisplay(_dpy);
	_cmap = XDefaultColormap(_dpy, DefaultScreen(_dpy));
	_fgpixel = BlackPixel(_dpy, defscreen);
	_bgpixel = WhitePixel(_dpy, defscreen);
	_vis = DefaultVisual(_dpy, DefaultScreen(_dpy));

	ap = argv;
	i = argc;
	memset(&sizehints, 0, sizeof(sizehints));
	while(i-- > 0){
		if( !strcmp("-font", ap[0]) ){
			stdfont = ap[1];
			i--; ap++;
		}else if( !strcmp("-foreground", ap[0]) ||
			 !strcmp("-fg", ap[0]) ){
			fgcname = ap[1];
			i--; ap++;
		}else if( !strcmp("-background", ap[0]) ||
			 !strcmp("-bg", ap[0]) ){
			bgcname = ap[1];
			i--; ap++;
		}else if( !strcmp("-reverse", ap[0]) ||
			 !strcmp("-rv", ap[0]) ){
			_tmppix = _bgpixel;
			_bgpixel = _fgpixel;
			_fgpixel = _tmppix;
		}else if( !strcmp("-geometry", ap[0]) ||
			 !strcmp("-g", ap[0]) ){
			geom = ap[1];	
			flags = XGeometry(_dpy,DefaultScreen(_dpy),geom,0,
					  0,1,1,0,0,&x,&y,&width,&height);
			if(WidthValue & flags){
				sizehints.flags |= USSize;
				sizehints.width = width;
			}
			if(HeightValue & flags){
	    			sizehints.flags |= USSize;
				sizehints.height = height;
			}
			if(XValue & flags){
				sizehints.flags |= USPosition;
				sizehints.x = x;
			}
			if(YValue & flags){
				sizehints.flags |= USPosition;
				sizehints.y = y + 20; /* 20 is for titlebar. */
			}
			i--; ap++;
		}
		ap++;
	}
	if( (fp = XLoadQueryFont(_dpy, stdfont)) )
		defont = *fp;
	else
		defont = getfont("8x13");

	font = &defont;
	sizehints.width_inc = sizehints.height_inc = 1;
	sizehints.min_width = sizehints.min_height = 20;
	sizehints.flags |= PResizeInc|PMinSize;
	if( !(sizehints.flags & USSize) ){
		sizehints.width = defont.max_bounds.width * 80;
		sizehints.height = (defont.max_bounds.ascent +
				 defont.max_bounds.descent) * 24;
		sizehints.flags |= PSize;
		if (hintwidth)
			sizehints.width = hintwidth;
		if (hintheight)
			sizehints.height = hintheight;
		if (hintflags) {
			sizehints.min_width = hintwidth;
			sizehints.min_height = hintheight;
		}
	}
	if (fgcname || bgcname) {
		if (DefaultDepth(_dpy, DefaultScreen(_dpy)) != 1 &&
		    (_vis->class == PseudoColor || _vis->class == GrayScale)) {
			if (!fgcname) fgcname = "black";
			if (!bgcname) bgcname = "white";
			if (!XAllocColorCells(_dpy,_cmap,False,&planes,1,&_bgpixel,1)) {
				perror("Cannot alloc color cells\n");
				exit(1);
			}
			_fgpixel = _bgpixel | planes;
			XStoreNamedColor(_dpy,_cmap,fgcname,_fgpixel,DoRed|DoGreen|DoBlue);
			XStoreNamedColor(_dpy,_cmap,bgcname,_bgpixel,DoRed|DoGreen|DoBlue);
		} else {
			if (fgcname
			&& XAllocNamedColor(_dpy,_cmap,fgcname,&ccolor,&tcolor))
				_fgpixel = ccolor.pixel;
			if (bgcname
			&& XAllocNamedColor(_dpy,_cmap,bgcname,&ccolor,&tcolor))
				_bgpixel = ccolor.pixel;
		}
	}
	r = Rect(0, 0, WidthOfScreen(xscreen)*3/4, HeightOfScreen(xscreen)*3/4);
	if(Dx(r) > Dy(r)*3/2)
		r.max.x = r.min.x + Dy(r)*3/2;
	if(Dy(r) > Dx(r)*3/2)
		r.max.y = r.min.y + Dx(r)*3/2;
	memset(&xswa, 0, sizeof(XSetWindowAttributes));
	xswa.colormap = DefaultColormap(_dpy, defscreen);
	xswa.background_pixel = ~0;
	xswa.border_pixel = 0;
	screen.id = XCreateWindow(
		_dpy,					/* screen */
		RootWindow(_dpy, DefaultScreen(_dpy)),	/* parent */
		r.min.x,				/* x */
		r.min.y,				/* y */
		Dx(r),					/* width */
	 	Dy(r),					/* height */
		0,					/* border width */
		depth,					/* depth */
		InputOutput,				/* class */
		_vis,					/* _vis */
							/* valuemask */
		CWEventMask|CWBackPixel|CWBorderPixel|CWColormap,
		&xswa					/* attributes */
	);
	screen.ldepth = log2(depth);
	screen.flag = SCR;
	if(_fgpixel != 0)
		screen.flag |= BL1;
	if(depth == 1)
		screen.flag |= DP1;
	xwmhints.input = True;
	xwmhints.flags = InputHint;
	XSetWMHints(_dpy, screen.id, &xwmhints);
	_fgcolor.pixel = _fgpixel;
	XQueryColor(_dpy, _cmap, &_fgcolor);
	_bgcolor.pixel = _bgpixel;
	XQueryColor(_dpy, _cmap, &_bgcolor);
	gc = XDefaultGC(_dpy, DefaultScreen(_dpy));
	XSetForeground(_dpy, gc, _fgpixel);
	XSetBackground(_dpy, gc, _bgpixel);
	XSetWindowBackground(_dpy, screen.id, _bgpixel);
	XSelectInput(_dpy, screen.id, inputmask);
	XMapWindow(_dpy, screen.id);
	XSetFont(_dpy, gc, defont.fid);
	screen.r = Drect = r;
	Jfscreen = screen;
	normalcursor = bigarrow;
	cursswitch(&normalcursor);
	unsetcliprect();
	mouse.xy = Pt(0,0);
	mouse.buttons = 0;
	while(!(P->state & RESHAPED)) {
		while (XPending(_dpy))
			handleinput();
	}
}

void request(int what){
	if(what & SEND)
		fcntl(1, F_SETFL, FNDELAY);;
	if(!(what & RCV))
		rcvmask = 0;
}

Font getfont(char *s){
	Font *fp;

	fp = XLoadQueryFont(_dpy, s);
	return fp ? *fp : defont;
}

/* for debugging */
void printgc(char *msg, GC g){
	XGCValues v;
	XGetGCValues(_dpy, g, GCFunction|GCForeground|GCBackground|GCFont|
			GCTile|GCFillStyle|GCStipple, &v);
	fprintf(stderr, "%s: gc %x\n", msg, g);
	fprintf(stderr, "  fg %d bg %d func %d fillstyle %d font %x tile %x stipple %x\n",
		v.foreground, v.background, v.function, v.fill_style,
		v.font, v.tile, v.stipple);
}

void printbm(Bitmap *b){
	char *flags(int);

	fprintf(stderr, "<bitmap> = {\n");
	fprintf(stderr, "    r      = { %d %d %d %d }\n",
		b->r.min.x,b->r.min.y,b->r.max.x,b->r.max.y);
	fprintf(stderr, "    clipr  = { %d %d %d %d }\n",
		b->clipr.min.x,b->clipr.min.y,b->clipr.max.x,b->clipr.max.y);
	fprintf(stderr, "    ldepth = %d\n",b->ldepth);
	fprintf(stderr, "    id     = %d\n",b->id);
	fprintf(stderr, "    cache  = 0x%08x\n",b->cache);
	fprintf(stderr, "    flag   = %s\n",flags(b->flag));
	fprintf(stderr, "}\n");
}

char *flags(int opts){
	char *sep = "";
	static char buf[32];
	buf[0] = '\0';
	if(opts&DP1){
		strcat(buf, "DP1");
		sep = "|";
	}
	if(opts&BL1){
		strcat(buf, sep);
		strcat(buf, "BL1");
		sep = "|";
	}
	if(opts&SCR){
		strcat(buf, sep);
		strcat(buf, "SCR");
		sep = "|";
	}
	if(opts&ZORG){
		strcat(buf, sep);
		strcat(buf, "ZORG");
		sep = "|";
	}
	if(opts&SHIFT){
		strcat(buf, sep);
		strcat(buf, "SHIFT");
		sep = "|";
	}
	if(opts&CLIP){
		strcat(buf, sep);
		strcat(buf, "CLIP");
	}
	return buf;
}

static int log2(int n){
	int i, v;
	for(i=0, v=1; i < 6; i++, v<<=1)
		if(n <= v)
			break;
	return i;
}

void snarf(Window, Atom);

void handleinput (){
	XEvent ev;
	KeySym key;
	unsigned char s[128], *cp;
	int n;
	Window rw, cw;
	int xr, yr, xw, yw;
	unsigned bstate;

	for(;;){
		XNextEvent(_dpy, &ev);
		switch (ev.type) {
		case ButtonPress:
			mouse.buttons |= (8 >> ev.xbutton.button);
			mouse.xy.x = ev.xbutton.x;
			mouse.xy.y = ev.xbutton.y;
			mouse.msec = ev.xbutton.time;
			break;
		case ButtonRelease:
			mouse.buttons &= ~(8 >> ev.xbutton.button);
			mouse.xy.x = ev.xbutton.x;
			mouse.xy.y = ev.xbutton.y;
			mouse.msec = ev.xbutton.time;
			break;
		case MotionNotify:
			XQueryPointer(_dpy, screen.id, &rw, &cw, &xr, &yr, &xw, &yw, &bstate);
			if(button123() && bstate==0)
				continue;
			mouse.xy.x = xw;
			mouse.xy.y = yw;
			break;
		case MapNotify:
		case NoExpose:
			break;
		case ConfigureNotify:
			if (screen.r.max.x != ev.xconfigure.width
			 || screen.r.max.y != ev.xconfigure.height) {
				screen.r.max.x = ev.xconfigure.width;
				screen.r.max.y = ev.xconfigure.height;
				Drect = screen.r;
				Jfscreen = screen;
				unsetcliprect();
			}
			break;
		case Expose:
			while (XCheckTypedEvent(_dpy, Expose, &ev))
				;
			P->state |= RESHAPED;
			break;
		case KeyPress:
			mouse.xy.x = ev.xkey.x;
			mouse.xy.y = ev.xkey.y;
			mouse.msec = ev.xkey.time;
			n = XLookupString(&ev.xkey, (char *)s, sizeof(s), NULL, NULL);
			if(n > 0){
				cp = s;
				P->state |= KBD;
				do {
					kbdread(cp++);
				} while (--n);
			}
			break;
		case SelectionNotify:
			if (ev.xselection.property != None)
				snarf(ev.xselection.requestor,ev.xselection.property);
			P->state |= SELECT;
			break;
		default:
			break;
		}
		return;
	}
}

int getmuxbuf(){
	Window w;

	Atom  prop = XInternAtom(_dpy, "VT_SELECTION", False);
	if( (w = XGetSelectionOwner(_dpy, XA_PRIMARY)) != None ){
		XConvertSelection(_dpy, XA_PRIMARY, XA_STRING, prop,
			(Drawable)screen.id, CurrentTime);
		wait(SELECT);
		P->state &= ~SELECT;
		return 1;
	}
	return 0;
}

void snarf(Window w, Atom prop){
	int fmt;
	Atom type;
	ulong length, dummy;
	uchar *data;

	if( XGetWindowProperty(_dpy, (Drawable)screen.id, prop, 0,
	    SNARF/sizeof(ulong), 0, AnyPropertyType, &type, &fmt, &length, &dummy, &data) == Success ){
		if(type == XA_STRING || length > 0)
			muxbuf = (uchar*)strdup((char*)data);
		if(data) XFree(data);
	}
}

void buttons(int updown){
	while((button123()!=0) != updown)
		nap(2);
}

/*
 * GRABMASK stolen from PointerGrabMask in X11R4/server/dix/events.c
 * otherwise X11R4 rejects the XGrabPointer
 */
#define GRABMASK (ButtonPressMask | ButtonReleaseMask | \
	EnterWindowMask | LeaveWindowMask |  PointerMotionHintMask \
	| KeymapStateMask | PointerMotionMask | Button1MotionMask | \
        Button2MotionMask | Button3MotionMask | ButtonMotionMask )

void Jscreengrab(){
	if (!Jlocklevel)
		XSetSubwindowMode(_dpy, gc, IncludeInferiors);
	Jlocklevel++;
}

void Jscreenrelease(){
	if (--Jlocklevel <= 0) {
		XSetSubwindowMode(_dpy, gc, ClipByChildren);
		Jlocklevel = 0;
	}
}

char *gcalloc(unsigned long nbytes, char **where){
	*where=(char *)malloc(nbytes);
	return *where;
}

void gcfree(char *s){
	if (s != 0)
		free(s);
}

void cursinhibit(){
	if( curStack == 0 )
		curSave = cursswitch( &nocursor );
	curStack++;
}

void cursallow(){
	curStack--;
	if( curStack <= 0 ) {
		cursswitch( curSave );
		curStack = 0;
	}
}

void ringbell(){ XBell(_dpy,50); }

void setcliprect(Rectangle r){
	XRectangle xr;

	xr.x = r.min.x;
	xr.y = r.min.y;
	xr.width = r.max.x - r.min.x;
	xr.height = r.max.y - r.min.y;
	XSetClipRectangles(_dpy, gc, r.min.x, r.min.y, &xr, 1, Unsorted);
}

void unsetcliprect(){
	setcliprect(Drect);
}

ulong pixval(ulong rgb, ulong def){
	XColor color;

	color.blue  = (rgb & 0x0000FF) << 8;
	color.green = (rgb & 0x00FF00);
	color.red   = (rgb & 0xFF0000) >> 8;
	color.flags = DoRed|DoGreen|DoBlue;

	if(XAllocColor(_dpy, _cmap, &color))
		return color.pixel;
	return def;
}

void fatal(char *s){
	fprintf(stderr, "fatal: %s\n", s);
	exit(1);
}
