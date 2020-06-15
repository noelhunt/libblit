#include <stdio.h>

#include <strings.h>
#include <stdlib.h>
#include "univ.h"
Selection Selected;
Pad *Current;
Pad *HostObject;
Pad *HostParent;
short Scrolly;
Point Zpoint;
Rectangle ZRectangle;
Pad *DirtyPad;

Pad Sentinel = { 0, 0, 0, 0, 0, 0, &Sentinel, &Sentinel };
Rectangle PadSpace;
Rectangle KBDrect;
Attrib NewFold;

#define ISPAD(p) ((p) != &Sentinel)
#define ISLINE(l,p) ((l) != &(p)->sentinel)	/* used how much? */

int kbdchar();
void GCString(char**, char*);

Point PickPoint(char *s){
	char *pick = "pick window: ";
	Point p;
	cursswitch(&Bullseye);
	if( s ) InvertKBDrect( pick, s );
	while( !button123() ) waitMOUSE();
	p =  (butts==BUTT2 || butts==BUTT3) ? mouse.xy: Zpoint;
	cursswitch( Pcursor );
	PaintKBD();
	return p;
}

#define MINWD 50
#define MINHT 50
int PadSized( Rectangle r ){

	return r.max.x-r.min.x > MINWD && r.max.y-r.min.y > MINHT;
}

#define SWEEP 125
int SWEEPTIMEOUT = 60*15;
Rectangle clipgetrect(char *s){
	Rectangle r, r1;
	register long start;
	Point p1, p2;

	if( s ) InvertKBDrect( "sweep ", s );
	for( start = realtime(); ; sleep(2) ){
		if( own()&MOUSE && ptinrect(mouse.xy, insetrect(PadSpace,-SWEEP)) ){
			break;
		}
		if( realtime()>start+SWEEPTIMEOUT ){
			r = Drect;
			while( PadSized(r1= insetrect(r,1)) )
				r = r1;
			goto TimedOut;
		}
	}
	r = getrect23();
	if( r.max.x && r.max.x-r.min.x<=5 && r.max.y-r.min.y<=5 )
		r = Drect;	/* from jim */
TimedOut:
	PaintKBD();
	if( !rectclip( &r, PadSpace ) ) return ZRectangle;
	return r;
}

void DelLine( l )
Line *l;
{
	assert( l && l->down && l->up, "DelLine" );

	if( Selected.line == l ) Selected.line = 0;
	l->down->up = l->up;
	l->up->down = l->down;
	gcfree( l->text );
	free( l );
}

Line *InsAbove(Line *l, Line *t){
	Line *n;

	assert( l && l->down && l->up, "InsAbove" );
	if( !t->text ) return 0;		/* || !t->text[0] allow */
	n = salloc(Line);
	*n = *t;
	GCString(&n->text, t->text);
	n->down = l;
	n->up = l->up;
	l->up->down = n;
	l->up = n;
	return n;
}

Line *InsPos(Pad *p, Line *tk){
	register Line *l = &p->sentinel;	

	assert( p && tk, "InsPos" );
	if( (p->attributes&SORTED) && tk->text ){
		while( ISLINE(l->up,p) && !dictorder(l->up->text,tk->text) )
			l = l->up;
	} else {
		while( ISLINE(l->up,p) && l->up->key > tk->key )
			l = l->up;
	}
	return l;
}

void CreateLine(Pad *p){
	long lo, hi, k;
	Line *inspos, *l;
	Line fake;
	char tilde;

	lo = RcvLong();
	hi = RcvLong();
	if( p->sentinel.key || p->attributes&SORTED ) return;
	tilde = p->attributes&NO_TILDE ? 0 : '~';
	for( k = lo; k <= hi; ++k ) if( k ){
		fake.key = k;
		fake.carte = 0;
		fake.attributes = FAKELINE;
		fake.text = itoa(k);
		*fake.text = tilde;
		inspos = InsPos(p, &fake);
		if( inspos->up->key == k )
			DelLine( inspos->up );
		InsAbove(inspos, &fake);
		p->attributes |= FAKELINE;
		Dirty(p);
	}
}	

