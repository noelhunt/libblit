#include "univ.h"

#include <stdio.h>

#define SPACING		(fontheight(font)+VSPACING)

enum {
	ARROWIDTH   = 2,	/* allow this amount of char space	*/
	ARROWSIZE   = 16,	/* size of texture			*/
	ASCEND      =  2,	/* AGH: gap between top of tr and mr	*/
	BARWIDTH    = 18,	/* width of scroll bar			*/
	BLACKBORDER =  2,	/* width of outlining border		*/
	BORDER      =  2,	/* outside to selection boxes		*/
	DELTA       =  6,
	DISPLAY     = 13,
	GULLY       =  4,	/* between text and scroll bar		*/
	MARGIN      =  3,	/* outside to text			*/
	MAXDEPTH    = 16,
	MAXUNSCROLL = 21,	/* maximum #entries before scrolling turns on */
	MBORDER     =  2,	/* outside to selection boxes		*/
	MENUDEPTH   =  8,	/* maximum depth of cascade		*/
	NSCROLL     = 13,	/* number entries in scrolling part	*/
	RMARGIN     = 10,	/* text/rmargin gap			*/
	VSPACING    =  2,	/* extra spacing between lines of text	*/
};

static uchar arrowdata[] = {
	0x00, 0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70,
	0x1f, 0xf8, 0x1f, 0xfc, 0x1f, 0xfe, 0x1f, 0xfe,
	0x1f, 0xfc, 0x1f, 0xf8, 0x00, 0x70, 0x00, 0x60,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int MenuNest;

Entry *EntryGen(int i, Script *s){
	Entry *e;

	for( ; s; s = s->more ){
		CIndex = s->cindex;
		if( i < s->items )
			return (s->generator)(i);
		i -= s->items;
	}
	return 0;
}

void Limits(Script *s){
	int i, l;
	Entry *e;

	s->items = s->width = 0;
	for( i = 0; e = (s->generator)(i); ++i ){
		++s->items;
		l = strlen(e->text);
		if( l > s->width ) s->width = l;
	}
}

#define scale( x, inmin, inmax, outmin, outmax )\
	( outmin + muldiv(x-inmin,outmax-outmin,inmax-inmin) )

#define bound( x, low, high ) ( min( high, max( low, x ) ) )

static ulong menucols[NCOL] = { 0 };

static void Setcols(void){
	extern ulong _fgpixel;

	/* Main tone is greenish, with negative selection */
	menucols[BACK] = pixval(0xE6FFE6, 0);
	menucols[HIGH] = pixval(DDarkgreen, 0);
	menucols[BORD] = pixval(DMedgreen, 0);
	menucols[TEXT] = _fgpixel;
	menucols[HTXT] = menucols[BACK];
}

static Rectangle itemrect(Rectangle tr, int n){
	if( n<0 ) return ZRectangle;
	++tr.min.x;
	tr.max.y = (tr.min.y += SPACING*n) + SPACING;
	--tr.max.x;
	return tr;
}

void PaintEntry(Entry *e, Rectangle r, int flag, int charswide){
	int j;
	Point q = r.min;
	char *from, *to, fill[128];
	int width = charswide*CHARWIDTH+RMARGIN;

	from = e->text;
	for( to = &fill[0]; *from; ++from ){
		if( *from & 0x80 )
			for(j=charswide-(strlen(from+1)+(to-&fill[0])); j-->0;)
				*to++ = *from & 0x7F;
		else
			*to++ = *from;
	}
	*to = '\0';
	q.x += ( width-strwidth(font,fill) )/2;
	rectf(&screen, r, flag? menucols[HIGH]: menucols[BACK], S);
	string(&screen, q, font, fill, flag? menucols[HTXT]: menucols[TEXT], S);
	if( e->script ){
		static Bitmap *bb = 0;
		Rectangle s = Rect(0,0,ARROWSIZE,ARROWSIZE);
		Point qq = Pt(r.max.x-ARROWSIZE,q.y+1);
		if( !bb ){
			bb = balloc(s, 0);
			loadbitmap(bb, s.min.y, s.max.y, arrowdata);
		}
		bitblt(&screen, qq, bb, bb->r, flag? D&~S: D|S);
	}
}

static void flip(Rectangle, int, int, Script*, int, Bitmap*, Bitmap*);

struct info { int hit; int top; };

Entry *ScriptHit(Script *sh, int but, RectList *rl){
	int width, i, j, top, newtop, hit, newhit, items, lines, charswide;
	Point p, q, savep, baro, barc;
	Rectangle sr, tr, mr, nr;	/* scroll, text, menu, next */
	Bitmap *bar = 0;
	Bitmap *backup, *save;
	register char *s, *from, *to;
	register Script *m;
	register Entry *e;
	RectList lrl, *l;

#define sro sr.min
#define src sr.max
#define tro tr.min
#define trc tr.max
#define mro mr.min
#define mrc mr.max

	if(menucols[TEXT]==0) Setcols();
	charswide = items = 0;
	for( m = sh; m; m = m->more ){
		CIndex = m->cindex;
		m->items = m->width = 0;
		if( m->limits )
			m->limits(m);
		else
			Limits(m);
		items += m->items;
		if( m->width > charswide ) charswide = m->width;
	}
	p = mouse.xy;
	if( items == 0 ) return 0;
	width = charswide*CHARWIDTH+RMARGIN;
	sro.x = sro.y = src.x = mro.x = mro.y = 0;
	tro.x = BORDER;
	if( items <= MAXUNSCROLL ) lines = items;
	else {
		lines = DISPLAY;
		tro.x = src.x = BARWIDTH;
		sro.x = sro.y = BORDER;
	}
	tro.y = ASCEND;
	mrc = trc = addpt(tro, Pt(width,min(items,lines)*SPACING) );
	mrc = addpt(mrc, Pt(BORDER,BORDER));
	src.y = mrc.y-1;
	if( sh->prevhit<0 || sh->prevhit+sh->prevtop>=items )
		sh->prevhit = sh->prevtop = 0;
	newtop = bound(sh->prevtop, 0, items-lines );
	p.y -= bound(sh->prevhit, 0, lines-1)*SPACING+SPACING/2;
	p.x = bound(p.x-(src.x+width/2), 0, screen.r.max.x-mrc.x);
	p.y = bound(p.y, 0, screen.r.max.y-mrc.y);
	sr = rectaddpt(sr, p);
	tr = rectaddpt(tr, p);
	mr = rectaddpt(mr, p);
	nr = tr;
	nr.min.x = nr.max.x-ARROWSIZE;
	backup = balloc(mr, screen.ldepth);
	bitblt(backup, mro, &screen, mr, S);
	save = balloc(itemrect(tr,0), screen.ldepth);
	hit = sh->prevhit;
PaintMenu:
	rectf(&screen, mr, menucols[BACK], S);
	border(&screen, mr, BORDER, menucols[BORD]);
	top = newtop;
	if( items > DISPLAY ){
		baro.y = scale(top, 0, items, sro.y, src.y);
		baro.x = sro.x;
		barc.y = scale(top+DISPLAY, 0, items, sro.y, src.y);
		barc.x = src.x;
		rectf(&screen, insetrect(Rpt(baro,barc),1), menucols[HIGH], S);
	}
	for( i = top; i < min(top+lines,items); ++i )
		PaintEntry(EntryGen(i,sh), itemrect(tr,i-top), 0, charswide);
	flip(tr, hit, 0, sh, charswide, save, 0);
	e = hit>=0 ? EntryGen(hit+top, sh) : 0;
	savep = mouse.xy;
#ifdef AGH
	newhit = hit = -1;
#else
	newhit = -1;;
#endif
	for( ; button(but); nap(2) ){
		p = mouse.xy;
		if( ptinrect(p, sr) ){
#ifdef AGH
			if( ptinrect(savep,tr) ){
				p.y = (baro.y+barc.y)/2;
				cursset( p );
			}
#endif
			newtop = scale( p.y, sro.y, src.y, 0, items );
			newtop = bound( newtop-DISPLAY/2, 0, items-DISPLAY );
			if( newtop != top )
				goto PaintMenu;
#ifdef AGH
		} else if( ptinrect(savep,sr) ){
			int dx, dy;
			if( abs(dx = p.x-savep.x) < DELTA ) dx = 0;
			if( abs(dy = p.y-savep.y) < DELTA ) dy = 0;
			if( abs(dy) >= abs(dx) ) dx = 0; else dy = 0;
			cursset( p = addpt(savep, Pt(dx,dy)) );
#endif
		}
		savep = p;
		newhit = -1;
		if( ptinrect(p, tr) ){
			newhit = bound((p.y-tro.y)/SPACING, 0, lines-1);
			if( hit>=0 && newhit!=hit
			 && abs(tro.y+SPACING*newhit+SPACING/2-p.y) > SPACING/3 )
				newhit = hit;
		}
		if( newhit!=hit ){
			flip(tr, hit, top, sh, charswide, 0, save);
			flip(tr, hit = newhit, top, sh, charswide, save, 0);
			e = hit>=0 ? EntryGen(hit+top, sh) : 0;
		}
		if( !ptinrect(p, mr) ){
			assert(!e, "ScriptHit: no hit");
			for( l = rl; l; l = l->more )
				if( ptinrect(p, *l->rp) )
					goto Done;
		}
		if( (newhit!=-1) && e && e->script && ptinrect(p, nr) ){
			lrl.rp = &mr;
			lrl.more = rl;
			++MenuNest;
			e = ScriptHit(e->script, but, &lrl);
			--MenuNest;
			if( e ) goto Done;
/* ->->-> */		goto PaintMenu;
		}
		if(newhit==0 && top>0){
			newtop = top-1;
			p.y += SPACING;
			cursset(p);
/* ->->-> */		goto PaintMenu;
		}
		if(newhit==DISPLAY-1 && top<items-lines){
			newtop = top+1;
			p.y -= SPACING;
			cursset(p);
/* ->->-> */		goto PaintMenu;
		}
	}
Done:
	if(backup){
		bitblt(&screen, mr.min, backup, mr, S);
		bfree(backup);
	}
	bfree(save);
	if( hit >= 0 ){
		sh->prevhit = hit;
		sh->prevtop = top;
	}
	return e;
}

static void flip(Rectangle tr, int n, int top, Script *sh, int charswide, Bitmap *save, Bitmap *restore){
	if( n<0 ) return;
	Rectangle r = itemrect(tr, n);
	if(restore){
		bitblt(&screen, r.min, restore, restore->r, S);
		return;
	}
	bitblt(save, save->r.min, &screen, r, S);
	PaintEntry(EntryGen(n+top, sh), r, 1, charswide);
}
