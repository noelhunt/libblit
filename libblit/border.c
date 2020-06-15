#include <blit.h>

void border(Bitmap *b, Rectangle r, int i, int v){
	ulong col = pixval(v, 0);
	Rectangle s[4];
	Bitmap *m[4];

	if(i > 0){
		s[0] = Rect(r.min.x, r.min.y, r.max.x, r.min.y+i);
		s[1] = Rect(r.min.x, r.max.y-i, r.max.x, r.max.y);
		s[2] = Rect(r.min.x, r.min.y+i, r.min.x+i, r.max.y-i);
		s[3] = Rect(r.max.x-i, r.min.y+i, r.max.x, r.max.y-i);
	}else if(i < 0){
		s[0] = Rect(r.min.x, r.min.y+i, r.max.x, r.min.y);
		s[1] = Rect(r.min.x, r.max.y, r.max.x, r.max.y-i);
		s[2] = Rect(r.min.x+i, r.min.y+i, r.min.x, r.max.y-i);
		s[3] = Rect(r.max.x, r.min.y+i, r.max.x-i, r.max.y-i);
	}
	m[0] = balloc(rectsubpt(s[0], s[0].min), screen.ldepth);
	rectf(m[0], m[0]->r, col, S);
	m[1] = balloc(rectsubpt(s[1], s[1].min), screen.ldepth);
	rectf(m[1], m[1]->r, col, S);
	m[2] = balloc(rectsubpt(s[2], s[2].min), screen.ldepth);
	rectf(m[2], m[2]->r, col, S);
	m[3] = balloc(rectsubpt(s[3], s[3].min), screen.ldepth);
	rectf(m[3], m[3]->r, col, S);
	if(i > 0){
		bitblt(b, r.min, m[0], m[0]->r, S);
		bitblt(b, Pt(r.min.x, r.max.y-i), m[1], m[1]->r, S);
		bitblt(b, Pt(r.min.x, r.min.y+i), m[2], m[2]->r, S);
		bitblt(b, Pt(r.max.x-i, r.min.y+i), m[3], m[3]->r, S);
	}else if(i < 0){
		bitblt(b, Pt(r.min.x, r.min.y+i), m[0], m[0]->r, S);
		bitblt(b, Pt(r.min.x, r.max.y), m[1], m[1]->r, S);
		bitblt(b, Pt(r.min.x+i, r.min.y+i), m[2], m[2]->r, S);
		bitblt(b, Pt(r.max.x, r.min.y+i), m[3], m[3]->r, S);
	}
}
