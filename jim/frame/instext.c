#include "frame.h"
int inscomplete;
#define	MAXEXTRA	1024	/* max # chars off end of screen */
void frinsert(Frame *t, String *s, int sn){	/* char number at which to insert */
	register char *p, *q;
	int n, y1, y2;
	Point sp;	/* point on screen */
	void strinsert(String*, String*, Posn);
	void strdelete(String*, Posn, Posn);
	sp=ptofchar(t, sn);
	p=t->str.s+sn;
	for(n=0,q=p; sn+n<t->str.n && *q!='\n'; q++)
		n++;
	/* n is #chars to next '\n' */
	frameop(t, opnull, sp, s->s, s->n);
	frameop(t, opnull, endpoint, p, n);
	y2=endpoint.y;		/* where p[n] will end up */
	draw(t, sp, p, n);	/* undraws, leaves endpoint */
	y1=endpoint.y;		/* where p[n] is now */
	strinsert(&t->str, s, sn);	/* build new string */
	if(y1 != y2){		/* bitblt up a hole */
		y1+=newlnsz;	/* BOTTOM of char */
		y2+=newlnsz;
		bitblt(&screen, Pt(t->rect.min.x, y2), &screen,
			Rect(t->rect.min.x, y1,
				t->rect.max.x, t->rect.max.y-(y2-y1)),
			 S);
		rectf(&screen, Rect(t->rect.min.x, y1, t->rect.max.x, y2), 0, F);
		y1=(y1-t->rect.min.y)/newlnsz-1;
		y2=(y2-t->rect.min.y)/newlnsz-1;
		scrollcpl(t, t->nlines-1-(y2-y1), y1, y2-y1);
	}else
		y2=(y2-t->rect.min.y)/newlnsz;
	draw(t, sp, t->str.s+sn, n+s->n);	/* redraw this line */
	inscomplete=complete;	/* cough! */
	setcpl(t, (ptofchar(t, sn).y-t->rect.min.y)/newlnsz, y2);
	n=charofpt(t, screen.r.max);	/* Re-use of n */
	if(t->str.n-n > MAXEXTRA)
		strdelete(&t->str, n+MAXEXTRA/2, t->str.n);
}