void PutLine(Pad *p,Protocol op){
	static Line prevrcvd;
	Line rcvd;
	char text[256];			/* 256 ? */
	register Line *l, *inspos;

	rcvd = prevrcvd;
	rcvd.object = RcvLong();
        rcvd.oid = RcvShort();
	if( op == P_NEXTLINE )
		rcvd.key = ++prevrcvd.key;
	else {
		rcvd.key = RcvLong();
		rcvd.carte = RcvShort();
		rcvd.attributes = RcvShort();
		prevrcvd = rcvd;
	}
	RcvString(rcvd.text = text);
	if( !p ) return;
	if( p->sentinel.key && rcvd.key>p->sentinel.key ) return;
	inspos = InsPos(p, &rcvd);
	{
	extern Script ObjScript;
	rcvd.phit = ObjScript.prevhit;
	rcvd.ptop = ObjScript.prevtop;
	}
	if( rcvd.attributes&SELECTLINE ) MakeCurrent(p);
	for( l = p->sentinel.up; ISLINE(l,p); l = l->up ){
		if( l->key == rcvd.key ){
			if( l == Selected.line )
				rcvd.attributes |= SELECTLINE;
			if( !strcmp(rcvd.text, l->text)
			 && rcvd.object == l->object
			 && rcvd.carte == l->carte
			 && rcvd.attributes == l->attributes
			 && !(rcvd.attributes&SELECTLINE) )
				return;
			inspos = l->down;
			if( NewFold = l->attributes & (FOLD|TRUNCATE) )
				FoldToggle( &rcvd.attributes );
			rcvd.phit = l->phit;
			rcvd.ptop = l->ptop;
			DelLine( l );
			break;
		}
	}
	l = InsAbove(inspos, &rcvd);
	if( !(rcvd.attributes&DONT_DIRTY) ) Dirty(p);
	if( l && rcvd.attributes&SELECTLINE ){	/* selected <= "" !! */
		Selected.line = l;
		Selected.pad = p;
		Paint(p);
	}
}

void SetCurrent( p )
register Pad *p;
{
	if( Current ) HeavyBorder(Current);
	if( Current = p ) HeavyBorder( p );
}

void PaintCurrent()
{
	if( Current ) Paint(Current);
}

void LineXOR( Line *l ){
	rectf( &screen, l->rect, ~0, F&~D );
}

Cover Covered( p )	/* should be smarter */
register Pad *p;
{
	register Pad *f;

	for( f = p->front; ISPAD(f); f = f->front )
		if( rectinrect( p->rect, f->rect ) ) return COMPLETE;
	for( f = p->front; ISPAD(f); f = f->front )
		if( rectXrect( p->rect, f->rect ) ) return PARTIAL;
	return CLEAR;
}

void Dirty(p)
Pad *p;
{
	void PaintForward(Rectangle, Pad*);
	if( p == DirtyPad ) return;
	if( DirtyPad && Covered(DirtyPad)!=COMPLETE )
		if( !ClipPaint(DirtyPad->rect, DirtyPad) )
			PaintForward( DirtyPad->rect, DirtyPad->front );
	DirtyPad = p;
}

void Linkin(p)
register Pad *p;
{
	SetCurrent( (Pad *) 0 );
	p->back = Sentinel.back;
	p->back->front = p;
	p->front = &Sentinel;
	Sentinel.back = p;
	if( PadSized(p->rect) ) SetCurrent(p);
}

void Unlink(p)
register Pad *p;
{
	if( p == Current ) SetCurrent( (Pad *) 0 );
	p->back->front = p->front;
	p->front->back = p->back;
	p->front = p->back = 0;		/* redundant - but caught an mcc bug! */
}

void FrontLink(p)
register Pad *p;
{
	if( !p ) return;
	Unlink(p);
	Linkin(p);
}

void PaintForward( Rectangle r, Pad *p ){
	int busy = 0;
	for( ; p != &Sentinel; p = p->front ){
		if( rectXrect(r, p->rect) && Covered(p)!=COMPLETE ){
			if( !ClipPaint( r, p ) )
				r = boundrect(r,p->rect);
			if( ++busy%4 == 0 ) wait(CPU);
		}
	}
}

void Refresh( Rectangle r ){
	rectf( &screen, r, 0, Zero );
	PaintForward( r, Sentinel.front );
}

char NewString[] = "<new>";

void P_Define( Pad *p, long o ){
	short oid = RcvShort();
	if( !p ){
		p = salloc(Pad);		/* zeros */
		p->sentinel.up = p->sentinel.down = &p->sentinel;
		p->sentinel.ptop = 255;
		Linkin(p);
		p->object = o;
		p->oid = oid;
		p->name = NewString;
		p->sentinel.text = NewString;
		p->tabs = 8;
		p->selkey = 0;
	}
	Dirty(p);
}

