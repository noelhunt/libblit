/* Copyright (c) 1992 AT&T - All rights reserved. */
#include <blit.h>
#include "libgint.h"
#include <stdio.h>

enum {
	UP,
	DOWN,
	Borderwidth=4
};

static Bitmap *tmp[4];
static Bitmap *sub[2];

static Cursor sweep={
	{-7, -7},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x07,
	 0xE0, 0x07, 0xE0, 0x07, 0xE3, 0xF7, 0xE3, 0xF7,
	 0xE3, 0xE7, 0xE3, 0xF7, 0xE3, 0xFF, 0xE3, 0x7F,
	 0xE0, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,},
	{0x00, 0x00, 0x7F, 0xFE, 0x40, 0x02, 0x40, 0x02,
	 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x41, 0xE2,
	 0x41, 0xC2, 0x41, 0xE2, 0x41, 0x72, 0x40, 0x38,
	 0x40, 0x1C, 0x40, 0x0E, 0x7F, 0xE6, 0x00, 0x00,}
};

static void grabcursor(void){
	/* Grab X server with an limp wrist. */
	while (XGrabPointer(_dpy, screen.id, False,
			ButtonPressMask|ButtonReleaseMask|
			ButtonMotionMask|StructureNotifyMask,
		GrabModeAsync, GrabModeAsync, None, None, CurrentTime)
			!= GrabSuccess)
		sleep(2);
	/* Grab the keyboard too */
	XSetInputFocus(_dpy, screen.id, RevertToParent, CurrentTime);
}

static void ungrabcursor(void){
	XUngrabPointer(_dpy, CurrentTime);
}

Rectangle getrect(int but, int block){
	Rectangle r;
	Point p1, p2;
	Rectangle canon(Point, Point);

	cursswitch(&sweep);
	if(block){
		buttons(UP);
		buttons(DOWN);
	}
	grabcursor();
	if(!(mouse.buttons&but)){
		r.min.x=r.min.y=r.max.x=r.max.y=0;
		buttons(UP);
		goto Return;
	}
	p1 = addpt(mouse.xy, Joffset);
	p2 = p1;
	r = canon(p1, p2);
	for( ; mouse.buttons&but; nap(2)){
		border( &screen, r, 2, F&~D );
		p2 = addpt(mouse.xy, Joffset);
		r = canon(p1, p2);
		border( &screen, r, 2, F&~D );
	};
Return:
	ungrabcursor();
	cursswitch((Cursor *)0);
	return r;
}

/*
 *	ngetrect:	ultimate getrect routine
 *			optional clipping rectangle
 *			optional blocking for mouse routines
 *			optional minimum width and height
 *			returns 1 if minimum rectangle is swept
 */

int ngetrect(Rectangle *r, Rectangle *clip, int but, int block, int minw, int minh){
	if(but == 0)
		but = 1;
	*r = getrect(8>>but, block);
	if(eqrect(*r, ZRectangle))
		return 0;
	if(r->max.x - r->min.x < minw ||
	   r->max.y - r->min.y < minh)
		return 0;
	if(clip && !rectclip(r, *clip))
		return 0;
	return 1;
}

Rectangle canon(Point p1, Point p2){
	Rectangle r;
	r.min.x = min(p1.x, p2.x);
	r.min.y = min(p1.y, p2.y);
	r.max.x = max(p1.x, p2.x);
	r.max.y = max(p1.y, p2.y);
	return(r);
}

#define	W	Borderwidth

static void brects(Rectangle r, Rectangle rp[4]){
	if(Dx(r) < 2*W) r.max.x = r.min.x+2*W;
	if(Dy(r) < 2*W) r.max.y = r.min.y+2*W;
	rp[0] = Rect(r.min.x, r.min.y, r.max.x, r.min.y+W);
	rp[1] = Rect(r.min.x, r.max.y-W, r.max.x, r.max.y);
	rp[2] = Rect(r.min.x, r.min.y+W, r.min.x+W, r.max.y-W);
	rp[3] = Rect(r.max.x-W, r.min.y+W, r.max.x, r.max.y-W);
}

void borders(Rectangle rc, int draw){
	int i;
	ulong red = pixval(0xFF0000,0);
	Rectangle r, brs[4];

	if(tmp[0] == 0){
		r = Rect(0, 0, Dx(screen.r), W);
		tmp[0] = balloc(r, screen.ldepth);
		tmp[1] = balloc(r, screen.ldepth);
		sub[0] = balloc(r, screen.ldepth);
		rectf(sub[0], sub[0]->r, red, S);
		r = Rect(0, 0, W, Dy(screen.r));
		tmp[2] = balloc(r, screen.ldepth);
		tmp[3] = balloc(r, screen.ldepth);
		sub[1] = balloc(r, screen.ldepth);
		rectf(sub[1], sub[1]->r, red, S);
	}
	brects(rc, brs);
	if(!draw){
		for(i=0; i<4; i++)
			bitblt(&screen, brs[i].min, tmp[i], Rect(0,0,Dx(brs[i]),Dy(brs[i])), S);
		return;
	}
	for(i=0; i<4; i++){
		bitblt(tmp[i], ZPoint, &screen, brs[i], S);
		bitblt(&screen, brs[i].min, sub[i/2], Rect(0,0,Dx(brs[i]),Dy(brs[i])), S);
	}
}

Rectangle getrectb(int but, int block){
	Rectangle r;
	Point p1, p2;
	Rectangle canon(Point, Point);

	cursswitch(&sweep);
	if(block){
		buttons(UP);
		buttons(DOWN);
	}
	grabcursor();
	if(!(mouse.buttons&but)){
		r.min.x=r.min.y=r.max.x=r.max.y=0;
		buttons(UP);
		goto Return;
	}
	p1 = p2 = mouse.xy;
	do{
		r = canon(p1, p2);
		borders( r, 1 );
		nap(2);
		borders( r, 0 );
		p2 = mouse.xy;
	}while( mouse.buttons&but );
Return:
	ungrabcursor();
	cursswitch((Cursor *)0);
	return r;
}
