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
void Strinit(String *p);
void Strfree(String *p);
String *bldstring(char *s);
char *charstar(String *p);
void Strzero(String *p);
void Strdup(String *p, char *s);
void Straddc(String *p, char c);
void Strinsure(String *p, ulong n);
void Strinsert(String *p, String *q, Posn p0);
void Strdelete(String *p, Posn p1, Posn p2);

/* file.c */
File *Fileid(int id);
File *Fnew(void);
File *Fcreat(File *f, char *s);
File *Fload(File *f);
int Fread(File *f, long posn, String *str, int setdate, int r);
void Fwrite(File *f, String *fname, int fd);
int Fwritepart(File *f, long posn, long nbytes, int fd);
int samefile(String *f1, String *f2);
void Fclose(File *f);
void Ffree(File *f);
void Freset(File *f);
void toolarge(File *f);
void Read(char *s, int fd, char *a, ulong n);
void Write(char *s, int fd, char *a, ulong n);
void Fgetblock(File *f, int n);
static void seek(int b);
void makeblock(File *f, int n);
void Fputblock(File *f);
int splitblock(File *f, long n);
int Fseek(File *f, long m);
void Finstext(File *f, long m, String *s);
void Fdeltext(File *f, long m, long n);
void Fsave(File *f, String *b, long m, long n);
int Forigin(File *f, long posn);
long Fcountnl(File *f, long charno);
long Fforwnl(File *f, long posn, int nlines);
long Fbacknl(File *f, long posn, int nlines);
long min(int a, int b);
long length(File *f);
void error(char*, char*);
void modified(File*);
