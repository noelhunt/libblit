#include "univ.h"
#define CHARWIDTH 9

#define COLOR
#ifdef COLOR
static ulong shade = 0;
#endif

Line *Lsent;
Rectangle Trect;
Attrib Attributes;
int CharsWide;
Line FakeLine;
Bitmap *Osbm;
char Tilde;

void DoubleOutline(Bitmap *b,Rectangle r){
	outline(b, r, 2, F&~D);
}

void HeavyBorder( Pad *p ){
	DoubleOutline( &screen, insetrect(p->rect,PADBORDER+1) );
}

Line *Linei(int i){
	register Line *lsent = Lsent, *l;
	register long ct = 1, k = lsent->key;
	Line fake;

	if( !i ) return lsent;
	for( l = lsent->down; l != lsent; l = l->down, ++ct )
		if( k ? (l->key==i) : (ct==i) )
			return l;
	if( k && i<=k ){
		FakeLine.key = i;
		*(FakeLine.text = itoa((int)i)) = Tilde;
		return &FakeLine;
	}
	return 0;
}

int Trunc(Line *l){
	int a;
	return (a=l->attributes)&(FOLD|TRUNCATE) ? a&TRUNCATE : Attributes&TRUNCATE;
}

int Expanded;
short Tabs = 8;
void Expand( char *blanks, char *tabs ){
	register int i, c;

	for( i = 0; (c = *tabs++) && i<250; )
		if( c == '\t' )
			do
				blanks[i++] = ' ';
			while( i % Tabs );
		else
			blanks[i++] = c;
	if( i==0 ) blanks[i++] = ' ';
	blanks[i] = '\0';
	Expanded = i;
}

int Needs(int i){
	register Line *l = Linei(i);
	char blanks[256];

	if( Trunc(l) ) return 1;
	Expand( blanks, l->text );
	return 1 + (Expanded-1)/CharsWide;
}

#define BARWIDTH 14
static Rectangle bar;
#ifdef GREY
ushort Darkgrey[] ={
	0xDDDD, 0x7777, 0xDDDD, 0x7777, 0xDDDD, 0x7777, 0xDDDD, 0x7777,
	0xDDDD, 0x7777, 0xDDDD, 0x7777, 0xDDDD, 0x7777, 0xDDDD, 0x7777,
};
static ushort Lightgrey[] ={
	0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,
	0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,
};
static Texture grey = 0;
#endif

