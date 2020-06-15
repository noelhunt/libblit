/* Copyright (c) 1992 AT&T - All rights reserved. */
#include <blit.h>
#include "libgint.h"

/*
 * Use  the  id field in Cursor to hold the X id corresponding to the
 * cursor, so that it doesn't have to be recreated  on  each  cursor-
 * switch.   This  doesn't  quite  match the semantics of Plan9 libg,
 * since the user could  create  a  cursor  (say  with  malloc)  with
 * garbage  in the id field; or the user could change the contents of
 * the other fields and we wouldn't know about it.  Neither of  these
 * happen in existing uses of libg.
 */

static Bitmap *fore, *mask;
extern Cursor normalcursor;

Cursor *cursswitch(Cursor *c){
	if(c == 0)
		c = &normalcursor;
	if(c->id == 0){
		if(fore == 0){
			fore = balloc(Rect(0,0,16,16), 0);
			mask = balloc(Rect(0,0,16,16), 0);
		}
		/*
		 * Cursor should have fg where "set" is 1,
		 * and bg where "clr" is 1 and "set" is 0,
		 * and should leave places alone where "set" and "clr" are both 0
		 */
		wrbitmap(fore, 0, 16, c->set);
		wrbitmap(mask, 0, 16, c->clr);
		bitblt(mask, Pt(0,0), fore, fore->r, S|D);
		c->id = (int) XCreatePixmapCursor(_dpy, (Pixmap)fore->id, (Pixmap)mask->id,
			&_fgcolor, &_bgcolor, -c->offset.x, -c->offset.y);
	}
	XDefineCursor(_dpy, (Window)screen.id, c->id);
	return c;
}
