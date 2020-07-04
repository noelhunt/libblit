#include "jim.h"
#include "file.h"
#include <strings.h>

#define	MINSIZE	16	/* minimum number of chars allocated */

char	*strncpy();

void Strinit(String *p){
	gcnew(p->s, MINSIZE);
	p->size=MINSIZE;
	p->n=0;
}
void Strfree(String *p){
	if(p->s)
		gcfree(p->s);
	p->s=0;
	p->n=0;
	p->size=0;
}
String *bldstring(char *s){
	static String junk;
	junk.s=s;
	junk.n=strlen(s);
	return &junk;
}
char *charstar(String *p){
	static char buf[256];
	int n=p->n;
	if(n>=256)
		n=256-1;
	strncpy(buf, p->s, n);
	buf[n]=0;
	return buf;
}
void Strzero(String *p){
	if(p->size>MINSIZE){
		(void)gcrealloc(p->s, (ulong)MINSIZE);	/* throw away the garbage */
		p->size=MINSIZE;
	}
	p->n=0;
}
void Strdup(String *p, char *s){
	Strinsure(p, p->n=strlen(s));
	strncpy(p->s, s, (int)p->n);
}
void Straddc(String *p, char c){
	Strinsure(p, p->n+1);
	p->s[p->n++] = c;
}
void Strinsure(String *p, ulong n){
	int i;
	if(p->size<n){	/* p needs to grow */
		for(i=1; i<n; i<<=1)
			;
		gcrenew(p->s, i);
		p->size=i;
	}
}
void Strinsert(String *p, String *q, Posn p0){
	if(p0>p->n) panic("Strinsert");
	Strinsure(p, (ulong)(p->n+q->n));
	Bcopy(p->s+p0, p->s+p->n, p->s+p0+q->n, -1);
	Bcopy(q->s, q->s+q->n, p->s+p0, 1);
	p->n+=q->n;
}
void Strdelete(String *p, Posn p1, Posn p2){
	if(p1>=p->n || p2>p->n || p1>p2) panic("Strdelete");
	Bcopy(p->s+p2, p->s+p->n, p->s+p1, 1);
	p->n-=(p2-p1);
}
