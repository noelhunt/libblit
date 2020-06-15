#define BSD_COMP
#include <sys/stropts.h>
#include <sys/ioctl.h>
#include <pads.h>
SRCFILE("remote.c")

void Remote::checkproto(int p)		{ if( get()!=p ) err(); }
void Remote::proto(int p)		{ put( p ); }

long Remote::rcvlong()			{ return (long)  shiftin( P_LONG  ); }
short Remote::rcvshort()		{ return (short) shiftin( P_SHORT ); }
unsigned char Remote::rcvuchar()	{ return (uchar) shiftin( P_UCHAR ); }

void Remote::sendlong(long  x)		{ shiftout( P_LONG, x );	 }
void Remote::sendshort(short x)		{ shiftout( P_SHORT, (long) x ); }
void Remote::senduchar(uchar x)		{ shiftout( P_UCHAR, (long) x ); }
void Remote::pktflush()			{ writesize = 0; pktend(); 	}

void Remote::pktstart(char c){
	const char *ProtoToString(Protocol);
	fprintf(stderr, "host -> %s\n", ProtoToString((Protocol)c));
	put(c);
}

void Remote::put(char c){
	writebuffer[pktsize++] = c;
	if (pktsize == sizeof(writebuffer))
		pktflush();
}

void Remote::sendobj(PadRcv *o){
	sendlong((long)o);
}

PadRcv *Remote::rcvobj(){
	PadRcv *obj = (PadRcv*)rcvlong();
	short oid = rcvshort();
	if (obj && obj->oid != oid)
		obj = 0;
	trace("0x%08x.rcvobj() obj.0x%08x", this, obj);
	return obj;
}

void Remote::err(const char *e){
	if( !e ) e = "Pads library: protocol error";
	PadsError(e);
}

#ifdef SAMTERM
Remote::Remote(int in, int out){
	fd[0] = in;
	fd[1] = out;
	pktsize = writesize = pktbase = 0;
}
#else
Remote::Remote(int opened){
	fd = opened;
	pktsize = writesize = pktbase = 0;
}
#endif
Remote::Remote(const char *dev){
	struct sgttyb tty;
#ifndef SAMTERM
	fd = open(dev, 2);
#ifdef TIOCGETP
	ioctl(fd, I_PUSH, "ttcompat");
	if( ioctl(fd, TIOCGETP, &tty) ) err("tty ioctl: GETP failed");
	tty.sg_flags = (tty.sg_flags|CBREAK|RAW) & ~ECHO;
	if( ioctl(fd, TIOCSETP, &tty) ) err("tty ioctl: SETP failed");
	if( ioctl(fd, TIOCEXCL, 0)  ) err("tty ioctl: EXCL failed");
#endif
#else
	fd[1] = open(dev, 2);
#ifdef TIOCGETP
	ioctl(fd[1], I_PUSH, "ttcompat");
	if( ioctl(fd[1], TIOCGETP, &tty) ) err("tty ioctl: GETP failed");
	tty.sg_flags = (tty.sg_flags|CBREAK|RAW) & ~ECHO;
	if( ioctl(fd[1], TIOCSETP, &tty) ) err("tty ioctl: SETP failed");
	if( ioctl(fd[1], TIOCEXCL, 0)  ) err("tty ioctl: EXCL failed");
#endif
#endif
	pktsize = writesize = 0;
}

void Remote::share(){
	trace( "%d.share()", this );
}	

long Remote::shiftin(int bytes){
	long shifter = 0;

	trace("0x%08x.shiftin(bytes.%s)", this, debug(bytes));
	checkproto( bytes );
	while( bytes-- ) shifter = (shifter<<8) + (get()&0xFF);
	return shifter;
}

void Remote::shiftout( int bytes, long shifter ){
	proto( bytes );
	do { put( (char)(shifter>>( (--bytes)*8 )) ); } while( bytes );
}

long BytesToTerm;
void Remote::pktend(){
#ifdef TAC
	if( pktsize<=0 ) err();
	if (pktbase + pktsize > writesize) {
#ifndef SAMTERM
		if (write(fd, (char*)writebuffer, pktbase) != pktbase)
#else
		if (write(fd[1], (char*)writebuffer, pktbase) != pktbase)
#endif
			abort();
		BytesToTerm += pktbase;
		pktbase = 0;
		writesize = sizeof(writebuffer);
	}
	pktbase += pktsize;
	pktsize = 0;
#else
	if (pktsize > writesize) {
#ifndef SAMTERM
		if (write(fd, (char*)writebuffer, pktsize) != pktsize)
#else
		if (write(fd[1], (char*)writebuffer, pktsize) != pktsize)
#endif
			abort();
		BytesToTerm += pktsize;
		pktsize = 0;
		writesize = sizeof(writebuffer);
	}
#endif
}

char *Remote::rcvstring( char *s0 ){
	register char *s = s0;
	register unsigned char len;

	checkproto( P_STRING );
	len = rcvuchar();
	if( !s0 ) s = s0 = new char [len+1];
	while( len-->0 ) *s++ = get();
	*s = '\0';
	return s0;
}

void Remote::sendstring(const char *s){
	int len;

	trace( "0x%08x.sendstring(%s)", this, s );
	proto( P_STRING );
	len =  strlen(s);
	if( len > 255 ) len = 255;
	senduchar( len );
	while( len-- ) put(*s++); 
}

#define BULKREAD

#include <errno.h>

long BytesFromTerm;
#ifndef BULKREAD
int Remote::get(){
	char c;
	trace("0x%08x.get pktsize.%d", this, pktsize);
	if (pktsize) {
		err();
		return 0;
	}
#ifndef SAMTERM
	while (read(fd, &c, 1) != 1) {
#else
	while (read(fd[0], &c, 1) != 1) {
#endif
		if (errno == EINTR)
			continue;
		err();
	}
	++BytesFromTerm;
	return c;
}
#else
int Remote::get(){
	static uchar buf[128];
	static int i, nleft = 0;

	if (pktsize) {
		err();
		return 0;
	}
	if( nleft <= 0 ){
again:
#ifndef SAMTERM
		if( (nleft = read(fd, (char *)buf, sizeof buf)) < 0 ){
#else
		if( (nleft = read(fd[0], (char *)buf, sizeof buf)) < 0 ){
#endif
			if(errno == EINTR)	/* why are we getting EINTR? */
				goto again;
			err(strerror(errno));
			return 0;
		}
		i = 0;
	}
	++BytesFromTerm, --nleft;
	return (int)buf[i++];
}
#endif
