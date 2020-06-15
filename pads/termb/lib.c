#include <stdlib.h>
#include <strings.h>
#include "univ.h"

void waitMOUSE() { wait(MOUSE); }

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
#ifdef ASSERT
long assertf(long l, char *s){
	extern char KBDStr[];
	register char *k = KBDStr, *p = ": Assertion failed.";
	if( l ) return l;
#ifdef LIBXDMD
	rectf( &screen, Drect, F_XOR );
#else
	rectf( &screen, Drect, 0, F );
#endif
	while( *s ) *k++ = *s++;
	while( *p ) *k++ = *p++;
	PaintKBD();
	for( ;; ) sleep(60);
}
#endif

Point dxordy(Point prev){
	int dx, dy;
	Point mxy;

	nap(2);
	mxy = mouse.xy;
	if( abs(dy = mxy.y-prev.y) >= abs(dx = mxy.x-prev.x) )
		dx = 0;
	else
		dy = 0;
	cursset( mxy = addpt( prev, Pt(dx,dy) ) );
	return mxy;
}

#ifdef BLIT
void outline( Bitmap *b, Rectangle r ){
	segment( b, r.min, Pt(r.min.x,--r.max.y), ~0, S );
	segment( b, Pt(--r.max.x,r.min.y), r.min, ~0, S );
	segment( b, Pt(r.min.x,r.max.y), r.max, ~0, S );
	segment( b, r.max, Pt(r.max.x,r.min.y), ~0, S );
}
#endif

Rectangle boundrect( Rectangle q, Rectangle r ){
	q.min.x = min( q.min.x, r.min.x );
	q.min.y = min( q.min.y, r.min.y );
	q.max.x = max( q.max.x, r.max.x );
	q.max.y = max( q.max.y, r.max.y );
	return q;
}

Rectangle scrollbar( Rectangle r, int from, int to, int lo, int hi ){
	long rh = Dy(r), h = hi-lo;
	r.max.y -= muldiv(hi-to,rh,h);
	r.min.y += muldiv(from-lo,rh,h);
	return r;
}

char *FreeSome = "out of terminal memory; free some to continue";

char *Alloc(int n){
	char *a;

	if( !(a = alloc(n)) ){
		cursswitch( &NoMemory );
		InvertKBDrect( FreeSome, "" );
		while( !(a = alloc(n)) ) wait( CPU );
		PaintKBD();
		cursswitch( Pcursor );
	}
	return a;
}

char *GCAlloc(int n,char **p){
	ulong nbytes = n;
	if( !(gcalloc(nbytes, p)) ){
		cursswitch( &NoGCMemory );
		InvertKBDrect( FreeSome, "" );
		while( !(gcalloc(nbytes, p)) ) wait( CPU );
		PaintKBD();
		cursswitch( Pcursor );
	}
	while( nbytes>0 ) (*p)[--nbytes] = 0;
	return *p;
}

void GCString( char **p, char *s ){
	*p = GCAlloc( strlen(s)+1, p );
	strcpy( *p, s );
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
		cursset( track );
		outline( &screen, target = rectaddpt( source, subpt(track,base) ), 2, S );	
		nap(2);
		outline( &screen, target, 2, S );
	} while( butts == b );
	return butts ? source : target;
}

