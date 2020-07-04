#include <string.h>
#include <stdlib.h>
#include <blit.h>
#include "libgint.h"
#include "recv.h"
#ifdef POLL
#include <poll.h>
#define INFTIM	-1

#define POLLCLR(x)	(x).fd = -1
#define POLLSET(x,y,z)	(x).fd = y, (x).events = (z)
#endif

#define _STDIN 0

#include <stdio.h>

void recv();
int realtime();
uint alarmtime, alarmstart;

extern int rcvmask;

#ifdef POLL
int wait(int resource){
	int ret;
	ulong timeout;
	struct pollfd pfds[2];
	uint diff;

	pfds[0].fd = dpyfd;
	pfds[0].events = POLLIN;
	for(;;){
		pfds[1].fd = -1;
		if (alarmtime) {
			diff = realtime() - alarmstart;
			if (diff >= alarmtime) {
				alarmtime = 0;
				P->state |= ALARM;
			} else {
				alarmstart += diff;
				alarmtime -= diff;
			}
		}
		if(P->state & resource)
			break;
		if(XPending(_dpy))
			goto xin;
		if (!Jrcvbuf.blocked & rcvmask) {
			pfds[1].fd = 0;
			pfds[1].events = POLLIN;
		}
		if (resource & CPU) {
			ret = poll(pfds, 2, 0);
			if (ret == 0)
				break;
		} else if (alarmtime) {
			timeout = ((alarmtime/60) * 1000)+((alarmtime%60) * 16);
			ret = poll(pfds, 2, timeout);
			if (ret == 0) {
				alarmtime = 0;
				P->state |= ALARM;
				continue;
			}
		} else {
			ret = poll(pfds, 2, INFTIM);
		}
		if (ret < 0)
			continue;
		if(!Jrcvbuf.blocked & rcvmask){
			if(pfds[1].revents & POLLHUP)
				exit(0);
			if(pfds[1].revents & POLLIN)
				recv();
		}
		if(pfds[0].revents & POLLIN){
xin:
			handleinput();
			if(resource & MOUSE) /* We always have the mouse */
				break;
		}
	}
	return resource & (P->state|MOUSE);

}
#else
int wait(int resource){
	int maxfd, ret;
	fd_set smask;
	uint diff;
	struct timeval tv;
	maxfd = dpyfd + 1;
	FD_ZERO(&smask);
	for(;;){
		if (alarmtime) {
			diff = realtime() - alarmstart;
			if (diff >= alarmtime) {
				alarmtime = 0;
				P->state |= ALARM;
			} else {
				alarmstart += diff;
				alarmtime -= diff;
			}
		}
		if(P->state & resource)
			break;
		if(XPending(_dpy))
			goto xin;
		FD_SET(dpyfd, &smask);
		if (!Jrcvbuf.blocked & rcvmask)
			FD_SET(0, &smask);
		if (resource & CPU) {
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			ret = select(maxfd, &smask, 0, 0, &tv);
			if (ret == 0)
				break;
		} else if (alarmtime) {
			tv.tv_sec = alarmtime/60;
			tv.tv_usec = (alarmtime%60) * 16666;
			ret = select(maxfd, &smask, 0, 0, &tv);
			if (ret == 0) {
				alarmtime = 0;
				P->state |= ALARM;
				continue;
			}
		} else {
			ret = select(maxfd, &smask, 0, 0, 0);
		}
		if (ret < 0)
			continue;
		if(rcvmask && FD_ISSET(0, &smask))
			recv();
		if(FD_ISSET(dpyfd, &smask)){
xin:
			handleinput();
			if(resource & MOUSE) /* We always have the mouse */
				break;
		}
	}
	return resource & (P->state|MOUSE);

}
#endif

void nap(int n){
	wait(MOUSE);
}

void alarm(int ticks){
	alarmtime = ticks;
	alarmstart = realtime();
	P->state &= ~ALARM;
}

void sleep(int ticks){
	int maxfd, ret;
	fd_set smask;
	unsigned diff = 0, tleft;
	struct timeval tv;
	unsigned start = realtime();

	maxfd = dpyfd + 1;
	for(tleft = ticks; diff < tleft; ) {
		tleft -= diff;
		if(XPending(_dpy))
			goto Event;
Again:
		FD_ZERO(&smask);
		FD_SET(dpyfd, &smask);
		if (!Jrcvbuf.blocked && rcvmask)
			FD_SET(0, &smask);
		tv.tv_sec = tleft / 60;
		tv.tv_usec = (tleft % 60) * 16666;
		ret = select(maxfd, &smask, 0, 0, &tv);
		if (ret == 0)
			break;
		if (ret == -1)
			goto Again;
		if(rcvmask && FD_ISSET(0, &smask))
			recv();
		if(FD_ISSET(dpyfd, &smask)){
Event:
			handleinput();
		}
		diff = realtime() - start;
		start += diff;
	}
	if (alarmtime) {
		if (ticks >= alarmtime) {
			alarmtime = 0;
			P->state |= ALARM;
		} else {
			alarmstart += ticks;
			alarmtime -= ticks;
		}
	}
}

int realtime(){
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 60 + (tv.tv_usec * 60 / 1000000);
}
