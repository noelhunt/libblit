#include <pads.pri>

static void Proto(p) { PutRemote(p); }

static void CheckProto( p ) { if( GetRemote() != p ) ProtoErr(); }

static long ShiftIn( int bytes ){
	int shifter = 0;

	CheckProto( bytes );
	while( bytes-- ) shifter = (shifter<<8) + (GetRemote()&0xFF);
	return shifter;
}

long  RcvLong()  { return (int)   ShiftIn( P_LONG  ); }
#ifdef __x86_64
long  RcvVLong() { return (long)  ShiftIn( P_VLONG  ); }
#endif
short RcvShort() { return (short) ShiftIn( P_SHORT ); }
uchar RcvUChar() { return (uchar) ShiftIn( P_UCHAR ); }

static void ShiftOut( int bytes, int shifter ){
	Proto( bytes );
	do { PutRemote( (char)(shifter>>( (--bytes)*8 )) ); } while( bytes );
}

void SendLong(int x)    { ShiftOut( P_LONG,  (int) x ); }
#ifdef __x86_64
long  SendVLong(long x) { ShiftOut( P_VLONG, (long) x ); }
#endif
void SendShort(short x) { ShiftOut( P_SHORT, (int) x ); }
void SendUChar(uchar x) { ShiftOut( P_UCHAR, (int) x ); }

char *RcvString( char *s0 ){
	char *s = s0;
	uchar len;

	CheckProto( P_STRING );
	len = RcvUChar();
	if( !s0 ) s = s0 = new char [len+1];
	while( len-->0 ) *s++ = GetRemote();
	*s = '\0';
	return s0;
}

void SendString( char *s ){
	int len;

	Proto( P_STRING );
	len = strlen(s);
	if( len > 255 ) len = 255;
	SendUChar( len );
	while( len-- ) PutRemote(*s++); 
}
