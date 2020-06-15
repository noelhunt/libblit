#include <ctype.h>
#include <pads.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <new.h>
SRCFILE("term.c")

ItemCache  *ICache;
CarteCache *CCache;

static void mallocerr(){
	PadsError("Pads library: malloc failed");
	abort();
}

void Pick( const char *s, Action a, long o ){
	Index ix;

	trace("Pick(%d,%d)", a, o);
	ix = ICache->place(Item(s, a, o));
	R->pktstart( P_PICK );
	R->sendshort( ix.sht() );
	R->pktend();
}

const char *padsterm = "padsterm";

const char *loadterm(int argc, char **argv, const char *cmd){
	int targc = 0;
	char *targv[20];
	int ph2t[2], pt2h[2];
	char err[128];
	char *StrDup(const char*);
	targv[targc++] = StrDup(cmd);
	while( argc > 1 ){
		targv[targc++] = argv[argc];
               --argc, argv++;
        }
	targv[targc] = 0;
	if(pipe(ph2t)==-1 || pipe(pt2h)==-1){
		sprintf(err, "loadterm: pipe: %s", strerror(errno));
		return strdup(err);
	}
	switch(fork()){
	case 0:
		dup2(ph2t[0], 0);
		dup2(pt2h[1], 1);
		close(ph2t[0]);
		close(ph2t[1]);
		close(pt2h[0]);
		close(pt2h[1]);
		execvp(padsterm, targv);
		fprintf(stderr, "can't exec: ");
		perror(padsterm);
		exit(127);
	case -1:
		sprintf(err, "can't fork padsterm: %s", strerror(errno));
		return strdup(err);
	}
	dup2(pt2h[0], 0);
	dup2(ph2t[1], 1);
	close(ph2t[0]);
	close(ph2t[1]);
	close(pt2h[0]);
	close(pt2h[1]);

	return 0;
}

void sigpipe_h(int s){ abort(); }

#ifdef SAMTERM
const char *PadsInit(const char *loadcmd){
	int afildes[2], bfildes[2], pid;
	if((pipe(afildes)==-1)||(pipe(bfildes)==-1))
		return "can't open pipe\n";
	if((pid=fork())==0){
		close(0);
		dup(afildes[0]);
		close(1);
		dup(bfildes[1]);
		execlp(loadcmd, "pads", (char *)0);
		exit(1);
	}
	if(pid==-1)
		return "terminal fork failed";
	sleep(3);	/* for dbx: allow time for child to get out */
	close(0);
	dup(bfildes[0]);
	close(1);
	dup(afildes[1]);
	R = new Remote(0,1);
	R->pktstart(P_VERSION); R->sendlong(PADS_VERSION); R->pktend();
	R->pktstart(P_BUSY); R->pktend();
	ICache = new ItemCache;
	CCache = new CarteCache;
	return 0;
}
#else
const char *PadsInit(const char *loadcmd){
	pid_t pid;
#ifdef DELAY
	struct timespec rqtp;
#endif
	int afildes[2];
	if( pipe(afildes)==-1 )
		return "can't open pipe\n";
	if( (pid=fork())==0 ){
		dup2(afildes[0], 0);
		dup2(afildes[0], 1);
		close(afildes[0]);
		close(afildes[1]);
		execlp(loadcmd, "pads", (char *)0);
		perror(loadcmd);
		exit(1);
	}
	if(pid==-1)
		return "terminal fork failed";
	close(afildes[0]);
#ifdef DELAY
	rqtp.tv_sec = 1; rqtp.tv_nsec = 50000;
	nanosleep(&rqtp, 0);
#endif
//	signal(SIGPIPE, sigpipe_h);
	R = new Remote(afildes[1]);
	R->pktstart(P_VERSION); R->sendlong(PADS_VERSION); R->pktend();
	R->pktstart(P_BUSY); R->pktend();
	ICache = new ItemCache;
	CCache = new CarteCache;
#ifdef NOSTDERR
	close(2);
#endif
	return 0;
}
#endif

const char *PadsTermInit(int argc, char **argv, char *machine){
	return "not implemented.";
}

void PadsRemInit(){
	R = new Remote(0);
	set_new_handler(mallocerr);
	R->pktstart(P_VERSION); R->sendlong(PADS_VERSION); R->pktend();
	R->pktstart(P_BUSY); R->pktend();
	ICache = new ItemCache;
	CCache = new CarteCache;
}

