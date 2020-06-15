/*
	hierarchical menu test
dfn() [f1] gets called when moving to a pop-aside menu
		also gets called when moving off right edge of any
		non-heirarchical menu!--f3 gets called immediately after!
bfn() [f2] gets called when moving back to a lower menu
		also gets called after f3 -- once per pop-aside menu
hfn() [f3] gets called when item is selected
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<blit.h>
#include	<menu.h>

#define DEFHEIGHT 14
#define CLEARLN	{ \
	rectf(&screen,Rpt(addpt(Drect.min,Pt(5,0)),Pt(Drect.max.x,Drect.min.y+DEFHEIGHT)),~0,D&~S);	\
}

#define PRINT(x) {					\
	CLEARLN;					\
	string(&screen, addpt(Drect.min,Pt(5,0)), &defont, (x), ~0, S);	\
}
				
void quit(), f1(), f2(), f3();

static NMenu m1, m2, m3, m4, m5;
NMitem item1[] = {
		"gods",		0,	0,f1,f2,f3,0,
		"men",		0,	0,f1,f2,f3,1,
		"art",		0,	0,f1,f2,f3,2,
		"philosophy",	0,	0,f1,f2,f3,3,
		0
};
NMitem item2[] = {
		"add all",	"Add all",		0,f1,f2,f3,4,
		"free all",	"Release all",		&m2,f1,f2,f3,5,
		"move",		"Displace",		0,f1,f2,f3,6,
		"top",		"Ascend",		0,f1,f2,f3,7,
		"bottom",	"Descend",		0,f1,f2,f3,8,
		"mark/unmark",	"Smudge",		&m1,f1,f2,f3,9,
		"free",		"Free one",		0,f1,f2,f3,10,
		"flowers",	"Common/Botanical",	&m5,f1,f2,f3,11,
		0
};
NMitem item3[] ={
		"save",	"Preserve",	0,f1,f2,f3,11,
		"read",	"Incorporate",	&m4,f1,f2,f3,12,
		"quit",	"Exit",		0,f1,f2,f3,13,
		0
};
NMitem item5[] = {
	"Achillea",            "Achillea",			0,f1,f2,f3,101,
	"African Daisy",       "Dimorphotheca Aurantica",	0,f1,f2,f3,102,
	"Alternanthera",       "Alternanthera",			0,f1,f2,f3,103,
	"Alyssum",             "Lobularia Maritima",		0,f1,f2,f3,104,
	"Amaranth",            "Amaranthaceae",			0,f1,f2,f3,105,
	"Aster, Crego Mix",    "Callistephus",			0,f1,f2,f3,106,
	"Bachelor Buttons",    "Centaurea Cyanus",		0,f1,f2,f3,107,
	"Balloon Flower",      "Platycodon Grandiflorus",	0,f1,f2,f3,108,
	"Balsam",              "Impatiens Balsamina",		0,f1,f2,f3,109,
	"Bee Balm",            "Monarda Didyma",		0,f1,f2,f3,110,
	"Begonia",             "Begoniaceae",			0,f1,f2,f3,111,
	"Bellflower",          "Camponula",			0,f1,f2,f3,112,
	"Birdhouse Gourds",    "Cucurbita Lagenaria",		0,f1,f2,f3,113,
	"Black Eyed Susan",    "Rudbeckia Hirta",		0,f1,f2,f3,114,
	"Blanketflower",       "Gaillardia Aristata",		0,f1,f2,f3,115,
	"Blazing Star",        "Liatris Spicata",		0,f1,f2,f3,116,
	"Bleeding Heart",      "Dicenta Spectabilis",		0,f1,f2,f3,117,
	"Butterfly Flower",    "Asciepias Tuberosa",		0,f1,f2,f3,118,
	"Cactus",              "Cacti",				0,f1,f2,f3,119,
	"Calendula",           "Calendula",			0,f1,f2,f3,120,
	"California Poppy",    "Eschschoizia Californica",	0,f1,f2,f3,121,
	"Canary Climber",      "Tropaeolum Peregrinum",		0,f1,f2,f3,122,
	"Candytuft",           "Iberis",			0,f1,f2,f3,123,
	"Canterbury Bell Cup", "Campenula Medium",		0,f1,f2,f3,124,
	"Cardinal Climber",    "Ipomoea Cardinalis",		0,f1,f2,f3,125,
	"Carnation",           "Dianthus",			0,f1,f2,f3,126,
	"Cathedral Bells",     "Cobaea Scandens",		0,f1,f2,f3,127,
	"Celosia",             "Celosia",			0,f1,f2,f3,128,
	"Chinese Lantern",     "Physalis Franchetti",		0,f1,f2,f3,129,
	"Cleome",              "Cleome Hasslerana",		0,f1,f2,f3,130,
	"Clematis",            "Jackmanii",			0,f1,f2,f3,131,
	"Climbing Spinach",    "Basella Rubra",			0,f1,f2,f3,132,
	"Cockscomb",           "Celosia Cristata",		0,f1,f2,f3,133,
	"Coleus",              "Coleus",			0,f1,f2,f3,134,
	"Columbine",           "Aquilegia",			0,f1,f2,f3,135,
	0
};
#ifdef old
NMitem item1[] = {
		"gods",	"",		0,0,0,0,0,
		"men",	"",		0,0,0,0,1,
		"art",	"",		0,0,0,0,2,
		"philosophy","",	0,0,0,0,3,
		0
};
NMitem item2[] = {
		"add all","",		0,0,0,0,4,
		"free all","",		&m2,0,0,0,5,
		"move","",		0,0,0,0,6,
		"top","",		0,0,0,0,7,
		"bottom","",		0,0,0,0,8,
		"mark/unmark","",	&m1,0,0,0,9,
		"free","",		0,0,0,0,10,
		0
};
NMitem item3[] ={
		"save","",		0,0,0,0,11,
		"read","",		&m4,0,0,0,12,
		"quit","",		0,0,0,0,13,
		0
};
#endif
int a = 0;
int b = 0;
int c = 0;
char str[50];
char *s = str;

NMitem *genmenu(int n){
	static NMitem menuitem[4];

	menuitem[0].text = "zero";
	menuitem[1].text = "one";
	menuitem[2].text = "two";
	menuitem[3].text = 0;
	menuitem[0].data = 0;
	menuitem[1].data = 1;
	menuitem[2].data = 2;
	menuitem[3].data = 0;

	if(n > 2)
		return &menuitem[3];
	return &menuitem[n];
}

NMitem *nmenuhit(NMenu*, int, int);

void main(int argc, char **argv){
	NMitem *genmenu();
	NMitem *temp;

	request(MOUSE);
	initdisplay(argc, argv);
	m1.item = item1;
	m2.item = item2;
	m3.item = item3;
	m4.item = 0;
	m4.generator = genmenu;
	m5.item = item5;
	for(;;){
		nap(1);
		if(button1() && (temp = nmenuhit(&m1,1,0))){
			switch(temp->data){
			case 0:
				PRINT("m1, case 0");
				break;
			case 1:
				PRINT("m1, case 1");
				break;
			case 2:
				PRINT("m1, case 2");
				break;
			case 3:
				PRINT("m1, case 3");
				break;
			}
		/*	while(button1())
				nap(1);*/
		}else if(button2() && (temp = nmenuhit(&m2,2,0))){
			switch(temp->data){
			case 0:
				PRINT("m1, case 0");
				break;
			case 1:
				PRINT("m1, case 1");
				break;
			case 2:
				PRINT("m1, case 2");
				break;
			case 3:
				PRINT("m1, case 3");
				break;
			case 4:
				PRINT("m2, case 0");
				break;
			case 5:
				PRINT("m2, case 1");
				break;
			case 6:
				PRINT("m2, case 2");
				break;
			case 7:
				PRINT("m2, case 3");
				break;
			case 8:
				PRINT("m2, case 4");
				break;
			case 9:
				PRINT("m2, case 5");
				break;
			case 10:
				PRINT("m2, case 6");
				break;
			}
		/*	while(button2())
				nap(2);*/
		}else if(button3() && (temp = nmenuhit(&m3,3,0))){
			switch(temp->data){
			case 0:
				PRINT("m4, case 0");
				break;
			case 1:
				PRINT("m4, case 1");
				break;
			case 2:
				PRINT("m4, case 2");
				break;
			case 11:
				PRINT("m3, case 0");
				break;
			case 12:
				PRINT("m3, case 1");
				break;
			case 13:
				quit();
				break;
			}
		/*	while(button3())
				nap(1);*/
		}
	}
}

void quit(){
	PRINT("quitting soon");
	sleep(15);
	exit(0);
}

void f1(){
	sprintf(s,"f1(): a = %d",a);
	PRINT(s);
/*	nap(2);*/
	a++;
}

void f2(){
	sprintf(s,"f2(): b = %d",b);
	PRINT(s);
/*	nap(2);*/
	b++;
}

void f3(){
	sprintf(s,"f3(): c = %d",c);
	PRINT(s);
/*	nap(2);*/
	c++;
}
