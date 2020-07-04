/*
	test circle graphics routines: circle ellipse, arc, elarc, disc,
		eldisc, and also string, segment, and cursswitch
*/
#include "blit.h"

#include "lattice.xbm"

Cursor Coffee = {
	{-7,-7},
	{0x03, 0xe0, 0x01, 0xf0, 0x03, 0xf8, 0x07, 0xf0,
	 0x0f, 0xf0, 0x1f, 0xfc, 0x3f, 0xfe, 0x3f, 0xff,
	 0x3f, 0xff, 0x38, 0x3f, 0x38, 0x3f, 0x38, 0x3e,
	 0x7f, 0xfc, 0xff, 0xfe, 0xff, 0xfe, 0x7f, 0xfc},
	{0x01, 0x00, 0x00, 0xe0, 0x00, 0x10, 0x03, 0xe0,
	 0x04, 0x00, 0x0f, 0xe0, 0x12, 0x3c, 0x1f, 0xe2,
	 0x10, 0x1a, 0x10, 0x1a, 0x10, 0x02, 0x10, 0x3c,
	 0x18, 0x10, 0x6f, 0xec, 0x40, 0x04, 0x3f, 0xf8}
};

uchar arrowbits[] = {
	0x00, 0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70,
	0x1f, 0xf8, 0x1f, 0xfc, 0x1f, 0xfe, 0x1f, 0xfe,
	0x1f, 0xfc, 0x1f, 0xf8, 0x00, 0x70, 0x00, 0x60,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void main(int argc, char **argv){
	int i, x,y;
	Point p;
	Bitmap *b, *b2, *arrow, *lattice;
	Bitmap *magenta, *orange, *box;
	Rectangle r;
	Cursor *prev = 0;

	request(MOUSE);
	initdisplay(argc, argv);
 	x = Drect.min.x + Drect.max.x / 2;
	y = Drect.min.y + Drect.max.y / 2;
	segment(&screen, Pt(x-100,y-100), Pt(x+200,y+200), ~0, D^S);
	p = string(&screen, Pt(x+20,y), font, "hello world", ~0, D^S);
	p = string(&screen, addpt(p, Pt(10,0)), font, "Goodbye world", ~0, D^S);

	b = balloc(Rect(0,0,2,2), screen.ldepth);
	rectf(b, b->r, pixval(0x99004C, 0), S);

	b2 = balloc(Rect(0,0,2,2), screen.ldepth);
	rectf(b2, b2->r, pixval(0xFFAAFF, 0), S);

	r = Rect(100,300,300,500);
	rectf(&screen, r, pixval(0xEEEEEE, 0), S);

	border(&screen, r, -6, pixval(0x99004C, 0)); sleep(45);
	for(i=0; i<3; i++){
		border(&screen, r, -3, pixval(0xFFAAFF, 0)); sleep(30);
		border(&screen, r, -6, pixval(0x99004C, 0)); sleep(30);
	}

	arrow = balloc(Rect(0,0,16,16), 0);
	loadbitmap(arrow, 0, 16, arrowbits);
	magenta = balloc(Rect(0,0,16,16), screen.ldepth);
	rectf(magenta, magenta->r, pixval(DMagenta,0), S);

	copymasked(&screen, Pt(25,25), magenta, arrow, magenta->r);

	lattice = balloc(Rect(0,0,136,136), 0);
	loadbitmap(lattice, lattice->r.min.y, lattice->r.max.y, Latticebits);
	orange = balloc(Rect(0,0,136,136), screen.ldepth);
	rectf(orange, orange->r, pixval(0xFFAA00,0), S);

	copymasked(&screen, Pt(50,50), orange, lattice, orange->r);

	box = balloc(Rect(0,0,128,16), screen.ldepth);
	rectf(box, box->r, pixval(DYellow,0), S);
	bitblt(&screen, Pt(600,500), box, box->r, S);
	sleep(45);
	copymasked(&screen, Pt(600,500), magenta, arrow, magenta->r);

	for(;;nap(1)){
		if(button1())
			cursswitch(&Coffee);
		if(button2())
			cursswitch(0);
		if(button3())
			break;
	}
}