void P_Carte(Pad *p){
	Index i = RcvShort();
	if( p && p->object ) p->carte = i;
}

void P_Lines(Pad *p){
	register long k = RcvLong();
	if( p ){
		if( k < p->sentinel.key ) DelLines(p);
		p->sentinel.key = k;
		Dirty(p);
	}
}

void P_Banner(p)
register Pad *p;
{
	char b[256];

	RcvString(b);
	if( p ){
		if( p->sentinel.text != NewString ) gcfree( p->sentinel.text );
		GCString( &p->sentinel.text, b );
		Dirty(p);
	}
}

void P_Name(p)
register Pad *p;
{
	char n[256];

	RcvString(n);
	if( p ){
		if( p->name != NewString ) gcfree( p->name );
		GCString( &p->name, n );
	}
}

void P_Attributes(p)
register Pad *p;
{
	register Attrib a = RcvShort();
	if( p ) p->attributes = a;
}

void P_Tabs(p)
register Pad *p;
{
	register short t = RcvShort();
	if( p && t>0 && t<128 ) p->tabs = t;
	Dirty(p);
}

void P_RemoveLine(p)
register Pad *p;
{
	register long k = RcvLong();
	register Line *l;

	if( !p ) return;
	for( l = p->sentinel.up; ISLINE(l,p); l = l->up ){
		if( l->key == k ){
			DelLine( l );
			Dirty(p);
			return;
		}
	}
	
}


Pad *ObjToPad(o)
register long o;
{
	register Pad *p;

	for( p = Sentinel.back; ISPAD(p); p = p->back )
		if( p->object == o ) return p;
	return 0;
}

void Cycle()
{
	register Pad *p;

	for( p = Sentinel.back; ISPAD(p); p = p->back ){
		if( p->ticks>0 ){
			if( --p->ticks == 0 ){
				HostParent = HostObject = p;
				ToHost( P_CYCLE, /* garbage */ 0 );
			}
		}
	}
}

void MakeGap(p)
Pad *p;
{
	register Line *l, *lsent = &p->sentinel;
	register long k = RcvLong();
	register long gap = RcvLong();

	for( l = lsent->down; l!=lsent; l = l->down )
		if( l->key >= k ) l->key += gap;
}

#undef PROTODEBUG

# ifdef PROTODEBUG
#include <stdio.h>
char *OpToStr(Protocol op){
	char *s;
	static char buf[32];
	switch ( op&0xFF ) {
	default: sprintf(buf, "P_NONE.%x", op&0xFF); s = buf; break;
	case P_UCHAR: s = "P_UCHAR"; break;
	case P_SHORT: s = "P_SHORT"; break;
	case P_LONG: s = "P_LONG"; break;
	case P_CACHEOP: s = "P_CACHEOP"; break;
	case P_I_DEFINE: s = "P_I_DEFINE"; break;
	case P_I_CACHE: s = "P_I_CACHE"; break;
	case P_C_DEFINE: s = "P_C_DEFINE"; break;
	case P_C_CACHE: s = "P_C_CACHE"; break;
	case P_STRING: s = "P_STRING"; break;
	case P_INDEX: s = "P_INDEX"; break;
	case P_PADDEF: s = "P_PADDEF"; break;
	case P_ATTRIBUTE: s = "P_ATTRIBUTE"; break;
	case P_BANNER: s = "P_BANNER"; break;
	case P_CARTE: s = "P_CARTE"; break;
	case P_LINES: s = "P_LINES"; break;
	case P_NAME: s = "P_NAME"; break;
	case P_TABS: s = "P_TABS"; break;
	case P_HELPCARTE: s = "P_HELPCARTE"; break;
	case P_PADOP: s = "P_PADOP"; break;
	case P_ACTION: s = "P_ACTION"; break;
	case P_ALARM: s = "P_ALARM"; break;
	case P_CYCLE: s = "P_CYCLE"; break;
	case P_DELETE: s = "P_DELETE"; break;
	case P_KBDSTR: s = "P_KBDSTR"; break;
	case P_LINE: s = "P_LINE"; break;
	case P_LINEREQ: s = "P_LINEREQ"; break;
	case P_CLEAR: s = "P_CLEAR"; break;
	case P_MAKECURRENT: s = "P_MAKECURRENT"; break;
	case P_MAKEGAP: s = "P_MAKEGAP"; break;
	case P_NEXTLINE: s = "P_NEXTLINE"; break;
	case P_NUMERIC: s = "P_NUMERIC"; break;
	case P_USERCLOSE: s = "P_USERCLOSE"; break;
	case P_CREATELINE: s = "P_CREATELINE"; break;
	case P_REMOVELINE: s = "P_REMOVELINE"; break;
	case P_HOSTSTATE: s = "P_HOSTSTATE"; break;
	case P_BUSY: s = "P_BUSY"; break;
	case P_IDLE: s = "P_IDLE"; break;
	case P_USERCUT: s = "P_USERCUT"; break;
	case P_PICK: s = "P_PICK"; break;
	case P_HELPSTR: s = "P_HELPSTR"; break;
	case P_SHELL: s = "P_SHELL"; break;
	case P_VERSION: s = "P_VERSION"; break;
	}
	return s;
}
#endif

