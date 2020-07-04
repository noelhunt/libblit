#include <stdio.h>

#include <signal.h>
#include <stdlib.h>
#include "univ.h"

int ALARM_CYCLE = 15;		/* 1/4 second for internal refresh */
int HOST_CYCLE = 4;		/*  1  second for host objects */
int quitok = 0;

void sigpipe_h(int s){ abort(); }

void ALARMServe(){
	static int cycle;
	void Cycle();

	if( own()&ALARM ){
		if( !(own()&RCV) ) Dirty((Pad*)0);
		if( ++cycle, (cycle%=HOST_CYCLE)==0 ) Cycle();
		alarm( ALARM_CYCLE );
	}
}

void main(int argc, char *argv[]){
	void PadStart();
        signal(SIGPIPE, sigpipe_h);
	request(KBD|MOUSE|SEND|RCV|ALARM);
	initdisplay(argc, argv);
	Configuration |= NOVICEUSER|BIGMEMORY;
	PadStart();
	cursswitch(&Coffee);
	sleep( 30 );
	PadClip();
	alarm( ALARM_CYCLE );
	for( ;; ){
		LayerReshaped();
		MOUSEServe();
		KBDServe();
		RCVServe();
		ALARMServe();
		wait(ALARM|RCV|MOUSE|KBD);
	}
}