char *TapTo;
void WireTap(PRINTF_ARGS){
	static int fd = -1;
	static long t0;
	struct stat s;
	long t;
	va_list ap;
	va_start(ap, fmt);

	if( !TapTo ) return;
	t = time(0);
	if( ::stat("/usr/tmp/.logpads", &s) || ctime(&t)[23]!='6' )
		goto BailOut;
	if( t0 )
		t -= t0;
	else
		t0 = t;
	char buf[256];
	sprintf(buf, "%x:", t);
	vsprintf(buf+strlen(buf), fmt, ap);
	va_end(ap);
	if( fd < 0 ){
		if( ::stat(TapTo, &s) )
			creat(TapTo, 0777); 
		fd = open(TapTo, 1);
	}
#define PILOGSIZE 32000
	if( fd<0
	 || fstat(fd, &s)
	 || s.st_size > PILOGSIZE )
			goto BailOut;
	lseek(fd, s.st_size, 0);
	write(fd, buf, strlen(buf));
	return;
BailOut:
	TapTo = 0;
}

int BothValid(PadRcv *p, PadRcv *o){
	return p && o;
}

void TermAction(PadRcv *parent, PadRcv *obj, int pick){
	Item *item;
	Index ix((int)R->rcvlong());

	trace( "TermAction(%d,%d,%d)", parent, obj, pick );
	if( ix.null() ) return;
	item = ICache->take(ix);
	if( !BothValid(parent,obj)
	 || (pick && !obj->accept(item->action)) )
		return;
	if( item->action ) (obj->*item->action)(item->opand, 0, 0);
}

const char *DoKbd(PadRcv *obj, char *buf){
//	WireTap("%x->%x(%x) %s\n", obj, &obj->kbd, strlen(buf), buf);
	const char *e = obj->kbd(buf);
	if( e ) PadsWarn("%s", e);
	return e;
}

void Shell(){
	char cmd[256];
	R->rcvstring(cmd);
	FILE *fp = Popen(cmd, "w");
	for( long lines = R->rcvlong(); lines>0; --lines ){
		char data[256];
		if( fp ) fprintf(fp, "%s\n", R->rcvstring(data));
	}
	if( !fp ){
		PadsWarn("cannot write to pipe");
		return;
	}
	int x = Pclose(fp);
	if( x ) PadsWarn( "exit(%d): %s", x, cmd );
}

void ShKbd(PadRcv *obj, char *cmd){
	trace( "ShKbd(%d,%s)", obj, cmd );
	FILE *fp = Popen(cmd, "r");
	if( !fp ){
		PadsWarn("cannot read from pipe");
		return;
	}
	char buf[256];
	while( fgets(buf, sizeof buf, fp) ){
		buf[strlen(buf)-1] = 0;
		if( DoKbd(obj, buf) ) break;
	}
	int x = Pclose(fp);
	if( x ) PadsWarn( "exit(%d): %s", x, cmd );
}

void Kbd(PadRcv *parent, PadRcv *obj){
	char buf[256];
	R->rcvstring(buf);
	trace( "Kbd %d %s", obj, buf );
	if( !BothValid(parent,obj) ) return;
	if( !strcmp( buf, "?" ) ){
		const char *h = obj->help();
		PadsWarn( "%s", (h && *h) ? h : "error: null help string" );
	} else if( buf[0] == '<' ){
		ShKbd(obj, buf+1);
	} else
		DoKbd(obj, buf);
}

const char *ProtoToString(Protocol);

