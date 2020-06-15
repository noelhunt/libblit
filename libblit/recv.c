#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include "recv.h"

#define	min(x,y)	(((x) < (y)) ? (x) : (y))

typedef unsigned char	uchar;

struct Jrcvbuf Jrcvbuf;

void rcvbfill(uchar*,int);

void recv(){
	static uchar rbuf[1024];
	int i;

	i = min (1024, Jrcvbuf.size - Jrcvbuf.cnt);
	i = read(0, rbuf, i);
	if (i <= 0)
		exit(0);
	rcvbfill(rbuf, i);
}
