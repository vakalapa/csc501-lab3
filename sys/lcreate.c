//lcreate.c


#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
int lcreate(void)
{
	STATWORD ps;
	int lock;

	disable(ps);
	if((lock=newlock())==SYSERR){
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	return lock;
}

LOCAL int newlock()
{
	int lock;
	int i;

	for (i=0;i<NLOCKS;i++){
		lock = nextlock--;

		if(nextlock<0){
			nextlock=NLOCKS-1;
			lockiter++;
		}
		if(locks[i].lstate == LFREE||locks[i].lstate == DELETED){
			locks[i].lstate = LUSED;
			locks[i].ltype = NONE;
			return i;
		}

	}
	
	return SYSERR;

}


