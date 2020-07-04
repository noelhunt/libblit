/* Copyright (c) 1992 AT&T - All rights reserved. */
#include <blit.h>
#include "libgint.h"

/*
 * Only allow cursor to move within screen Bitmap
 */
void cursset(Point p){
	/* motion will be relative to window origin */
	p = subpt(p, screen.r.min);
	XWarpPointer(_dpy, None, (Window)screen.id, 0, 0, 0, 0, p.x, p.y);
	mouse.xy.x = p.x;
	mouse.xy.y = p.y;
}