void PadOp(Protocol op){
	static long LINEobj;
	register long obj;
	register Pad *p;
	register short t;

# ifdef PROTODEBUG
	fprintf(stderr, "+ PadOp(%s)\n", OpToStr(op) );
# endif
	obj = op == P_NEXTLINE ? LINEobj : RcvLong();
	p = ObjToPad( obj );
	switch( (int) op ){
	case P_PADDEF:
		P_Define(p,obj);
		break;
	case P_ATTRIBUTE:
		P_Attributes(p);
		break;
	case P_REMOVELINE:
		P_RemoveLine(p);
		break;
	case P_TABS:
		P_Tabs(p);
		break;
	case P_BANNER:
		P_Banner(p);
		break;
	case P_CARTE:
		P_Carte(p);
		break;
	case P_LINES:
		P_Lines(p);
		break;
	case P_NAME:
		P_Name(p);
		break;
	case P_CLEAR:
		DelLines(p);
		Dirty(p);
		break;
	case P_MAKECURRENT:
		MakeCurrent(p);
		break;
	case P_LINE:
		LINEobj = obj;
	case P_NEXTLINE:
		PutLine(p,op);
		break;
	case P_CREATELINE:
		CreateLine(p);
		break;
	case P_DELETE:
		if( p ) p->attributes |= USERCLOSE;
		DeletePad(p);
		break;
	case P_MAKEGAP:
		MakeGap(p);
		break;
	case P_ALARM:
		t = RcvShort();
		if(p){
			if( !(p->ticks = t) ){
				HostParent = HostObject = p;
				ToHost( P_CYCLE, /* garbage */ 0 );
			}
		}
		break;
	default:
		ProtoErr( "PadOp(): " );
	}
# ifdef PROTODEBUG
	fprintf(stderr, "- PadOp(%s)\n", OpToStr(op) ); fflush(stderr);
# endif
}

void PickOp()
{
	register Pad *p;
	Index i;

	i = RcvShort();
	p = PickPad(PickPoint(IndexToStr(i)));
	MakeCurrent(p);
	while( butts ) waitMOUSE();
	if( p && (HostParent = HostObject = p) ){
		FlashBorder(p);
		PutRemote(P_PICK);
		HostAction( &i );
	}
}

Pad *PickPad(pt)
Point pt;
{
	register Pad *p;

	for( p = Sentinel.back; ISPAD(p); p = p->back )
		if( PadSized(p->rect) && ptinrect(pt,p->rect) )
			return p;
	return 0;
}

void DeletePick()
{
	DeletePad(PickPad(PickPoint(0L)));
}

void DelLines(p)
register Pad *p;
{
	if( !p ) return;
	while( ISLINE(p->sentinel.up,p) )
		DelLine( p->sentinel.up );
}

void DeletePad(Pad *p){
	Rectangle r;
	register Line *l, *lu;

	if( !p ) return;
	if( DirtyPad == p ) Dirty((Pad*) 0);
	if( p->attributes&USERCLOSE ){
		HostParent = HostObject = p;
		ToHost( P_USERCLOSE, /* garbage */ 0 );
		if( p->attributes&DONT_CLOSE ) return;
		DelLines( p );
		Unlink( p );
		if( p->sentinel.text != NewString ) gcfree( p->sentinel.text );
		if( p->name != NewString ) gcfree( p->name );
		free(p);
	} else {
		if( p->attributes&DONT_CLOSE ) return;
		for( l = p->sentinel.up; ISLINE(l,p); l = lu ){
			lu = l->up;
			if( !(l->attributes&DONT_CUT) ) DelLine(l);
		}
	}
	if( Current == p ) SetCurrent((Pad *) 0);
	if( Selected.line && Selected.pad == p ) Selected.line = 0;
	r = p->rect;
	p->rect = ZRectangle;
	Refresh( r );
}

