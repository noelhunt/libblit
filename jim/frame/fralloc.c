#include <stdlib.h>
#include "frame.h"
void strzero(String*);
Frame *fralloc(Rectangle r, int m){
	Frame *t=(Frame *)alloc(sizeof(Frame));
	if(t){
		strzero(&t->str);
		setrects(t, r, m);
	}
	return t;
}
void setrects(Frame *t, Rectangle r, int m){
	t->totalrect = r;
	t->rect = insetrect(r, m);
	t->rect.max.y -= (t->rect.max.y-t->rect.min.y)%newlnsz;
	t->nlines = (t->rect.max.y-t->rect.min.y)/newlnsz;
	t->scrollrect = t->rect;
	t->scrollrect.min.x += 2;
	t->scrollrect.max.x = t->scrollrect.min.x+SCROLLWIDTH;
	t->rect.min.x += SCROLLWIDTH+SCROLLWIDTH/2;
	setcpl(t, 0, t->nlines-1);
}
void frinit(Frame *t){
	box(t);
	strzero(&t->str);
	t->s1=t->s2=0;
	setcpl(t, 0, t->nlines-1);
}
void frfree(Frame *f){
	if(f==0 || f->str.s==0)
		return;
	gcfree(f->str.s);
	free(f);
}
