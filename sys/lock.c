/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

extern unsigned long	ctr1000;	/* counts in 1000ths of second 0-INF	*/
int is_wait_higher_prio(int prio, int ldesc);
/*------------------------------------------------------------------------
 * lock  --  acquire a lock and assign it to the current process
 *------------------------------------------------------------------------
 */
int lock (int ldes1, int type, int priority)
{
	STATWORD ps;
	struct	pentry	*pptr;
	//kprintf("\n\t[LOCK.C: 21] currpid = '%s' asking for lock %d locks[ldes1].ltype = %d\n", proctab[currpid].pname, ldes1,locks[ldes1].ltype);

	disable(ps);
	if (isbadlock(ldes1) || locks[ldes1].lstate == DELETED || proctab[currpid].locktype[ldes1] != NONE) /* If the lock descriptor is out of bounds return SYSERR */
	{
		restore(ps);
		return(SYSERR);
	}

	proctab[currpid].plockwaitret = OK;

	if(locks[ldes1].ltype == NONE)
	{
		locks[ldes1].ltype = type;
		proctab[currpid].locktype[ldes1] = type;
		locks[ldes1].lprocs[currpid] = type;

		if(type == READ)
		{
			locks[ldes1].lreaders++;
		}
		//kprintf("\n\t[LOCK.C: 40] Lock %d given to '%s'number of readers = %d\n",ldes1,proctab[currpid].pname,locks[ldes1].lreaders);

	}

	else if(locks[ldes1].ltype == WRITE)
	{
		(pptr = &proctab[currpid])->pstate = PRWAIT;
		pptr->locktype[ldes1] = type;
		pptr->pintime = ctr1000;
		insert(currpid,locks[ldes1].lqhead,priority);
		//kprintf("\n\t[LOCK.C: 51] Lock %d not free, '%s' with prio %d put in queue\n",ldes1,proctab[currpid].pname,priority);
		resched();
	}

	else if(locks[ldes1].ltype == READ)
	{
		if(type == WRITE)
		{
			(pptr = &proctab[currpid])->pstate = PRWAIT;
			pptr->locktype[ldes1] = type;
			pptr->pintime = ctr1000;
			insert(currpid,locks[ldes1].lqhead,priority);
			//kprintf("\n\t[LOCK.C: 65] Lock %d not free, '%s' with prio %d put in queue\n",ldes1,proctab[currpid].pname,priority);
			resched();
		}
		else if(type == READ)
		{
			if(is_wait_higher_prio(priority,ldes1) == 1)
			{
				(pptr = &proctab[currpid])->pstate = PRWAIT;
				pptr->locktype[ldes1] = type;
				pptr->pintime = ctr1000;
				insert(currpid,locks[ldes1].lqhead,priority);
				//kprintf("\n\t[LOCK.C: 77] Lock %d not free, '%s' with prio %d put in queue\n",ldes1,proctab[currpid].pname,priority);
				resched();
			}
			else
			{
				locks[ldes1].ltype = type;
				locks[ldes1].lreaders++;
				proctab[currpid].locktype[ldes1] = type;
				locks[ldes1].lprocs[currpid] = type;
				//kprintf("\n\t[LOCK.C: 86] Lock %d given to '%s' number of readers = %d\n",ldes1,proctab[currpid].pname,locks[ldes1].lreaders);
			}
		}

	}

	restore(ps);
	return(proctab[currpid].plockwaitret);
}

/* To check if there is a writer with higher priority than 'prio' in the queue of the lock 'ldesc' */
int is_wait_higher_prio(int prio, int ldesc)
{
	int temp;

	temp = q[locks[ldesc].lqtail].qprev;
	/* If the list is empty or last process in the lock wait queue is a writer and it's priority is less than 'prio', return FALSE */
	if((temp!=locks[ldesc].lqhead) || (prio >= q[temp].qkey && proctab[temp].locktype[ldesc] == WRITE))
		return 0;
	else
	{
		while(temp != locks[ldesc].lqhead)
		{
			if(proctab[temp].locktype[ldesc] == WRITE && q[temp].qkey > prio)
				return 1;
			temp = q[temp].qprev;
		}
	}
	return 0;
}