void Select(Line *l, Pad *p){
	if( Selected.line == l ) return;
	if( Selected.line ){
		if( Selected.pad != p )
			Selected.pad->selkey = Selected.line->key;
		switch( (int) Covered(Selected.pad) ){
		case COMPLETE:
			break;
		case CLEAR:
			LineXOR(Selected.line);
			break;
		case PARTIAL:
			Selected.line = 0;
			Refresh(Selected.pad->rect);
		}
	}
	if( Selected.pad = p )
		p->selkey = 0;
	if( Selected.line = l )
		LineXOR(l);
}

void MakeCurrent(Pad *p){
	int paint;
	Line *l;	

	if( !p ) return;
	if( Selected.line && Selected.pad!=p ) Select((Line*)0, (Pad*)0);
	paint = Covered(p) != CLEAR;
	if( !PadSized(p->rect) ){
		if( Selected.line ) Select((Line*)0, (Pad*)0);
		if( !PadSized(p->rect = clipgetrect(p->sentinel.text))) return;
		paint = 1;
	}
	if( p == Current ) return;
	FrontLink(p);
	if( paint ) Paint(p);
	l = &p->sentinel;
	if( p->selkey )	
		while( ISLINE(l->up, p) ){
			l = l->up;
			if( l->key == p->selkey ){
				Select(l, p);
				break;
			}
		}
}
	
void Relocate(Pad *p,Rectangle r){
	Rectangle bounding;

	if( !PadSized(r) ) return;
	MakeCurrent(p);			/* bug - used to be FrontLink(p); */
	bounding = boundrect( r, p->rect );
	p->rect = r;
	Refresh( bounding );
}

void Move(){
	register Pad *p = PickPad(PickPoint(0L));

	if( !p ) return;
	cursinhibit();
	Relocate( p, moverect(p->rect, PadSpace) );
	cursallow();
}

void Reshape(){
	register Pad *p = PickPad(PickPoint(0L));

	if( !p ) return;
	while( button123() ) waitMOUSE();
	Relocate( p, clipgetrect(0L) );
}
	
Rectangle NewSpace;
Point Scale( Point p ){
#define		fo PadSpace.min
#define		fc PadSpace.max
#define		to NewSpace.min
#define		tc NewSpace.max
#define SCALE(xy) p.xy = to.xy + muldiv( p.xy-fo.xy, tc.xy-to.xy, fc.xy-fo.xy );

	SCALE(x);
	SCALE(y);
	return p;
}

#define KBDLEN 90
char KBDStr[KBDLEN]=  "\1";

void PadClip(){
	register Pad *p;

	KBDrect = NewSpace = Jfscreen.r;
	KBDrect.min.y = NewSpace.max.y -= fontheight(&defont)+2;
	KBDrect.max.y -= 1;
	KBDrect.min.x += 1;
	KBDrect.max.x -= 1;
	for( p = Sentinel.back; ISPAD(p); p = p->back ){
		p->rect.min = Scale( p->rect.min );
		p->rect.max = Scale( p->rect.max );
		if( !PadSized(p->rect) ){
			p->rect = ZRectangle;
			if( p == Selected.pad ){
				Selected.pad = 0;
				Selected.line = 0;
			}
		}
	}
	Refresh( PadSpace = NewSpace );
	PaintKBD();
}

void InvertKBDrect(char *s1, char *s2){
	char buf[300];

	strcpy( buf, s1 );
	strcat( buf, s2 );
	rectf( &screen, KBDrect, 0, Zero );
	string(&screen, KBDrect.min, &defont, buf, ~0, F&~D);
	rectf( &screen, KBDrect, 0, F&~D );
}		

void PaintKBD(){
	rectf( &screen, KBDrect, 0, Zero );
	string(&screen, KBDrect.min, &defont, KBDStr, ~0, F&~D);
}	
	
