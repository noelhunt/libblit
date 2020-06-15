#include "frame.h"

int eightspaces=72;	/* sleazy */
Point endpoint;
int nexttab(Frame *t, int x){
	int xx=x-t->rect.min.x;
	return(xx-(xx%eightspaces)+eightspaces+t->rect.min.x);
}
void opshow();
int complete;
void frameop(Frame *t, void (*op)(Frame*,Point,Point,char*,int), Point pt, char *cp, int n){
	int i, j;
	Point startpt;
	register char *startcp;
	complete=0;
	endpoint=pt;
	i=0;	/* Not in for(;;) because of \t's */
    Top_of_loop:
	endpoint=pt;
	for(j=0,startpt=pt,startcp=cp; ; i++,j++,cp++){
    Continue:
		if(i>=n)
			break;
		if(*cp=='\n'){
			i++, j++, cp++;
	    Emit_newline:
			pt.x=t->rect.max.x;
			endpoint=pt;
			/* must do this even if j==0 */
			(*op)(t, startpt, pt, startcp, j);
			j=0;
			startpt.x=t->rect.min.x;
			startpt.y+=newlnsz;
			if(startpt.y >= t->rect.max.y)	/* off screen */
				return;
			pt=startpt;
			startcp=cp;
			goto Continue;
		}
		if(*cp=='\t'){
			if(j>0){
				/* Emit what's saved up */
				(*op)(t, startpt, pt, startcp, j);
				goto Top_of_loop;
			}
			if((pt.x=nexttab(t, pt.x))>t->rect.max.x)
				goto Emit_newline;
			(*op)(t, startpt, pt, cp++, 1);
			i++;
			goto Top_of_loop;
		}
		if(*cp>=' ' && *cp<='\177')
			if((pt.x+=cwidth(*cp)) > t->rect.max.x)
				goto Emit_newline;
	}
	if(startpt.y<t->rect.max.y){
		endpoint=pt;
		(*op)(t, startpt, pt, startcp, j);
		complete=1;
	}
}
char genbuf[100];
/*ARGSUSED*/
void opdraw(Frame *t, Point p, Point q, char *cp, int n){
	register char *gp=genbuf;
	while(n--){
		if(*cp>=' ' && *cp<='\177')	/* in case the font has ctrl chars in it */
			*gp++= *cp;
		cp++;
	}
	*gp=0;
	p.x++;
	string(&screen, p, &defont, genbuf, D^S);
}
void draw(Frame *t, Point p, char *s, int n){
	frameop(t, opdraw, p, s, n);
}

int	F_rectf;
/*ARGSUSED*/
void oprectf(Frame *t, Point p, Point q, char *str, int n){
	rectf(&screen, Rpt(p, Pt(q.x, q.y+newlnsz)), 0, F_rectf);
}
void selectf(Frame *t, int f){
	F_rectf=f;
	frameop(t, oprectf, ptofchar(t, t->s1), t->str.s+t->s1, t->s2-t->s1);
}
void opnull(Frame* t,Point p,Point q,char* s,int n){}
void rXOR(Rectangle r){		/* a space-saving routine */
	rectf(&screen, r, 0, D^S);
}
void Rectf(Rectangle r, Fcode f){
	rectf(&screen, r, ~0, f);
}
