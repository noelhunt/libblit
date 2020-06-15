/*
	t0:	bitblt graphics tests
*/
#include <stdio.h>
#include <string.h>

#include "blit.h"
#include "libgint.h"

#define ESCAPE 033
#define CNTRL_U 025
#define KBDLEN 90

char KBDStr[KBDLEN]=  "\1";

uchar coffeeset[]={
	0x01, 0x00, 0x00, 0xE0, 0x00, 0x10, 0x03, 0xE0,
	0x04, 0x00, 0x0F, 0xE0, 0x12, 0x3C, 0x1F, 0xE2,
	0x10, 0x1A, 0x10, 0x1A, 0x10, 0x02, 0x10, 0x3C,
	0x18, 0x10, 0x6F, 0xEC, 0x40, 0x04, 0x3F, 0xF8,
};

uchar arrowset[] = {
	0x00, 0x00, 0x7F, 0xC0, 0x7F, 0x00, 0x7C, 0x00,
	0x7E, 0x00, 0x7F, 0x00, 0x6F, 0x80, 0x67, 0xC0,
	0x43, 0xE0, 0x41, 0xF0, 0x00, 0xF8, 0x00, 0x7C,
	0x00, 0x3E, 0x00, 0x1C, 0x00, 0x08, 0x00, 0x00
};

uchar arrowdata[] = {
	0x00, 0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70,
	0x1f, 0xf8, 0x1f, 0xfc, 0x1f, 0xfe, 0x1f, 0xfe,
	0x1f, 0xfc, 0x1f, 0xf8, 0x00, 0x70, 0x00, 0x60,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uchar oneset[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

uchar Darkgrey[] ={
	0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
	0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
	0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
	0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
};

void snooze();

Rectangle KBDrect;
void PaintKBD();
void KBDAppend(int);
void MuxSnarf(void);

void main(int argc, char **argv){
	int i = 0, h, w, y;
	char c;
	Rectangle r;
	Bitmap *b64, *bmap, *bm, *b16, *arrow, *box;
	Bitmap *darkgrey, *temp;
	Point p1, pt;
	ulong pixel;

	request(KBD|MOUSE|SELECT);
	initdisplay(argc, argv);

	extern ulong _fgpixel, _bgpixel;

	h = fontheight(&defont);
	y = h + 35;
	w = strwidth(&defont, "m");

	KBDrect = Jfscreen.r;
	KBDrect.min.y = KBDrect.max.y - (fontheight(&defont)+4);
	KBDrect.min.x += 1;
	KBDrect.max.x -= 1;

	outline(&screen, KBDrect, 1, F);

	darkgrey = balloc(Rect(0,0,16,16), 0);
	wrbitmap(darkgrey, 0, 16, Darkgrey);
	bm = balloc(Rect(0,0,256,128), 0);
	texture(bm, bm->r, darkgrey, S);
	rectf(&screen, rectaddpt(bm->r,Pt(400,50)), ~0, S);
	bitblt(&screen, Pt(450,100), bm, bm->r, D^S);

	r = Rect(0,0,128,128);
	bmap = balloc(r, screen.ldepth);
	pixel = pixval(0xFF00FF, 0);
	rectf(bmap, r, pixel, S);
	bitblt(&screen, Pt(400,400), bmap, bmap->r, S);

	temp = balloc(Rect(0,0,75,75), screen.ldepth);
	pixel = pixval(0xFF0000, 0);
	rectf(temp, temp->r, pixel, S);
	bitblt(&screen, Pt(100,400), temp, temp->r, S);
	
	b16 = balloc(Rect(0,0,16,16), 0);
	wrbitmap(b16, 0, 16, coffeeset);

	texture(&screen, rectaddpt(bmap->r, Pt(500,300)), b16, S|D);

	b16 = balloc(Rect(0,0,16,16), 0);
	wrbitmap(b16, 0, 16, coffeeset);
	b64 = balloc(Rect(0,0,64,64), 0);
	texture(b64, Rect(0,0,64,64), b16, S);

	r = Rect(700,400,850,418);

	outline(&screen, r, -2, F);				snooze();

	box = balloc(rectsubpt(r,r.min), screen.ldepth);
	rectf(box, box->r, pixval(0xBFFFBF, 0), S);
	pt = string(box, Pt(1,1), font, "Menu...Entry", ~0, S);	snooze();

	arrow = balloc(Rect(0,0,16,16), 0);
	loadbitmap(arrow, 0, 16, arrowdata);

	bitblt(box, addpt(pt, Pt(4,0)), arrow, arrow->r, D|S);	snooze();

	bitblt(&screen, r.min, box, box->r, S);

	p1 = Pt(300,300);
	rectf(&screen, Rpt(p1, Pt(450, 320)), pixval(DDarkgreen, 0), S);
	bitblt(&screen, p1, arrow, arrow->r, D&~S);

			/* add some stripes		*/
	bmap = balloc(Rect(0,0,16,16), 0);
	wrbitmap(bmap, 0, 16, oneset);

	/* rectf(b64, Rect(0,0,32,64), F_XOR); */
	texture(b64, Rect(0,0,32,64), bmap, D^S);
	/* rectf(&screen, Rect(0,0,256,32), F_STORE); */
	texture(&screen, Rect(0,0,256,32), bmap, S);
	/* rectf(&screen, Rect(0,64,256,96), F_STORE); */
	texture(&screen, Rect(0,64,256,96), bmap, S);

			/* bitblt pr to pw		*/
	bitblt(&screen, Pt(0,0), b64, b64->r, S);
	bitblt(&screen, Pt(0,64), b64, b64->r, D^S);
	bitblt(&screen, Pt(64,0), b64, b64->r, D&~S);
	bitblt(&screen, Pt(64,64), b64, b64->r, D|S);

			/* screenswap	*/
//	screenswap(&screen, Rect(0,0,128,128), Rect(0,256,128,384));

	for(;;){
		char *t;
		int len;

		wait(MOUSE|KBD);
		if(own() & KBD){
			c = kbdchar();
			if( c == ESCAPE )
				MuxSnarf();
			if( c == '\r' )
				break;
			len = strlen(t = KBDStr);
			if( c == '\b' && len > 1 ){
				t[len-2] = 001;
				t[len-1] = 000;
				continue;
			}
			KBDAppend(c);
			PaintKBD();
		} else if(button123())
			break;
	}
}

void KBDAppend(int c){
	char *t;
	int len = strlen(t = KBDStr);

	if( c < 040 || (c&0200) || len >= KBDLEN ){
		if( c == CNTRL_U ){
			t[0] = 001;
			t[1] = 000;
		}
		if( c != '\t' )		/* bug: \t when len >= KBDLEN !! */
			return;
	}
	t[len-1] = c;
	t[len] = 001;
	t[len+1] = 000;
}

void PaintKBD(){
	rectf( &screen, insetrect(KBDrect, 2), 0, S );
	string(&screen, addpt(KBDrect.min,Pt(1,2)), &defont, KBDStr, ~0, F&~D);
}	

void MuxSnarf(){
	char *cp;
	if(getmuxbuf())
		for( cp = (char*)muxbuf; *cp; ++cp ){
			if( *cp == '\n' ) break;
			KBDAppend(*cp);
		}
}

void snooze(){ sleep(30); }