#define PAD_TO_SH       (1L)
#define LINE_TO_SH      (2L)
#define LINE_TO_SEL     (3L)
#define LINE_TO_PAD     (4L)
void CarriageReturn(long obj){
	register char *kbds = KBDStr;
	register Line *l, *lsent;
	register long ct;

	kbds[strlen(kbds)-1] = '\0';
	if( obj == LINE_TO_SH ){
		PutRemote(P_SHELL);
		SendLong(0L);			/* common protocol */
		SendShort(0);
		SendLong(0L);
		SendShort(0);
		SendString(kbds+1);
		SendLong(1L);
		SendString(Selected.line->text);
	} else if( obj == PAD_TO_SH ){
		PutRemote(P_SHELL);
		SendLong(0L);			/* common protocol */
		SendShort(0);
		SendLong(0L);
		SendShort(0);
		SendString(kbds+1);
		lsent = &Current->sentinel;
		ct = 0;
		for(l = lsent->down; l != lsent; l = l->down)
			++ct;
		SendLong(ct);
		for(l = lsent->down; l != lsent; l = l->down)
			SendString(l->text);
	} else {
		PutRemote(P_KBDSTR);
		SendLong(Current->object);
		SendShort(Current->oid);
		switch( obj ){
		case LINE_TO_SEL:
			SendLong(Selected.line->object);
			SendShort(Selected.line->oid);
			break;
		case LINE_TO_PAD:
			SendLong(Current->object);
			SendShort(Current->oid);
			break;
		}
		SendString(kbds);
	}
	kbds[0] = 001;
	kbds[1] = 000;
}

void LayerReshaped(){
	if( P->state & RESHAPED ){
		P->state &= ~RESHAPED;
		PadClip();
	}
}

long LiveKBD(){
	Line *sel = Selected.line;
	Pad *cur = Current;

	if( KBDStr[0] == '>' && cur ){
		if( sel ){
			DoubleOutline(&screen, sel->rect);
			return LINE_TO_SH;
		} else {
			HeavyBorder(cur);
			return PAD_TO_SH;
		}
	}
	if( sel && (sel->attributes&ACCEPT_KBD) ){
		DoubleOutline(&screen, sel->rect);
		return LINE_TO_SEL;
	}
	if( cur && (cur->attributes&ACCEPT_KBD) ){
		HeavyBorder(cur);
		return LINE_TO_PAD;
	}
	return 0L;
}

#define CNTRL_U 025
void KBDAppend(int c){
	register char *t;
	register int len = strlen(t = KBDStr);

	if( c < 040 || (c&0200) || len >= KBDLEN ){
		if( c == CNTRL_U ){
			t[0] = 001;
			t[1] = 000;
		}
		if( c != '\t' )		/* bug: \t when len >= KBDLEN !! */
			return;
	}
	t[len-1] = c;
	t[len] = 001;
	t[len+1] = 000;
}

typedef struct String String;

struct String{
	char	*s;	/* pointer to string */
	ulong	n;	/* number used; s[n]==0 */
	ulong	size;	/* size of allocated area */
};

void MuxSnarf(){
	char *cp;
	if(getmuxbuf())
		for( cp = (char*)muxbuf; *cp; ++cp ){
			if( *cp == '\n' ) break;
			KBDAppend(*cp);
		}
}

#define KBD_PAUSE 4
#define ESCAPE 033
void KBDServe(){
	register char c, *t;
	register int live, lenmake, len;

	if( own()&KBD ){
		live = LiveKBD();
		while( own() & KBD ){
			c = kbdchar();
			if( c == ESCAPE ){
				MuxSnarf();
				break;
			}
			if( c == '\r' && live ){
				CarriageReturn(live);
				break;
			}
			len = strlen(t = KBDStr);
			if( c == '\b' && len > 1 ){
				t[len-2] = 001;
				t[len-1] = 000;
				continue;
			}
			KBDAppend(c);
		}
		PaintKBD();
		sleep( KBD_PAUSE );
		LiveKBD();
	}
}

void FoldToggle(Attrib *a){
	*a &= ~(TRUNCATE|FOLD);
	*a |= NewFold;
	Paint(Current);
}

Entry *FoldEntry(Attrib *a){
	static Entry e = { 0, FoldToggle, 0 };

	assert(Current, "FoldEntry");
	if( ( (*a&(TRUNCATE|FOLD)) ? *a : Current->attributes )&TRUNCATE )
		e.text = "fold",	  NewFold = FOLD;
	else
		e.text = "truncate", NewFold = TRUNCATE;
	e.opand = (long) a;
	return &e;
}
