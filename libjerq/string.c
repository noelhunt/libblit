#include <blit.h>
#include "libgint.h"
#include <strings.h>

void printgc(char*,GC);

Point string(Bitmap *b, Point p, Font *f, char *s, int v, Fcode c){
	XftColor col;
	XRenderColor rgba = makecol( v );
	if(b->flag&SHIFT)
		p = subpt(p, b->r.min);

	if (!XftColorAllocValue(_dpy, _vis, _cmap, &rgba, &col))
		fprintf(stderr, "cannot allocate xft color\n");
	
	XftDrawString8(b->draw, &col, f, p.x, p.y+f->ascent,
		(const uchar*)s, strlen(s));

	return(addpt(p, Pt(strwidth(f,s),0)));
}

int strwidth(Font *f, char *s){
	XGlyphInfo metrics = { 0 };
	XftTextExtentsUtf8(_dpy, f, (const uchar*)s, strlen(s), &metrics);
	return metrics.width;
}