void TermServe(){
	Protocol p;
	long n, to, pick = 0;

	R->pktstart(P_IDLE); R->pktflush();
fprintf(stderr, "TermServe() sent P_IDLE\n");
	p = (Protocol) R->get();
	if( p == P_PICK ) {
		pick = 1;
		p = (Protocol) R->get();
	}
	PadRcv *par = R->rcvobj();
	PadRcv *obj = R->rcvobj();
fprintf(stderr, "TermServe() got 0x%08x 0x%08x %s\n",par,obj,ProtoToString(p));
	if( p != P_CYCLE ){ R->pktstart(P_BUSY); R->pktflush(); }
	trace("TermServe() p.%X", p&0xFF);
	switch( (int) p ){
		case P_ACTION:
			TermAction(par, obj, (int)pick);
			break;
		case P_KBDSTR:
			Kbd(par, obj);
			break;
		case P_SHELL:
			Shell();
			break;
		case P_NUMERIC:
		case P_CYCLE:
		case P_USERCLOSE:
		case P_USERCUT:
			n = R->rcvlong();
			if( !BothValid(par,obj) ) return;
			switch( (int) p ){
			case P_NUMERIC:
				obj->numeric(n);	break;
			case P_CYCLE:
				obj->cycle();		break;
			case P_USERCLOSE:
				obj->userclose();	break;
			case P_USERCUT	:
				obj->usercut();		break;
			default: R->err();
			}
			break;
		case P_LINEREQ:
			n = R->rcvlong();
			to = R->rcvlong();
			if( !BothValid(par,obj) ) return;
			while( n <= to )
				obj->linereq((long) n++, 0);
			break;
		case P_QUIT:
			exit(0);
			break;
		default:
			R->err();
	}
}

const char *ProtoToString(Protocol p){
	char buf[32];
	switch( p ){
	default: return sf("P_NONE.0x%02X",p&0xFF);
	case P_UCHAR: return "P_UCHAR";
	case P_SHORT: return "P_SHORT";
	case P_LONG: return "P_LONG";
	case P_CACHEOP: return "P_CACHEOP";
	case P_I_DEFINE: return "P_I_DEFINE";
	case P_I_CACHE: return "P_I_CACHE";
	case P_C_DEFINE: return "P_C_DEFINE";
	case P_C_CACHE: return "P_C_CACHE";
	case P_STRING: return "P_STRING";
	case P_INDEX: return "P_INDEX";
	case P_PADDEF: return "P_PADDEF";
	case P_ATTRIBUTE: return "P_ATTRIBUTE";
	case P_BANNER: return "P_BANNER";
	case P_CARTE: return "P_CARTE";
	case P_LINES: return "P_LINES";
	case P_NAME: return "P_NAME";
	case P_TABS: return "P_TABS";
	case P_HELPCARTE: return "P_HELPCARTE";
	case P_PADOP: return "P_PADOP";
	case P_ACTION: return "P_ACTION";
	case P_ALARM: return "P_ALARM";
	case P_CLEAR: return "P_CLEAR";
	case P_CYCLE: return "P_CYCLE";
	case P_DELETE: return "P_DELETE";
	case P_KBDSTR: return "P_KBDSTR";
	case P_LINE: return "P_LINE";
	case P_LINEREQ: return "P_LINEREQ";
	case P_MAKECURRENT: return "P_MAKECURRENT";
	case P_MAKEGAP: return "P_MAKEGAP";
	case P_NEXTLINE: return "P_NEXTLINE";
	case P_NUMERIC: return "P_NUMERIC";
	case P_USERCLOSE: return "P_USERCLOSE";
	case P_CREATELINE: return "P_CREATELINE";
	case P_REMOVELINE: return "P_REMOVELINE";
	case P_HOSTSTATE: return "P_HOSTSTATE";
	case P_BUSY: return "P_BUSY";
	case P_IDLE: return "P_IDLE";
	case P_USERCUT: return "P_USERCUT";
	case P_PICK: return "P_PICK";
	case P_HELPSTR: return "P_HELPSTR";
	case P_SHELL: return "P_SHELL";
	case P_VERSION: return "P_VERSION";
	case P_ERROR: return "P_ERROR";
	case P_QUIT: return "P_QUIT";
	}
}

void PadsServe(long n){
	trace("PadsServe( n.%d )", n);
	if( n ){
		while( n-->0 ) TermServe();
	} else {
		for( ;; )  TermServe();
	}
}

void PadsWarn(const char *fmt, ...){
	va_list av;
	char t[256];
	va_start(av, fmt);
	vsprintf(t, fmt, av);
	va_end(av);
	R->pktstart( P_HELPSTR );
	R->sendstring( t );
	R->pktend();
}

void PadsError(const char *fmt, ...){
	va_list av;
	char t[256];
	va_start(av, fmt);
	vsprintf(t, fmt, av);
	va_end(av);
	R->pktstart( P_ERROR );
	R->sendstring( t );
	R->pktflush();
	exit(1);
}

void PadsQuit(){
	R->pktstart( P_QUIT );
	R->pktflush();
	exit(0);
}
