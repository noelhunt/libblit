#include "../frame/frame.h"
#include "msgs.h"
#include "menu.h"

typedef struct Text{
	Frame	*frame;
	unsigned char file;	/* Which file associated with frame */
	char	obscured;	/* Whether another frame covers part of this */
	int	selecthuge;	/* selection is bigger than visible */
	struct Text *next;	/* linked back to front */
}Text;
extern Rectangle diagrect;
extern int diagclr;
extern short newlnsz;
extern Text *current;	/* which guy we are typing at (can be diag) */
extern Text *worktext;	/* which guy we are working on (never diag) */
extern Text *text;	/* list of text frames */
extern Texture bullseye;
extern Texture deadmouse;
extern String typein;
extern int diagdone;
extern int diagnewline;
extern int snarfhuge;
extern Text *diag;	/* separate from the rest */
extern Text *txalloc();

#define	PRIME		0
#define	STARDOT		1

#define	INFINITY	32767
#define	M		2

Text	*textoffile(), *textofpoint();

extern Rectangle screenrect;
#define	UP	0
#define	DOWN	1
#define	MAXFILES	256
extern unsigned char menupos[];
int iodone, diagdone, scrolllines, filedone;

/* main.c */
void sendsnarf(void);
void panic(char *s);
void closeall(void);
void txinit(Text *t);
void txfree(Text *t);
void close(Text *t);
void init(void);
void tellseek(Text *t, int y);
int usualtest(void);
void loadfile(Text *t, int posn, int n);
void urequest(int f);
void waitunix(int *flag);
void message(void);
int integer(char *s);
void mesg(char *s, int sendit);
void sendstr(Text *t, int op, int posn, int n, char *d);
int scrolltest(void);
int scroll(Text *t, int nlines);
char *data2(int n);
void Send(int op, int posn, int n, char *s);
