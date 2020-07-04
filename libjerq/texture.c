/* Copyright (c) 1992 AT&T - All rights reserved. */
#include <blit.h>
#include "libgint.h"

void texture(Bitmap *d, Rectangle r, Bitmap *s, Fcode f){
	int x, y, bfunc;
	GC g;

	x = r.min.x;
	y = r.min.y;
	if(d->flag&SHIFT){
		x -= d->r.min.x;
		y -= d->r.min.y;
	}
	g = _getcopygc(f, d, s, &bfunc);
	XSetGraphicsExposures(_dpy, g, False);
	if(d->flag&SHIFT)
		XSetTSOrigin(_dpy, g, -d->r.min.x, -d->r.min.y);
	else
		XSetTSOrigin(_dpy, g, 0, 0);
	if(bfunc == UseFillRectangle){
		/* source isn't involved at all */
		XFillRectangle(_dpy, (Drawable)d->id, g, x, y, Dx(r), Dy(r));
	}else if(bfunc == UseCopyArea){
		XSetTile(_dpy, g, (Drawable)s->id);
		XSetFillStyle(_dpy, g, FillTiled);
		XFillRectangle(_dpy, (Drawable)d->id, g, x, y, Dx(r), Dy(r));
		XSetFillStyle(_dpy, g, FillSolid);
	}else{
		/* bfunc == UseCopyPlane */
		if(s->ldepth != 0)
			fatal("unsupported texture");
		XSetStipple(_dpy, g, (Drawable)s->id);
		XSetFillStyle(_dpy, g, FillOpaqueStippled);
		XFillRectangle(_dpy, (Drawable)d->id, g, x, y, Dx(r), Dy(r));
		XSetFillStyle(_dpy, g, FillSolid);
	}
}
