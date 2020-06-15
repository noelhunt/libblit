#include <strings.h>
#define PADS_TERM
#include <pads.h>

int GetRemote();
void PutRemote(char);

static void Proto(int p)
{
	PutRemote(p);
}

static void CheckProto( int p ){
	void ProtoErr(char*);
	if( GetRemote() != p ) ProtoErr("");
}

static long ShiftIn( int bytes ){
	register long shifter = 0;

	CheckProto( bytes );
	while( bytes-- ) shifter = (shifter<<8) + (GetRemote()&0xFF);
	return shifter;
}

long  RcvLong()  { return (long)  ShiftIn( P_LONG  ); }
ushort RcvShort() { return (ushort) ShiftIn( P_SHORT ); }
unsigned char RcvUChar() { return (unsigned char) ShiftIn( P_UCHAR ); }

static void ShiftOut( int bytes, long shifter ){
	Proto( bytes );
	do PutRemote( (char)(shifter>>( (--bytes)*8 )) ); while( bytes );
}

void SendLong(x)  long  x; { ShiftOut( P_LONG,  (long) x ); }
void SendShort(x) short x; { ShiftOut( P_SHORT, (long) x ); }
void SendUChar(x) unsigned char x; { ShiftOut( P_UCHAR, (long) x ); }

char *RcvString( char *s ){
	unsigned char len;
	long assertf(long,char*);

	assertf( (long) s, "RcvString" );
	CheckProto( P_STRING );
	len = RcvUChar();
	while( len-->0 ) *s++ = GetRemote();
	*s = '\0';
	return s;
}

void SendString(s)
register char *s;
{
	Proto( P_STRING );
	SendUChar( strlen(s) );
	while( *s ) PutRemote(*s++); 
}
