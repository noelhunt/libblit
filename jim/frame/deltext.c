#include "frame.h"

#define	INFINITY	32767
int frdelete(Frame *t, int f_clr){
	register char *p, *q;
	int n, y1, y2;
	Point pt;
	void strdelete(String*, Posn, Posn);
	selectf(t, f_clr);	/* also sets F_rectf */
	p=t->str.s+t->s2;
	for(n=0,q=p; t->s2+n<t->str.n && *q!='\n'; q++)
		n++;
	/* Found the '\n' at the end of this line; clear out */
	frameop(t, oprectf, ptofchar(t, t->s2), p, n);
	y1=endpoint.y;	/* current y position of '\n' at p[n] */
	/* Draw rest of line at new place */
	pt=ptofchar(t, t->s1);
	draw(t, pt, p, n);
	y2=endpoint.y;
	if(y1 != y2){	/* More housekeeping */
		/* NOTE: y1 > y2 */
		y1+=newlnsz;
		y2+=newlnsz;
		/* Scroll up */
		bitblt(&screen,
			Pt(t->rect.min.x, y2),
			&screen,
			Rect(t->rect.min.x, y1, t->rect.max.x, t->rect.max.y),
			S);
		/* Clear the rest */
		rectf(&screen, Rpt(Pt(t->rect.min.x,
			t->rect.max.y-(y1-y2)),
			t->rect.max), ~0, f_clr);
		y1-=newlnsz;
		y2-=newlnsz;
	}
	y1=lineno(t, y1);
	y2=lineno(t, y2);
	strdelete(&t->str, t->s1, t->s2);
	if(y1 == y2)
		setcpl(t, lineno(t, pt.y), y1);
	else
		setcpl(t, 0, t->nlines-1);	/* SLOW, correct */
	t->s2=t->s1;
	if(y1 != y2){
		n=charofpt(t, Pt(0, (t->nlines-(y1-y2))
			*newlnsz+t->rect.min.y));	/* Re-use of n */
		/* n is last visible char */
		draw(t, ptofchar(t, n), t->str.s+n, t->str.n-n);
		if(complete)	/* There's more to get from file */
			return TRUE;
	}
	return FALSE;
}