int ClipPaint(Rectangle clip,Pad *p){		/* nobody said it would be easy */
	int middle, lines, lo, hi, capacity;
	int change, i, selpainted;
	register Line *lsent, *l;
	char save, *terminate, blanks[256];
	Point pt;

	Tabs = p->tabs;
	Attributes = p->attributes;
	Tilde = Attributes&NO_TILDE ? 0 : '~';
	Lsent = lsent = &p->sentinel;
	if( !PadSized(p->rect) ) return 1;

	p->srect = Trect = insetrect( p->rect, PADBORDER+4 );
	p->srect.max.x = (Trect.min.x += BARWIDTH);
	Trect.min.x += 2;
	p->srect.min.y -= 1;
	p->srect.max.y += 1;
	p->srect.min.x -= 1;

	capacity = (Trect.max.y-Trect.min.y) / fontheight(&defont);
	CharsWide = (Trect.max.x-Trect.min.x) / CHARWIDTH - 1;

	middle = lines = 0;
	for( l = lsent->down; l != lsent; l = l->down ){
		++lines;
		if( l == Selected.line && rectinrect(p->rect,clip) )
			middle = lsent->key ? Selected.line->key : lines;
	}
	if( lsent->key ) lines = lsent->key;
	if( Scrolly ){
		Scrolly -= p->srect.min.y;
		middle = 1 +
			muldiv(Scrolly,lines,p->srect.max.y-p->srect.min.y);
		if( middle > lines ) middle = lines;
	}
	if( !middle ){
		middle = (p->lo+p->hi)/2;
		if( middle<1 || middle>lines )
			middle = lsent->key ? 1 : lines;
	}
	lo = middle+1;
	hi = middle;
	do {
		change = 0;
		if( lo>0 && capacity>=Needs(lo-1)){
			capacity -= Needs(--lo);
			change = 1;
		}
		if( hi<lines && capacity>=Needs(hi+1)){
			capacity -= Needs(++hi);
			change = 1;
		}
	} while( change );
	if( lo>hi ){
		Selected.line = 0;
		return 1;
	}
	if( Scrolly && lo==p->lo && hi==p->hi ) return 1;
	p->lo = lo;
	p->hi = hi;
	for( l = lsent->down; l != lsent; l = l->down )
		l->rect = ZRectangle;
	if( Osbm && !eqrect(Osbm->r,p->rect) && Osbm!=&screen ){
		bfree(Osbm);
		Osbm = 0;
	}
	if( !Osbm && !(Osbm = balloc(p->rect, screen.ldepth)) ) Osbm = &screen;
	rectf( Osbm, p->rect, 0, Zero );
	DoubleOutline( Osbm, insetrect(p->rect, PADBORDER-1) );
	if( p == Current ) DoubleOutline( Osbm, insetrect(p->rect,PADBORDER+1) );
	if( !shade ) shade = pixval(0x555555, 0);
	rectf( Osbm, bar = scrollbar(p->srect,lo,hi+1,0,lines+1), shade, S );
	pt = Trect.min;
	selpainted = 0;
	for( ; lo <= hi; ++lo ){
		l = Linei(lo);
		l->rect.min = pt;
		Expand( blanks, l->text );
		for( i = 0; i < Expanded; i += CharsWide ){
			if( i ){
				if( Trunc(l) ) break;
				pt.x += CHARWIDTH;
			}
			save = '\0';
			if( i+CharsWide < Expanded ){
				save = *(terminate=blanks+i+CharsWide);
				*terminate = '\0';
			}
			string( Osbm, pt, &defont, blanks+i, ~0, S );
			pt.x = Trect.min.x;
			l->rect.max.y = (pt.y += fontheight(&defont));
			l->rect.max.x = Trect.max.x;
			if( save ) *terminate = save;
		}
		if( Selected.line == l ){
			rectf( Osbm, l->rect, ~0, S&~D );	/* LineXOR(l); */
			selpainted = 1;
		}
	}
	if( !selpainted && Selected.pad == p ) Selected.line = 0;
	if( !Scrolly ) RequestLines(p);
	if( Osbm != &screen ){
		Rectangle clipsrc;
		clipsrc = clip;
		rectclip(&clipsrc,Osbm->r);
		PadBlt( Osbm, clipsrc, p->front );
		return 1;
	}
	return 0;
}

#define rc (r.max)
#define ro (r.min)
#define pc (p->rect.max)
#define po (p->rect.min)

void PadBlt(Bitmap *b,Rectangle r,Pad *p){
	extern Pad Sentinel;

	if( p == &Sentinel ){
		bitblt( &screen, r.min, b, r, S );
		return;
	}
	if( !rectXrect(p->rect, r) ){
		PadBlt( b, r, p->front );
		return;
	}
	if(ro.y < po.y){
		PadBlt( b, Rpt(ro,Pt(rc.x,po.y)), p->front );
		ro.y = po.y;
	}
	if(rc.y > pc.y){
		PadBlt( b, Rpt(Pt(ro.x,pc.y),rc), p->front );
		rc.y = pc.y;
	}
	if(ro.x < po.x){
		PadBlt( b, Rpt(ro,Pt(po.x,rc.y)), p->front );
		ro.x = po.x;
	}
	if(rc.x > pc.x){
		PadBlt( b, Rpt(Pt(pc.x,ro.y),rc), p->front );
		rc.x = pc.x;
	}
}

