#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef POLL
#include <poll.h>
#define INFTIM	-1
#endif

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

typedef unsigned char uchar;
extern int rcvmask;
extern int dpyfd;
extern Display *_dpy;

void recv();
void handleinput();
void sendnchars(int, uchar*);

void sendchar(char c){ sendnchars(1, (uchar*)&c); }

#ifdef POLL

#define FDSET(e,x,y)	(e).fd = (y), (e).events = (z)

void sendnchars(int n, uchar *p){
	int i;
	struct pollfd pfds[3];

	pfds[0].fd = dpyfd;
	pfds[0].events = POLLIN;
	if (rcvmask) {
		pfds[1].fd = 0;
		pfds[1].events = POLLIN;
	}
	pfds[2].fd = 1;
	pfds[2].events = POLLIN;
	while(n){
		i = write(1, p, n);
		if(i > 0){
			n -= i;
			p += i;
			continue;
		}
		if(i < 0 && errno == EWOULDBLOCK){
			do {
				while (XPending(_dpy))
					handleinput();
				poll(pfds, 3, INFTIM);
				if (rcvmask && (pfds[1].revents & POLLIN))
					recv();
				if (pfds[0].revents & POLLIN)
					handleinput();
			} while(!(pfds[2].revents & POLLIN));
		} else
			exit(1);
	}
}
#else
void sendnchars(int n, uchar *p){
	int i;
	int maxfd;
	fd_set rmask, wmask;

	FD_ZERO(&rmask);
	FD_ZERO(&wmask);
	maxfd = dpyfd + 1;
	while(n){
		i = write(1, p, n);
		if(i > 0){
			n -= i;
			p += i;
			continue;
		}
		if(i < 0 && errno == EWOULDBLOCK){
			do {
				while (XPending(_dpy))
					handleinput();
				FD_SET(dpyfd, &rmask);
				if (rcvmask)
					FD_SET(0, &rmask);
				FD_SET(1, &wmask);
				select(maxfd, &rmask, &wmask, 0, 0);
				if (rcvmask && FD_ISSET(0, &rmask))
					recv();
				if (FD_ISSET(dpyfd, &rmask))
					handleinput();
			} while(!FD_ISSET(1, &wmask));
		} else
			exit(1);
	}
}
#endif
