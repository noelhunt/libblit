#include <blit.h>
#define	MAXLINES	66/8	/* assume 8 is minimum newlnsz */

typedef unsigned long        Posn;
typedef struct String{
	char *s;	/* pointer to string */
	short n;	/* number used, no terminal null */
	short size;	/* size of allocated area */
} String;

typedef unsigned char Nchar;	/* number of chars on a line */
typedef struct Frame{
	Rectangle rect;		/* Screen area of frame, exact #lines high */
	Rectangle scrollrect;	/* Screen area of scrollbar */
	Rectangle totalrect;	/* Screen area covered by entire frame */
	String 	str;		/* What's in the frame */
	unsigned short	s1, s2;	/* Indexes of ends of selected text */
	int	scrolly;	/* last argument to tellseek, for redrawing */
	short	nlines;		/* Number of screen lines of text */
	short	nullsel;	/* True if last selection was null */
	short	lastch;		/* Last selected char when s1==s2 */
	short	margin;		/* margin around frame */
	Nchar	cpl[MAXLINES];	/* Number of characters per line */
} Frame;

#define	SCROLLWIDTH	10	/* width of scroll bar */

extern Rectangle canon();
extern Point nullpoint,toscreen(), startline();
extern String snarfbuf;
extern short newlnsz;
extern Point endpoint;	/* last position drawn during a frameop() */
extern int complete;	/* did frameop do all it was supposed to? */
extern int inscomplete;	/* is full insertion visible on screen? */
extern int F_rectf;		/* function code for oprectf */

#define	TRUE	1
#define	FALSE	0

int cwidth(char);
int frdelete(Frame *t, int f_clr);
void frinsert(Frame*, String*, int);
int nexttab(Frame *t, int x);
void frameop(Frame *t, void (*op)(Frame*,Point,Point,char*,int), Point pt, char *cp, int n);
void opdraw(Frame*, Point, Point, char*, int);	/* standard routine to draw text */
void draw(Frame *t, Point p, char *s, int n);
void oprectf(Frame *t, Point p, Point q, char *str, int n);
void selectf(Frame *t, int f);
void opnull(Frame*,Point,Point,char*,int);	/* do nothing routine for frameop side effects */
void rXOR(Rectangle r);
void Rectf(Rectangle r, Fcode f);
Point ptofchar(Frame *t, int cn);
int charofpt(Frame *t, Point pt);
Point startline(Frame *t, int n);
void setcpl(Frame *t, int first, int last);
void opcpl(Frame *t, Point p, Point q, char *cp, int n);
int cpl(Frame *t, int posn, Point pt);
void scrollcpl(Frame *t, int first, int last, int n);
int lineno(Frame *t, int y);
Frame *fralloc(Rectangle r, int m);
void setrects(Frame *t, Rectangle r, int m);
void frinit(Frame *t);
void frfree(Frame *f);