void Paint(Pad *p){ ClipPaint(p->rect,p); }

void LineReq(Pad *p, long lo, long hi, int fake){
	Line *InsPos();

	if( lo == 0 ) lo = 1;
	if( !p || !p->object || hi < lo ) return;
	PutRemote( P_LINEREQ );
	SendLong( p->object ); SendShort(p->oid);
	SendLong( p->object ); SendShort(p->oid);
	SendLong( lo );
	SendLong( hi );
	if( fake )
		for( FakeLine.key = lo; FakeLine.key <= hi; ++FakeLine.key ){
			*(FakeLine.text = itoa((int)FakeLine.key)) = Tilde;
			InsAbove(InsPos(p,&FakeLine),&FakeLine);
		}
}
	
void CRequestLines(p)
register Pad *p;
{
	Line *l;
	int reqhi = -1, reqlo = 0, k, i;

	for( i = p->lo; i <= p->hi; ++i ){
		l = Linei(i);
		k = l->key;
		if( l->attributes&FAKELINE ){
			l->attributes &= ~FAKELINE;
			if( k == reqhi+1 )
				reqhi = k;
			else {
				LineReq(p, reqlo, reqhi, 0);
				reqlo = reqhi = k;
			}
		}
	}
	LineReq(p, reqlo, reqhi, 0);
}

void RequestLines(p)
register Pad *p;
{
	Line *lsent = Lsent, *l;
	int reqlo, cutlo, cuthi, k, cushion;

	if( !lsent || !p ) return;
	if( !lsent->key ){
		if( p->attributes&FAKELINE )
			CRequestLines(p);
		return;
	}
	cushion = Configuration&BIGMEMORY ? 1000 : 10;
	reqlo = p->lo;
	cutlo = p->lo - cushion;
	cuthi = p->hi + cushion;
	for( l = lsent->down; l != lsent; l = l->down ){
		k = l->key;
		if( k>=reqlo && k<=p->hi ){
			LineReq(p, reqlo, k-1, 1);
			reqlo = k+1;
		} else if( k<cutlo || k>cuthi )
			DelLine(l);
	}
	LineReq(p, reqlo, p->hi, 1);
		
}

void Pointing()
{
	Pad *PickPad(Point);
	Pad *p = PickPad(mouse.xy);
	Line *l, *ll, *lsent = &p->sentinel, *sel;
	int i, paint = 0;
	Point pt, ppt;
	extern Rectangle KBDrect;

	if( ptinrect(mouse.xy, KBDrect) ) return;
	if( !p ){
		Select( (Line*)0, (Pad*)0 );
		SetCurrent( (Pad *)0 );
		return;
	}
	if( p != Current ){
		MakeCurrent(p);
		while( own()&MOUSE && butts==BUTT1 ) nap(2);	/* for rollover */
		return;
	}
	pt = mouse.xy;
	while( own()&MOUSE && butts==BUTT1 ){
		paint = 0;
		pt = dxordy(ppt=pt);
		if( ptinrect(pt,p->srect) ){
			if( !ptinrect(ppt,p->srect) && ptinrect(ppt,p->rect) ){
				pt.y = (bar.min.y+bar.max.y)/2;
				cursset( pt );
			}
			Scrolly = pt.y;
			Paint(p);
		} else {
			sel = 0;
			for( l = lsent->down, i = 1; l!=lsent ; l = l->down, ++i ){
				if( ptinrect( mouse.xy, l->rect ) ){
					sel = l;
					break;
				}
			}
			Select( sel, p);
			if( sel )
				if( p->sentinel.key
				 ? (sel->key == p->lo || sel->key == p->hi )
				 : (i == p->lo || i == p->hi) ){
					paint = 1;
					Scrolly = 0;
			}
		}
	}
	if( paint )
		Paint(p);
	if( Scrolly ){
		Scrolly = 0;
		RequestLines(p);
	}
}

