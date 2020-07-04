#include <blit.h>
static char *editstrs[]={
	"cut",
	"paste",
	"snarf",
	"send",
	"cut",
	"paste",
	"snarf",
	"send",
	"cut",
	"paste",
	"snarf",
	"send",
	"paste",
	"snarf",
	"send",
	"cut",
	"paste",
	"snarf",
	"send",
	0,
};
static	Menu	editmenu = {editstrs};

void main(int argc, char **argv){
	int i, n, h, w, y;
	char *m3gen(int);
	static Menu menu3 = { (char **) 0, m3gen, 0 };
	static char *s = "Hello world";
	char c[2];

	request(KBD|MOUSE);
	initdisplay(argc, argv);

	c[1] = 0;
	h = fontheight(font);
	y = h + 5;
	w = strwidth(font, "m");
	for(i = 0; ; ){
		wait(MOUSE|KBD);
		if(P->state & KBD){
			c[0] = kbdchar();
			string(&screen, Pt(10+w*i, y), font, c, ~0, D^S);
			i++;
			if(i > 50){
				i = 0;
				y += h;
			}
		}
		if(button2())
			n = menuhit(2, &editmenu);
		else if(button3())
			if((n = menuhit(3, &menu3)) == 0)
				break;
	}
}

char *m3gen(int n){
	static char *m2[] = { "quit", "thing1", "thing2" };
	static char *m3[] = {
		"quit", "gods", "men", "art", "philosophy",
		"scholastics", "aquinas", "plato", "socrates",
		"aeschylos", "pindar", "callimachos", "scholastics",
		"aquinas", "plato", "socrates", "aeschylos",
		"pindar", "callimachos", "scholastics", "aquinas",
		"plato", "socrates", "aeschylos", "pindar",
		"callimachos", "gods", "men", "art",
	};
	if (n < 0 || n > 29)
		return 0;
	else 
		return m3[n];
}
