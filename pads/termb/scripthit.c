#include <stdio.h>

#include <strings.h>
#include "univ.h"

#define scale( x, inmin, inmax, outmin, outmax )\
	( outmin + muldiv(x-inmin,outmax-outmin,inmax-inmin) )

#define bound( x, low, high ) ( min( high, max( low, x ) ) )

#define SPACING		(fontheight(&defont))		/* was 14 */
#define DISPLAY		13
#define CHARWIDTH	9

#define DELTA		6
#define BARWIDTH	18

int MenuNest;

static	Bitmap	*scroll;

static	uchar scrollbits[] = {
	0x22, 0x22, 0x88, 0x88, 0x22, 0x22, 0x88, 0x88,
	0x22, 0x22, 0x88, 0x88, 0x22, 0x22, 0x88, 0x88,
	0x22, 0x22, 0x88, 0x88, 0x22, 0x22, 0x88, 0x88,
	0x22, 0x22, 0x88, 0x88, 0x22, 0x22, 0x88, 0x88,
};

static uchar Arrowset[] = {
	0x00, 0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70,
	0x00, 0x78, 0x7f, 0xfc, 0x7f, 0xfe, 0x7f, 0xff,
	0x7f, 0xfe, 0x7f, 0xfc, 0x00, 0x78, 0x00, 0x70,
	0x00, 0x60, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00
};

static void flip(Rectangle,int);

Entry *EntryGen(int i, Script *s){
	register Entry *e;

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

Entry *ScriptHit(Script *sh, int but, RectList *rl){
	int width, i, j, top, newtop, hit, newhit, items, lines, charswide;
	Point p, q, savep, baro, barc;
	Rectangle sr, tr, mr;	/* scroll, text, menu */
	Bitmap *b;
	char *s, *from, *to, fill[64];
	Script *m;
	Entry *e;
	RectList lrl, *l;

#define sro sr.min
#define src sr.max
#define tro tr.min
#define trc tr.max
#define mro mr.min
#define mrc mr.max

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
	width = charswide*CHARWIDTH+10;
	mro.x = mro.y = 0;
	sro.x = sro.y = src.x = tro.x = 1;
	if( items <= DISPLAY ) lines = items;
	else {
		lines = DISPLAY;
		tro.x = src.x = BARWIDTH;
		sro.x = sro.y = 1;
	}
#define ASCEND 2
	tro.y = ASCEND;
	mrc = addpt(tro, Pt(width,min(items,lines)*SPACING));
	trc = subpt(mrc, Pt(1,1));
	src.y = mrc.y-1;
	newtop = bound(sh->prevtop, 0, items-lines );
	p.y -= bound(sh->prevhit, 0, lines-1)*SPACING+SPACING/2;
	p.x = bound(p.x-(src.x+width/2), Jfscreen.r.min.x, Jfscreen.r.max.x-mrc.x);
	p.y = bound(p.y, Jfscreen.r.min.y, Jfscreen.r.max.y-mrc.y);
	sr = rectaddpt(sr, p);
	tr = rectaddpt(tr, p);
	mr = rectaddpt(mr, p);
	b = balloc(mr, screen.ldepth);
	if( b ) bitblt(b, mro, &screen, mr, S);
	bitblt(&screen, mr.min, &screen, mr, 0);
	outline(&screen, mr, 1, F);
PaintMenu:
	rectf( &screen, insetrect(mr,1), 0, Zero );
	top = newtop;
	if( items > DISPLAY ){
		baro.y = scale(top, 0, items, sro.y, src.y);
		baro.x = sr.min.x;
		barc.y = scale(top+DISPLAY, 0, items, sro.y, src.y);
		barc.x = sr.max.x;
		rectf(&screen, Rpt(baro,barc), ~0, F);
	}
	for( p = tro, i = top; i < min(top+lines,items); ++i ){
		q = p;
		e = EntryGen(i, sh);
		from = e->text;
		for( to = &fill[0]; *from; ++from ){
			if( *from & 0x80 )
				for(j=charswide-(strlen(from+1)+(to-&fill[0])); j-->0;)
					*to++ = *from & 0x7F;
			else
				*to++ = *from;
		}
		if( e->script ){
			static Bitmap *bb;
			Point qq;
			qq.y = q.y-1;
			qq.x = q.x+width-17;
			if( !bb ){
				bb = balloc(Rect(0,0,16,16), 0);
				assert(bb, "ScriptHit: balloc()");
				wrbitmap(bb, 0, 16, Arrowset);
			}
			bitblt(&screen, qq, bb, bb->r, S);
		}
		*to = '\0';
		q.x += ( width-strwidth(&defont,fill) )/2;
		string( &screen, q, &defont, fill, ~0, S );
		p.y += SPACING;
	}
	savep = mouse.xy;
	newhit = hit = -1;
	e = 0;
	for( ; button(but); nap(2) ){
		p = mouse.xy;
		if( ptinrect(p, sr) ){
			if( ptinrect(savep,tr) ){
				p.y = (baro.y+barc.y)/2;
				cursset( p );
			}
			newtop = scale( p.y, sro.y, src.y, 0, items );
			newtop = bound( newtop-DISPLAY/2, 0, items-DISPLAY );
			if( newtop != top )
				goto PaintMenu;
		} else if( ptinrect(savep,sr) ){
			int dx, dy;
			if( abs(dx = p.x-savep.x) < DELTA ) dx = 0;
			if( abs(dy = p.y-savep.y) < DELTA ) dy = 0;
			if( abs(dy) >= abs(dx) ) dx = 0; else dy = 0;
			cursset( p = addpt(savep, Pt(dx,dy)) );
		}
		savep = p;
		newhit = -1;
		if( ptinrect(p, tr) ){
			newhit = bound((p.y-tro.y)/SPACING, 0, lines-1);
			if( hit >= 0
			 && abs(tro.y+SPACING*newhit+SPACING/2-p.y) > SPACING/3 )
				newhit = hit;
		}
		if( newhit != hit ){
			flip(tr, hit);
			flip(tr, hit = newhit);
			e = hit>=0 ? EntryGen(hit+top, sh) : 0;
		}
		if( !ptinrect(p, mr) ){
			assert(!e, "ScriptHit: no hit");
			for( l = rl; l; l = l->more )
				if( ptinrect(p, *l->rp) )
					goto Done;
		}
		if(newhit==0 && top>0){
			newtop = top-1;
			p.y += SPACING;
			cursset(p);
			goto PaintMenu;
		}
		if(newhit==DISPLAY-1 && top<items-lines){
			newtop = top+1;
			p.y -= SPACING;
			cursset(p);
			goto PaintMenu;
		}
 		if( e && e->script
		 && p.x > trc.x-CHARWIDTH ){
			p.x = trc.x;
			cursset(p);
			nap(1);
			lrl.rp = &mr;
			lrl.more = rl;
			++MenuNest;
			e = ScriptHit(e->script, but, &lrl);
			--MenuNest;
			if( e ) goto Done;
			goto PaintMenu;
		}
	}
Done:
	if( b ){
		bitblt(&screen, mr.min, b, mr, S);
		bfree(b);
	}
	sh->prevhit = hit;
	sh->prevtop = top;
	return e;
}

static void flip(Rectangle r,int n){
	if( n<0 ) return;
	++r.min.x;
	r.max.y = (r.min.y += SPACING*n-1) + SPACING;
	--r.max.x;
	bitblt(&screen, r.min, &screen, r, F&~D);
}
