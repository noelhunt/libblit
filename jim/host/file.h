#define	NBLOCK		512
#define	BLOCKSIZE	(4096)

/*
 * Descriptor pointing to a disk block
 */
typedef struct{
	short	nbytes;	/* number of bytes used in this block */
	short	bnum;	/* which disk block it is */
}Block;
/*
 * Strings all live in a common buffer and are nicely garbage compacted.
 */
typedef struct{
	char	*s;	/* pointer to string */
	ulong	n;	/* number used; s[n]==0 */
	ulong	size;	/* size of allocated area */
}String;
String *bldstring();	/* (temporary String)-building function */
#define	DUBIOUS	1L	/* An unlikely date; means we don't know origin of buffer */
/*
 * A File is a local buffer for the in-core block
 * and an array of block pointers.  The order of the block pointers
 * in the file structure determines the order of the true data,
 * as opposed to the order of the bits in the file (c.f. ed temp files *).
 */
typedef struct File{
	String	name;		/* name of associated real file, "" if none */
	String	str;		/* storage for in-core data */
	long	date;		/* date file was read in */
	long	origin;		/* file location of first char on screen */
	long	selloc;		/* start location of selected text */
	long	nsel;		/* number of chars selected */
	int	curblk;		/* block associated with File.str */
	short	id;		/* id number for communication */
	char	changed;	/* changed since last write */
	short	nblocks;	/* number of blocks in File.block */
	short	nalloc;		/* number of blocks allocated */
	Block	*block;		/* array of block pointers */
	struct File *next;	/* list chain */
}File;
File *file;
String buffer;		/* Place to save squirreled away text */
String transmit;	/* String to send to Jerq */

File	*Fcreat();
File	*Fnew();
File	*Fload();
File	*Fileid();
long	length(), Fforwnl(), Fbacknl(), Fcountnl();
#define	YMAX	1024	/* as in jerq.h */
#define	DIAG	file
#define	TRUE	1
#define	FALSE	0
long	loc1, loc2;	/* location of searched-for string */
char	*strcpy();
int	fileschanged;

/* string.c */
void strinit(String *p);
void strfree(String *p);
String *bldstring(char *s);
char *charstar(String *p);
void strzero(String *p);
void strdup(String *p, char *s);
void straddc(String *p, char c);
void strinsure(String *p, ulong n);
void strinsert(String *p, String *q, Posn p0);
void strdelete(String *p, Posn p1, Posn p2);

/* file.c */
File *Fileid(int id);
File *Fnew(void);
File *Fcreat(register File *f, char *s);
File *Fload(register File *f);
int Fread(register File *f, long posn, String *str, int setdate, int r);
int Fwrite(register File *f, register String *fname, int fd);
int Fwritepart(register File *f, long posn, long nbytes, int fd);
int samefile(String *f1, String *f2);
int Fclose(register File *f);
int Ffree(register File *f);
int Freset(register File *f);
int toolarge(register File *f);
int Read(char *s, int fd, char *a, int n);
int Write(char *s, int fd, char *a, ulong n);
int Fgetblock(register File *f, int n);
int makeblock(register File *f, int n);
int Fputblock(register File *f);
int splitblock(register File *f, long n);
int Fseek(register File *f, register long m);
int Finstext(register File *f, register long m, register String *s);
int Fdeltext(register File *f, register long m, register long n);
int Fsave(register File *f, register String *b, register long m, register long n);
int Forigin(register File *f, register long posn);
long Fcountnl(register File *f, register long charno);
long Fforwnl(register File *f, register long posn, register int nlines);
long Fbacknl(register File *f, register long posn, register int nlines);
long min(register long a, register long b);
long length(register File *f);
