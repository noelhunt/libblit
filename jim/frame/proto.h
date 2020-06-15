/* deltext.c */
void frdelete(Frame *t, int f_clr);
/* fralloc.c */
/* frameop.c */
int nexttab(Frame *t, int x);
int frameop(register Frame *t, register void (*op)(void), Point pt, register char *cp, register n);
void opdraw(Frame *t, Point p, Point q, register char *cp, register n);
int draw(Frame *t, Point p, char *s, int n);
void oprectf(Frame *t, Point p, Point q, char *str, int n);
int selectf(register Frame *t, int f);
void opnull(void);
int rXOR(Rectangle r);
/* instext.c */
int frinsert(register Frame *t, register String *s, register sn);
/* ptofchar.c */
Point startline(register Frame *t, register n);
int setcpl(register Frame *t, int first, int last);
void opcpl(Frame *t, Point p, Point q, char *cp, int n);
int cpl(register Frame *t, int posn, Point pt);
int scrollcpl(Frame *t, register first, register last, register n);
int lineno(register Frame *t, register y);
