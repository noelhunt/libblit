#include <blit.h>
#define PADS_TERM
#include "../pads.h"

#define PADBORDER 3

long assertf();
#define ASSERT
#define assert(e,s) ( assertf( (long) (e), (s) ) )
#define salloc(s) ((struct s*) Alloc(sizeof(struct s)) )

typedef unsigned char uchar;
typedef int  (*PFI)();
typedef long (*PFL)();
typedef char (*PFC)();

typedef struct RectList RectList ;
struct RectList {
	Rectangle	*rp;
	RectList	*more;
};

typedef struct Line {
	short		oid;
	long		object;
	Index		carte;
unsigned char		ptop;
unsigned char		phit;
	Attrib		attributes;
/* ***************************** */
	char		*text;
	long		key;
	Rectangle	rect;
 struct	Line		*down;
 struct	Line		*up;
} Line;

typedef struct Pad {
	short		oid;
	long		object;
	Index		carte;
unsigned char		ptop;
unsigned char		phit;
	Attrib		attributes;
/* ***************************** */
 struct	Pad		*front;
 struct	Pad		*back;
	Rectangle	rect;
	Rectangle	srect;
	char		*name;
	Line		sentinel;
	long		selkey;
	short		lo;
	short		hi;
	short		ticks;
	short		tabs;
} Pad ;

typedef struct Selection {
	Pad	*pad;
	Line	*line;
} Selection;

typedef struct Entry {
	char	*text;
	void	(*action)();
	long	opand;
struct	Script	*script;
} Entry;

typedef struct Script{
	Entry	*(*generator)();
	void	(*limits)();
struct	Script	*more;
	Index	cindex;		/* bogus! bogus! bogus! */
	short	items;
	short	width;
	short	prevtop;
	short	prevhit;
} Script;

Entry *ScriptHit();

typedef enum Cover { CLEAR, PARTIAL, COMPLETE } Cover;

extern Selection Selected;	/* selected line 			*/
extern Pad *Current;		/* current pad				*/
extern Pad *HostObject;		/* global arg to HostAction(Index)	*/
extern Pad *HostParent;		/*  and its pad's object		*/
extern short Scrolly;		/* suggest middle for Paint()		*/

extern Point Zpoint;
extern Rectangle ZRectangle;
extern Pad *DirtyPad;

#define BIGMEMORY 1
#define NOVICEUSER 2
int Configuration;

Point dxordy();

/* buttons.c */
Entry *CarteEntry(Index, short);
void MOUSEServe(void);
void FlashBorder(Pad*);
/* cache.c */
void CacheOp(Protocol);
char *IndexToStr(Index);
Carte *IndexToCarte(Index);
/* host.c */
void FlushRemote(void);
int GetRemote(void);
void PutRemote(char);
void ToHost(Protocol, long);
void HostAction(Index*);
void HostNumeric(long);
void RCVServe(void);
void HelpString(void);
void ProtoErr(char*);
void ALARMServe(void);
/* lib.c */
void waitMOUSE(void);
char *itoa(int);
int dictorder(char*, char*);
long assertf(long,char*);
Point dxordy(Point);
Rectangle boundrect(Rectangle, Rectangle);
Rectangle scrollbar(Rectangle, int, int, int, int);
int rectinrect(Rectangle, Rectangle);
char *Alloc(int);
Rectangle moverect(Rectangle, Rectangle);
void Quit(void);
void panic(char*);
void dprint(int, char*, ...);
/* lineops.c */
void DoCut(Line*);
void CutLine(void);
void Sever(void);
/* master.c */
void ALARMServe(void);
/* pad.c */
int PadSized(Rectangle);
void DelLine(Line*);
Line *InsAbove(Line*, Line*);
Line *InsPos(Pad*, Line*);
void KBDServe(void);
void SetCurrent(Pad*);
void LineXOR(Line*);
void Dirty(Pad*);
void PadOp(Protocol op);
void PickOp(void);
Pad *PickPad(Point pt);
void DeletePick(void);
void DelLines(Pad*);
void DeletePad(Pad*);
void Select(Line*, Pad*);
void MakeCurrent(Pad*);
void Move(void);
void Reshape(void);
void PadClip(void);
void PadStart();
void InvertKBDrect(char*, char*);
void PaintKBD(void);
void LayerReshaped(void);
void FoldToggle(Attrib*);
Entry *FoldEntry(Attrib*);
/* paint.c */
void DoubleOutline(Bitmap*,Rectangle);
void HeavyBorder(Pad*);
int ClipPaint(Rectangle, Pad*);
void PadBlt(Bitmap*, Rectangle, Pad*);
void Paint(Pad*);
void LineReq(Pad*, long, long, int);
void CRequestLines(Pad*);
void RequestLines(Pad*);
void Pointing(void);
/* scripthit.c */
Entry *ScriptHit(Script*, int, RectList*);

extern Cursor Sweep;
extern Cursor Danger;
extern Cursor Bullseye;
extern Cursor NoMemory;
extern Cursor NoGCMemory;
extern Cursor Coffee;
extern Cursor HostBusy;
extern Cursor *Pcursor;

extern Index CIndex;

#define butts (mouse.buttons&07)
#define BUTT1 4
#define BUTT2 2
#define BUTT3 1
