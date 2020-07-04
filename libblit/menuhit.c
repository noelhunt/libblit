#if defined(sun)
#include <strings.h>
#elif defined(linux)
#include <string.h>
#endif
#include <blit.h>

#define scale(x, inmin, inmax, outmin, outmax)\
	(outmin + muldiv(x-inmin,outmax-outmin,inmax-inmin))

#define bound(x, low, high) min(high, max( low, x ))

#define DISPLAY		16
#define BARWIDTH	18

static void flip(Rectangle, int, int);

static char **table;

static char *tablegen(int i){ return table[i]; }

int menuhit(int but, Menu *m){
	register int width, i, j, top, newtop, hit, newhit, items;
	register int lines, length;
	Point p, q, savep, baro, barc;
	Rectangle sr, tr, mr;	/* scroll, text, menu */
	register Bitmap *b;
	char *s, *(*generator)(), *from, *to, fill[64];
	int spacing = fontheight(font);
	int charwidth = fontwidth(font);

#define sro sr.min
#define src sr.max
#define tro tr.min
#define trc tr.max
#define mro mr.min
#define mrc mr.max

	generator = (table=m->item) ? tablegen : m->generator;
	length = width = items = 0;
	for( ; s=(*generator)(items); ++items) {
		length = max(length, strlen(s));
		width = max(width, strwidth(font,s));
	}
	if(items == 0){
		while(button(but));
		return -1;
	}
	Jscreengrab();
	width += 10;
	sro.x = sro.y = src.x = tro.x = mro.x = mro.y = 0;
	if(items <= DISPLAY)
		lines = items;
	else{
		lines = DISPLAY;
		tro.x = src.x = BARWIDTH;
		sro.x = sro.y = 1;
	}
	tro.y = 1;
	mrc = trc = addpt(Pt(tro.x, mro.y), Pt(width, min(items, lines)*spacing+2));
	trc.y = src.y = mrc.y-1;
	newtop = bound(m->prevtop, 0, items-lines);
	p = addpt(mouse.xy, Joffset);
	p.y -= bound(m->prevhit, 0, lines-1)*spacing+spacing/2;
	p.x = bound(p.x-(src.x+width/2), Jfscreen.r.min.x,
		Jfscreen.r.max.x-mrc.x);
	p.y = bound(p.y, Jfscreen.r.min.y, Jfscreen.r.max.y-mrc.y);
	sr = rectaddpt(sr, p);
	tr = rectaddpt(tr, p);
	mr = rectaddpt(mr, p);
	b = balloc(mr, screen.ldepth);
PaintMenu:
	bitblt(&screen, mr.min, &screen, mr, 0);
	border(&screen, mr, 1, F);
	top = newtop;
	if(items > DISPLAY){
		baro.y = scale(top, 0, items, sro.y, src.y);
		baro.x = sr.min.x;
		barc.y = scale(top+DISPLAY, 0, items, sro.y, src.y);
		barc.x = sr.max.x;
		rectf(&screen, Rpt(baro,barc), ~0, S);
	}
	for(p=tro, i=top; i < min(top+lines, items); ++i){
		q = p;
		from = generator(i);
		for(to = &fill[0]; *from; ++from)
			if(*from & 0x80)
				for(j=length-(strlen(from+1)+(to-&fill[0])); j-->0;)
					*to++ = *from & 0x7F;
			else
				*to++ = *from;
		*to = '\0';
		q.x += (width-strwidth(font,fill))/2;
		string(&screen, q, font, fill, ~0, S);
		p.y += spacing;
	}
	savep = mouse.xy;
	for(newhit = hit = -1; button(but); nap(2)){
		p = mouse.xy;
		if(ptinrect(p, sr)){
			if(ptinrect(savep,tr)){
				p.y = (baro.y+barc.y)/2;
				cursset(subpt(p,Joffset));
			}
			newtop = scale(p.y, sro.y, src.y, 0, items);
			newtop = bound(newtop-DISPLAY/2, 0, items-DISPLAY);
			if(newtop != top)
				goto PaintMenu;
		}
		savep = p;
		newhit = -1;
		if(ptinrect(p, tr)){
			newhit = bound((p.y-tro.y)/spacing, 0, lines-1);
			if(newhit!=hit && hit>=0
			 && abs(tro.y+spacing*newhit+spacing/2-p.y) > spacing/3)
				newhit = hit;
		}
		if(newhit != hit){
			flip(tr, hit, spacing);
			flip(tr, hit = newhit, spacing);
		}
		if(newhit==0 && top>0){
			newtop = top-1;
			p.y += spacing;
			cursset(p);
			goto PaintMenu;
		}
		if(newhit==DISPLAY-1 && top<items-lines){
			newtop = top+1;
			p.y -= spacing;
			cursset(p);
			goto PaintMenu;
		}
	}
	if(b){
		bitblt(&screen, mr.min, b, mr, S);
		bfree(b);
	}
	Jscreenrelease();
	if(hit>=0){
		m->prevhit = hit;
		m->prevtop = top;
		return hit+top;
	}else
		return -1;
}

static void flip(Rectangle r,int n,int spacing){
	if(n<0)
		return;
	++r.min.x;
	r.max.y = (r.min.y += spacing*n) + spacing;
	--r.max.x;
	bitblt(&screen, r.min, &screen, r, F&~D);
}
