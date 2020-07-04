#define	gcnew(p, n)	((void)gcalloc((ulong)(n)*sizeof((p)[0]), (long **)&(p)))
#define	gcrenew(p, n)	((void)gcrealloc((char *)p, ((ulong)(n)*sizeof((p)[0]))))
#define	MAXFILES	256	/* 255 real files, 0 for unallocated */

typedef unsigned long ulong;
typedef unsigned long Posn;

typedef void                SIG_FUNC_TYP(int);
typedef SIG_FUNC_TYP        *SIG_TYP;

char *homedir;

char *gcalloc(ulong, long**);
char *gcrealloc(char*, ulong);
void shiftgcarena(ulong nl);
void gcfree(char *cp);
static void compact(void);
void Bcopy(char*, char*, char*, int);
void gcchk(void);
char *charstar();
char *jerqname();
long min(int, int);
void mesg(char*, char*);
void panic(char*);
void allocinit(void);
char *alloc(ulong);
void free(void*);
void allocerr(void);
void *malloc(unsigned);
void send(unsigned, int, int, unsigned, char*);
void rcv(void);
