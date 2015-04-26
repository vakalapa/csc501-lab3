#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * ldelete  --  delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */
int ldelete (int lockdescriptor)
{

STATWORD ps;
int	pid;
int temp,temp1;

struct lentry *lptr;
disable(ps);	

if (isbadlock(lockdescriptor) || locks[lockdescriptor].lstate==DELETED || locks[lockdescriptor].lstate==LFREE) {	
	restore(ps);	
	return(SYSERR);	
	}	

lptr = &locks[lockdescriptor];
lptr->lstate = DELETED;	
lptr->ltype = NONE;	
lptr->lreaders= 0;	

if (nonempty(lptr->lqhead)) {	
	while( (pid=getfirst(lptr->lqhead)) != EMPTY)		 
		{		   
		proctab[pid].plockwaitret = DELETED;		
		proctab[pid].locktype[lockdescriptor] = DELETED;	
		dequeue(pid);		  
		ready(pid,RESCHNO);	

		//kprintf("\n\t\t[LDELETE.C: 37] Proc '%s' deleted from waitqueue of Lock %d\n",proctab[pid].pname,lockdescriptor);		
		}		
		resched();
	}	


for(pid=1;pid<NPROC;pid++)	{	
	if(locks[lockdescriptor].holders[pid] == READ || locks[lockdescriptor].holders[pid] == WRITE)		
		{		
			locks[lockdescriptor].holders[pid] = DELETED;	
		}	
	}	

restore(ps);	
return(OK);


}

