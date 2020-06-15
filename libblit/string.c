#include <blit.h>
#include "libgint.h"
#include <strings.h>

void printgc(char*,GC);

Point string(Bitmap *b, Point p, Font *f, char *s, int v, Fcode c){
	GC g;
	if(b->flag&SHIFT)
		p = subpt(p, b->r.min);
	g = _getfillgc(c, b, v);
	XSetFont(_dpy, g, f->fid);
	/* libXdmd uses XDrawImageString here */
	XDrawString(_dpy, b->id, g, p.x, p.y+f->max_bounds.ascent, s, strlen(s));
	return(addpt(p, Pt(strwidth(f,s),0)));
}

long strwidth(Font *f, char *s){
	return XTextWidth(f, s, strlen(s));
}
