/* frame.c */
int curse(register Frame *f);
int cut(register Text *t, int save, int f_clr);
int type(register Text *t);
int nback(Frame *f, int c);
int notin(register c, register char *s);
int newlines(register String *s);
int senddiag(void);
int setsel(register Text *t, register n);
int workintext(register Text *t);
int drawtext(register Text *t);
int dodraw(register Text *t);
int ontop(register Text *t);
int obscured(register Text *t);
int buttons(int updown);
/* main.c */
int main(void);
int sendsnarf(void);
int panic(char *s);
int closeall(void);
int txinit(register Text *t);
int txfree(register Text *t);
int close(register Text *t);
int init(void);
int tellseek(Text *t, int y);
int usualtest(void);
int loadfile(register Text *t, register posn, int n);
int urequest(int f);
int waitunix(register *flag);
int message(void);
int integer(register char *s);
int mesg(register char *s, int sendit);
int sendstr(Text *t, int op, int posn, register n, register char *d);
int scrolltest(void);
int scroll(register Text *t, register nlines);
char *data2(int n);
int Send(register op, register posn, register n, register char *s);
/* menu.c */
int keepsearch(int which);
Text *textoffile(register unsigned f);
int setchar(unsigned f, int n, int c);
int delmenu(register f);
int insmenu(register f, register char *s);
int strcmp(register char *s, register char *t);
int setname(register f, register char *s);
int modified(register Text *t, int mod);
char *gnamegen(int i);
int menughit(Text *t, register unsigned hit, int remote);
/* msgs.c */
int send(unsigned f, int op, int posn, unsigned n, register char *d);
int rcv(void);
/* string.c */
int strzero(String *p);
int straddc(register String *p, int c);
int snarf(register String *p, int i, int j, register String *q);
