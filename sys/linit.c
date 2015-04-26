// void linit(void)
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <stdio.h>
#include <lock.h>


struct lentry locks[NLOCKS];
int nextlock;
int lockiter;

void linit(void) {

    struct lentry *lptr;
    int i;

    nextlock = NLOCKS - 1;
    lockiter = 0;

    for (i = 0; i < NLOCKS; i++){
		lptr = &locks[i];
        lptr->lstate = LFREE;
		lptr->lreaders=0;
		lptr->lwriters=0;
        lptr->lqtail = 1  + (lptr->lqhead = newqueue()); 
		lptr->liter = 0;	
    }

}

