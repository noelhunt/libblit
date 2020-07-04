#if defined(sun)
#include <strings.h>
#elif defined(linux)
#include <string.h>
#endif
#include <blit.h>
#include <menu.h>

void cursset(Point);

static void	flip(Rectangle, int);
static void	helpon(char*, Rectangle, Bitmap**);
static void	helpoff(Bitmap **);

#define scale(x, inmin, inmax, outmin, outmax)\
	(outmin + muldiv(x-inmin,outmax-outmin,inmax-inmin))

#define bound(x, low, high) min(high, max( low, x ))

#define SPACING		(1+fontheight(font))
#define CHARWIDTH	fontwidth(font)
#define DISPLAY		16
#define DELTA		6
#define BARWIDTH	18
#define	MAXDEPTH	16	/* don't use too much stack */
#define ARROWIDTH	20

static uchar Arrowset[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
	0x00, 0x60, 0xff, 0xf0, 0xff, 0xf8, 0xff, 0xfc,
	0xff, 0xfc, 0xff, 0xf8, 0xff, 0xf0, 0x00, 0x60,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static Bitmap *arrow;

static int firstime = 1;

static int abs(int arg){
	if(arg < 0)
		arg = -arg;
	return(arg);
}

static NMitem *tablegen(int i, NMitem *table){
	return &table[i];
}

static char fill[64];

NMitem *nmenuhit(NMenu *m, int but, int depth){
	register int width, mwidth, i, j, top, newtop, hit, newhit;
	register int items, lines, length, w, x;
	Point p, q, savep, baro, barc;
	Rectangle sr, tr, mr;	/* scroll, text, menu */
	Rectangle rside, rhit;
	register Bitmap *b;
	register char *from, *to;
	Bitmap *bhelp = 0;
	NMitem *(*generator)(), *mi, *table, *ret = 0;
	int dohfn;

#define sro sr.min
#define src sr.max
#define tro tr.min
#define trc tr.max
#define mro mr.min
#define mrc mr.max

	generator = (table=m->item) ? tablegen : m->generator;
	w = x = length = 0;
	for(items = 0; (mi=(*generator)(items, table))->text; ++items) {
		register int s = strlen (mi->text);
		length = max(length, s);
		if (mi->next) {
			w = max (w, s);
		} else
			x = max (x, s);
	}
	if(items == 0)
		return(ret);
	width = length*CHARWIDTH+10;
	w *= CHARWIDTH;
	x *= CHARWIDTH;
	if (x <= w)
		mwidth = w + ARROWIDTH;
	else if (x >= w + 2*ARROWIDTH)
		mwidth = x;
	else
		mwidth = w + ARROWIDTH + (x - w) / 2;
	mwidth += 10;
	sro.x = sro.y = src.x = tro.x = mro.x = mro.y = 0;
	if(items <= DISPLAY)
		lines = items;
	else{
		lines = DISPLAY;
		tro.x = src.x = BARWIDTH;
		sro.x = sro.y = 1;
	}
#define ASCEND 2
	tro.y = ASCEND;
	mrc = trc = addpt(tro, Pt(mwidth, min(items, lines)*SPACING));
	src.y = mrc.y-1;
	newtop = bound(m->prevtop, 0, items-lines);
	p = addpt(mouse.xy, Joffset);
	p.y -= bound(m->prevhit, 0, lines-1)*SPACING+SPACING/2;
	p.x = bound(p.x-(src.x+mwidth/2), Jfscreen.r.min.x, Jfscreen.r.max.x-mrc.x);
	p.y = bound(p.y, Jfscreen.r.min.y, Jfscreen.r.max.y-mrc.y);
	sr = rectaddpt(sr, p);
	tr = rectaddpt(tr, p);
	mr = rectaddpt(mr, p);
	rside = mr;
	rside.min.x = rside.max.x-16;
//	mr = insetrect(mr, -1);
	b = balloc(mr, screen.ldepth);
	if(b)
		bitblt(b, mro, &screen, mr, S);
PaintMenu:
	bitblt(&screen, mr.min, &screen, mr, 0);
	border(&screen, mr, 1, F);
	top = newtop;
	if(items > DISPLAY){
		baro.y = scale(top, 0, items, sro.y, src.y);
		baro.x = sr.min.x;
		barc.y = scale(top+DISPLAY, 0, items, sro.y, src.y);
		barc.x = sr.max.x;
		rectf(&screen, Rpt(baro,barc), 0, S);
	}
	for(p=tro, i=top; i < min(top+lines, items); ++i){
		q = p;
		mi = generator(i, table);
		from = mi->text;
		for(to = &fill[0]; *from; ++from)
			if(*from & 0x80)
				for(j=length-(strlen(from+1)+(to-&fill[0])); j-->0;)
					*to++ = *from & 0x7F;
			else
				*to++ = *from;
		*to = '\0';
		q.x += (width-strwidth(font,fill))/2;
		string(&screen, q, font, fill, pixval(0,0), D^S);
		if(mi->next){
			if ( !arrow ) {
				arrow = balloc(Rect(0,0,16,16), 0);
				wrbitmap(arrow, 0, 16, Arrowset);
			}
			bitblt(&screen, Pt(trc.x-18, p.y-2), arrow, arrow->r, D|S);
		}
		p.y += SPACING;
	}
	savep = mouse.xy;
	mi = 0;
	for(newhit = hit = -1; button(but); ){
		p = mouse.xy;
		if(depth && ((p.x < mro.x) || button(5-but))){
			ret = 0;
			break;
		}
		if(ptinrect(p, sr)){
			if(ptinrect(savep,tr)){
				p.y = (baro.y+barc.y)/2;
				cursset(subpt(p,Joffset));
			}
			newtop = scale(p.y, sro.y, src.y, 0, items);
			newtop = bound(newtop-DISPLAY/2, 0, items-DISPLAY);
			if(newtop != top)
/* ->->-> */			goto PaintMenu;
		}else if(ptinrect(savep,sr)){
			int dx, dy;
			if(abs(dx = p.x-savep.x) < DELTA)
				dx = 0;
			if(abs(dy = p.y-savep.y) < DELTA)
				dy = 0;
			if(abs(dy) >= abs(dx))
				dx = 0;
			else
				dy = 0;
			p = addpt(savep, Pt(dx,dy));
			cursset(p);
		}
		savep = p;
		newhit = -1;
		if(ptinrect(p, tr)){
			newhit = bound((p.y-tro.y)/SPACING, 0, lines-1);
			if(newhit!=hit && hit>=0
			 && abs(tro.y+SPACING*newhit+SPACING/2-p.y) > SPACING/3)
				newhit = hit;
			rhit = tr;
			rhit.min.y += newhit*SPACING-ASCEND/2;
			rhit.max.y = rhit.min.y + SPACING;
		}
		if(newhit == -1)
			ret = 0, dohfn = 0;
		else
			ret = mi = (*generator)(top+newhit, table), dohfn = 1;
		if(newhit == hit){
			if((newhit != -1) && (bhelp == 0) && button1() && mi->help)
				helpon(mi->help, rhit, &bhelp);
			else if(bhelp && !button1())
				helpoff(&bhelp);
		}else{
			flip(tr, hit);
			helpoff(&bhelp);
			flip(tr, newhit);
			hit = newhit;
			if((newhit != -1) && button1() && mi->help)
				helpon(mi->help, rhit, &bhelp);
		}
		if((newhit != -1) && ptinrect(p, rside)){
			if(mi->dfn) (*mi->dfn)(mi);
			if(mi->next && (depth <= MAXDEPTH))
				ret = nmenuhit(mi->next, but, depth+1), dohfn = 0;
			if(mi->bfn) (*mi->bfn)(mi);
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
		if (button(but))
			nap(2);
	}
	if(bhelp)
		helpoff(&bhelp);
	if(b){
		bitblt(&screen, mr.min, b, mr, S);
		bfree(b);
	}
	if(hit>=0){
		m->prevhit = hit;
		m->prevtop = top;
		if(ret && ret->hfn && dohfn) (*ret->hfn)(mi);
	}
	return(ret);
}

static void flip(Rectangle r,int n){
	if(n<0)
		return;
	++r.min.x;
	r.max.y = (r.min.y += SPACING*n-1) + SPACING;
	--r.max.x;
	bitblt(&screen, r.min, &screen, r, F&~D);
}

static void helpon(char *msg, Rectangle r, Bitmap **bhelp){
	Bitmap *b;
	int w;

	w = strwidth(font, msg)+10;
	if(r.max.x+w < screen.r.max.x){
		r.min.x = r.max.x;
		r.max.x += w;
	}else{
		r.max.x = r.min.x;
		r.min.x -= w;
	}
	if(*bhelp = b = balloc(r = insetrect(r,-1), 0)){
		bitblt(b, r.min, &screen, r, S);
		border(&screen, r, 1, F);
		string(&screen, addpt(r.min, Pt(5, 2)), font, msg, ~0, S);
	}
}

static void helpoff(Bitmap **bhelp){
	Bitmap *bh;

	if(bh = *bhelp){
		bitblt(&screen, bh->r.min, bh, bh->r, S);
		bfree(bh);
		*bhelp = 0;
	}
}
