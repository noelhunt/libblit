/*
	test for getrect, ngetrect, texture, mouse input, rectf
*/
#include "blit.h"
uchar Darkgrey[] ={
	0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
	0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
	0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
	0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
};

void main(int argc, char **argv){
	Bitmap *darkgrey;
	Rectangle r;

	request(KBD|MOUSE|ALARM);
	initdisplay(argc, argv);

	darkgrey = balloc(Rect(0,0,16,16), 0);
	wrbitmap(darkgrey, 0, 16, Darkgrey);
	alarm(60);
	for( ; ; wait(MOUSE|ALARM)) {
		if(button1()){
			ngetrect(&r, 0, 1, 0, 10, 10);
			rectf(&screen, r, ~0, D^S);
		}else if(button2()){
			ngetrect(&r, 0, 2, 0, 10, 10);
			texture(&screen, r, darkgrey, D^S);
		}else if(button3())
			break;
		if (own() & ALARM) {
			rectf(&screen, Rect(0, 0, 100, 100), ~0, D^S);
			rectf(&screen,
			Rect(Drect.max.x-100, Drect.max.y-100, Drect.max.x, Drect.max.y), ~0, D^S);
			alarm(60);
		}
			
	}
}
