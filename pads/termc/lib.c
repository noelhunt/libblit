#include <stdarg.h>
#include <stdio.h>
#include "univ.h"

char *itoa(int n){
	static char pic[] = " [-][0-9]* ";
	char cip[10];
	char *p = pic, *c;

	*p++ = ' ';
	if( n < 0 ) *p++ = '-', n = -n;
	*(c = &cip[0]) = '\0';
	do {
		*++c = "0123456789"[n%10];
	} while( n = n/10 );
	while( *c ) *p++ = *c--;
	*p++ = ' ';
	*p = '\0';
	return pic;
}
	
int dictorder(char *a, char *b){
	int disc;

	for( ;; ){
		if( !*a	) return 1;
		if( !*b	) return 0;
		if( disc = (*a++|' ')-(*b++|' ') ) return disc<0;
	}
}

#ifdef NOTDEF
void PaintAssert(char *msg, Rectangle r, Bitmap *back, Bitmap *text, Bitmap *bord){
        draw(screen, r, back, nil, ZP);
        border(screen, r, PADBORDER, bord, ZP);
        string(screen, addpt(r.min, Pt(GAP*2,GAP*2)), text, ZP, font, msg);
}

int assertf(int l){
	int wide;
	Point p;
	Rectangle r;
	Bitmap *back, *bord;
	char *s = "Assertion failed.";
	if( l ) return l;
	back = allocimage(display, Rect(0,0,1,1), screen->chan, 1, DRed);
	bord = allocimage(display, Rect(0,0,1,1), screen->chan, 1, DGreen);
	p = mouse.xy; p.x = min(p.x, screen->r.min.x-300);
	p = Pt(10,10);
	wide = stringwidth(font, s)+GAP*4;
	r = rectaddpt( Rect(0,0,wide,font->height+GAP*4), p );
	PaintAssert(s, r, back, display->white, bord);
	flushimage(display, 1);		/* in case display->locking is set */
	sleep( 5 SECONDS );
	abort();
}
#endif

#ifdef ASSERT
int assertf(int l, char *s){
	extern char KBDStr[];
	char *k = KBDStr, *p = ": Assertion failed.";
	if( l ) return l;
	rectf( &screen, Drect, 0, S );
	while( *s ) *k++ = *s++;
	while( *p ) *k++ = *p++;
	PaintKBD();
	for( ;; ) sleep(60);
}
#endif

Point dxordy(Point prev){
	int dx, dy;
	Point mxy;

	mxy = mouse.xy;
	if( abs(dy = mxy.y-prev.y) >= abs(dx = mxy.x-prev.x) )
		dx = 0;
	else
		dy = 0;
	cursset( mxy = addpt( prev, Pt(dx,dy) ));
	return mxy;
}

Rectangle boundrect( Rectangle q, Rectangle r ){
	q.min.x = min( q.min.x, r.min.x );
	q.min.y = min( q.min.y, r.min.y );
	q.max.x = max( q.max.x, r.max.x );
	q.max.y = max( q.max.y, r.max.y );
	return q;
}

Rectangle scrollbar( Rectangle r, int from, int to, int lo, int hi ){
	int rh = Dy(r), h = hi-lo;
#ifdef TAGS
	if( h == 0 ) return r;
#endif
	r.max.y -= muldiv(hi-to,rh,h);
	r.min.y += muldiv(from-lo,rh,h);
	return r;
}

#ifdef TAC
int rectinrect( Rectangle r, Rectangle s ){
	Rectangle clipped = r;

	return rectclip(&clipped,s) && eqrect(r, clipped);
}
#endif

char *FreeSome = "out of terminal memory; free some to continue";

char *Alloc(int n){
	char *a;
	if( !(a = malloc(n)) ){
		cursswitch( &NoMemory );
		InvertKBDrect( FreeSome, "" );
		sleep(100);
		cursswitch( Jcursor );
	}
	while( n>0 ) a[--n] = 0;
	return a;
}

Rectangle moverect(Rectangle source, Rectangle bound){
	Rectangle target;
	Point base, track;
	int b = butts;

	base = mouse.xy;
	bound.min.x += base.x - source.min.x;
	bound.min.y += base.y - source.min.y;
	bound.max.x += base.x - source.max.x;
	bound.max.y += base.y - source.max.y;
	do {
		track = mouse.xy;
		if( track.x < bound.min.x ) track.x = bound.min.x;
		if( track.y < bound.min.y ) track.y = bound.min.y;
		if( track.x > bound.max.x ) track.x = bound.max.x;
		if( track.y > bound.max.y ) track.y = bound.max.y;
		borders( target = rectaddpt( source, subpt(track,base) ), 1 );
		wait(MOUSE);
		borders( target, 0 );
	} while( butts == b );
	return butts ? source : target;
}

void Quit(void){ quitok = 1; }

void panic(char *s){
	fprintf(stderr, "padsterm: panic: %s\n", s);
	abort();
}

static int depth = 0;
static char dbuf[256];

void dprint( int step, char* fmt, ... ){
	int n;
	va_list av;
	memset(dbuf, 0, 256);
	if (step == 1)
		n = sprintf(dbuf, "%*s ", ++depth, "+");
	else if (step == 2)
		n = sprintf(dbuf, "%*s ", depth, "|");
	else if (step == 0)
		n = sprintf(dbuf, "%*s ", depth--, "-");
	else
		return;

	va_start(av, fmt);
	vsnprintf(dbuf+n, 128-n-1, fmt, av);
	va_end(av);
	fprintf(stderr, "%s\n", dbuf);
}
