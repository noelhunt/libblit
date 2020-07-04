#include "univ.h"
#include "stdio.h"

void Proto( int p ){
	PutRemote(p);
}

static void CheckProto( int p ){
	char buf[64];
	int q;
	if( (q = GetRemote()) != p ){
		sprintf(buf, "CheckProto: expecting %x ", p);
		ProtoErr(buf);
	}
}

static int ShiftIn( int bytes ){
	int shifter = 0;
	CheckProto( bytes );
	while( bytes-- ) shifter = (shifter<<8) + (GetRemote()&0xFF);
	return shifter;
}

int RcvLong()	  { return (int)    ShiftIn( P_LONG  ); }
#ifdef __x86_64
long RcvVLong()	  { return (long)   ShiftIn( P_VLONG  ); }
#endif
ushort RcvShort() { return (ushort) ShiftIn( P_SHORT ); }
uchar RcvUChar()  { return (uchar)  ShiftIn( P_UCHAR ); }

static void ShiftOut( int bytes, int shifter ){
	Proto( bytes );
	do PutRemote( (char)(shifter>>( (--bytes)*8 )) ); while( bytes );
}

void SendLong(int x)    { ShiftOut( P_LONG,  (int) x ); }
#ifdef __x86_64
void SendVLong(long x)  { ShiftOut( P_VLONG, (long) x ); }
#endif
void SendShort(short x) { ShiftOut( P_SHORT, (int) x ); }
void SendUChar(uchar x) { ShiftOut( P_UCHAR, (int) x ); }

void SendObj(long obj){
#ifndef __x86_64
	SendLong( obj );
#else
	SendVLong( obj );
#endif
}

char *RcvString( char *s ){
	uchar len;

	assertf( (int) s, "RcvString"  );
	CheckProto( P_STRING );
	len = RcvUChar();
	while( len-->0 ) *s++ = GetRemote();
	*s = '\0';
	return s;
}

void SendString( char *s ){
	Proto( P_STRING );
	SendUChar( strlen(s) );
	while( *s ) PutRemote(*s++); 
}
