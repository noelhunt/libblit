#include <stdio.h>

#include "univ.h"
#define REMOTEBUFFER 64
unsigned char RemoteBuffer[REMOTEBUFFER];
int RemoteIndex;

void sendnchars(int,uchar*);
int rcvchar();

void FlushRemote(){
	if( RemoteIndex ){
		sendnchars(RemoteIndex, RemoteBuffer);
		RemoteIndex = 0;
	}
}

int GetRemote(){
	int c;
	while( (c = rcvchar() ) == -1 ){
		cursswitch(&Coffee);
		wait(RCV);
		cursswitch(Pcursor);
	}
	return c;
}

void PutRemote( char c ){
	if( RemoteIndex >= REMOTEBUFFER ) FlushRemote();
	RemoteBuffer[RemoteIndex++] = c;
}

void ToHost( Protocol p, long l ){
	extern Pad *Current;

	PutRemote( p );
	SendLong( HostParent->object );
	SendShort( HostParent->oid );
	SendLong( HostObject->object );
	SendShort( HostObject->oid );
	SendLong( l );
	FlushRemote();
}
	
void HostAction( Index *i ){	/* <text:action:opand> stored at host	*/
	ToHost( P_ACTION, *i );
}

void HostNumeric( long n ){	/* always in short range ! */
	ToHost( P_NUMERIC, (short) n );
}

void RCVServe(){	/* poll the host */
#ifdef LIBXDMD
	Texture *t = Pcursor;
#else
	Cursor *t = Pcursor;
#endif
	int op;

	FlushRemote();
Again:
	if( P->state & RCV ){
		op = GetRemote();
		switch( op&0xF0 ){
		case (int)P_VERSION&0xF0:
			if( RcvLong() != PADS_VERSION )
			   ProtoErr( "host/term versions differ - regenerate. " );
			break;
		case (int)P_PICK&0xF0:
			PickOp();
			break;
		case (int)P_HOSTSTATE&0xF0:
			t = (op==(int)P_BUSY) ? &HostBusy : 0;
			goto Again;
		case (int)P_PADDEF&0xF0:
		case (int)P_PADOP&0xF0:	
			PadOp(op);
			break;
		case (int)P_CACHEOP&0xF0:
			CacheOp(op);
			break;
		case (int)P_HELPSTR&0xF0:
			HelpString();
			break;
		default:
			ProtoErr( "RCVServe(): " );
		}
	}
	if( t != Pcursor ) cursswitch( Pcursor = t );
}

void HelpString(){
	char s[256];

	RcvString(s);
	InvertKBDrect(s,"");
}

void ProtoErr(char *s){
	extern char KBDStr[];
	register char *k = KBDStr, *p = "protocol: ";

	while( *s ) *k++ = *s++;
	while( *p ) *k++ = *p++;
	for(;;){
		PaintKBD();
		if( k<&KBDStr[64] ) *k++ = GetRemote()&0x7F;
		sleep(30);
	}
}
