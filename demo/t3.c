/*
	sliderbar. tests mouse input and rectf
*/
#include "blit.h"

#define COLOR

void main(int argc, char **argv){
	static int oldx;
	Bitmap *bmap;
	ulong pixel;
	extern ulong _bgpixel;

	request(KBD|MOUSE);
	initdisplay(argc, argv);
	pixel = pixval(0x00FF00, 0);
	for(;;){
		nap(1);
		if(button1()){
			if(oldx == mouse.xy.x)
				continue;
			if(oldx < mouse.xy.x){
#ifndef COLOR
				rectf(&screen, Rect(oldx, Drect.min.y,
					mouse.xy.x, Drect.max.y), ~0, D^S);
#else
				rectf(&screen, Rect(oldx, Drect.min.y,
					mouse.xy.x, Drect.max.y), pixel, S);
#endif
			}else{
#ifndef COLOR
				rectf(&screen, Rect(mouse.xy.x,
					Drect.min.y, oldx, Drect.max.y), ~0, D^S);
#else
				rectf(&screen, Rect(mouse.xy.x,
					Drect.min.y, oldx, Drect.max.y), _bgpixel, S);
#endif
			}
			oldx = mouse.xy.x;
		}else if(button23()) {
			break;
		}
	}
}

