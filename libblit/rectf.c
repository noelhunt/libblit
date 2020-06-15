#include <blit.h>
#include "libgint.h"

void rectf(Bitmap *d, Rectangle r, int v, Fcode f){
	int x, y;
	GC g;

	x = r.min.x;
	y = r.min.y;
	if (d->flag&SHIFT){
		x -= d->r.min.x;
		y -= d->r.min.y;
	}
	g = _getfillgc(f, d, v);
	if(d->flag&SHIFT){
		XSetTSOrigin(_dpy, g, -d->r.min.x, -d->r.min.y);
	}else
		XSetTSOrigin(_dpy, g, 0, 0);
	XFillRectangle(_dpy, (Drawable)d->id, g, x, y, Dx(r), Dy(r));
}
