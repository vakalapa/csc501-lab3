/* releaseall.c - releaseall, release */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

int release(int ldesc);
int get_next(int ldesc, int *max_w_prio);
void remove_other_reads(int ldesc, int border_index, int max_prio);

int releaseall (numlocks, ldes1)
int	numlocks;			/* number of locks to be released */
long	ldes1;
{
	STATWORD ps;
	int ret = OK;
	int ltemp;
	int i;
	disable(ps);

	for(i=0;i<numlocks;i++)
	{
		ltemp = (int)*((&ldes1) + i);
		ret = release(ltemp);
	}
	resched();
	restore(ps);
	return ret;
}

int release(int ldesc)
{

	struct lentry *lptr;
	int take_next = 0;
	int next = 0, max_w_prio = 0;
	int border_index;
	lptr = &locks[ldesc];

	//kprintf("\n\t[RELEASEALL.C: 44] Release of Lock %d by proc %s lptr->ltype =%d\n lptr->lprocs[%s] = %d number of readers = %d\n",ldesc,proctab[currpid].pname,lptr->ltype,proctab[currpid].pname,lptr->lprocs[currpid],lptr->lreaders);

	/* return SYSERR if */
	/* 1. ldesc is bad     2. the lock is deleted            3. current process doesn't hold the lock  4. the lock is already released */
	if(isbadlock(ldesc) || locks[ldesc].lstate == DELETED || lptr->lprocs[currpid] == NONE || lptr->ltype == NONE)
	{
		//kprintf("\n\t[RELEASEALL.C: 48] SYSERR locks[ldesc].lstate = %d , lptr->ltype = %d, proctab[currpid].locktype[ldesc]= %d\n", locks[ldesc].lstate,lptr->ltype,proctab[currpid].locktype[ldesc]);
		return SYSERR;
	}

	proctab[currpid].locktype[ldesc] = NONE;
	locks[ldesc].lprocs[currpid] = NONE; /* Update the lprocs entry of the lock for the current process as it is release the lock */


	if(lptr->ltype == READ)
	{
		lptr->lreaders--;

		if(lptr->lreaders > 0)
		{
			take_next = 0;
			return OK;
		}
		else
		{
			take_next = 1;
		}
	}
	else if(lptr->ltype == WRITE)
	{
		take_next = 1;
	}
	if(take_next == 1)
	{
		next = get_next(ldesc,&max_w_prio);
		//kprintf("\n\t[RELEASEALL.C: 75] Found next element = '%s' and next type = %d\n",proctab[next].pname,proctab[next].locktype[ldesc]);
		if(next == -1)
		{
			locks[ldesc].ltype = NONE;
			return OK;
		}
		//kprintf("\n\t[RELEASALL.C: 88] Will run '%s' next and '%s' is the last one in the queue: New lock type will be %d \n",proctab[next].pname,proctab[q[locks[ldesc].lqtail].qprev].pname,proctab[next].locktype[ldesc]);
		if(proctab[next].locktype[ldesc] == READ)
		{
			border_index = q[next].qprev;
			dequeue(next);
			ready(next,RESCHNO);
			locks[ldesc].lprocs[next] = READ;
			locks[ldesc].lreaders++;
			//kprintf("\n\t[RELEASALL.C: 93] Will run '%s' next and '%s' is the last one in the queue: number of readers = %d\n",proctab[next].pname,proctab[q[locks[ldesc].lqtail].qprev].pname,locks[ldesc].lreaders);
			/* Remove all the other Readers that have priority greate than the maximum priority of waiting Writer*/
			remove_other_reads(ldesc,border_index,max_w_prio);
			locks[ldesc].ltype = READ;
		}
		else
		{
			locks[ldesc].ltype = WRITE;
			dequeue(next);
			ready(next,RESCHNO);
			locks[ldesc].lprocs[next] = WRITE;
		}
	}
	return OK;
}

/* get the next process to be run .*/
/* if it is a READ, get the max priority of the WRITE processes in the wait queue */
/* this max_w_prio is used to consider all the READ processes less than this prio */
int get_next(int ldesc, int *max_w_prio)
{
	if(q[locks[ldesc].lqtail].qprev == locks[ldesc].lqhead)
	{
		return -1;
	}
	int temp = q[locks[ldesc].lqtail].qprev;
	int temp1;
	int diff_in_time = 0;
	int ret = 0;

	/* If the last entry in queue is WRITE directly run it as it would be of the highest priority */
	if(proctab[temp].locktype[ldesc] == WRITE)
	{
		ret = temp;
		*max_w_prio = -1;
	}
	else
	{
		temp1 = q[temp].qprev;

		/* if last entry in the queue has the highest prio           */
		/* directly run it as there are no others with the same prio */
		if(q[temp].qkey > q[temp1].qkey)
		{
			ret = temp;
			*max_w_prio = -1;
		}
		else if((q[temp].qkey == q[temp1].qkey))
		{
			/* else, check if there is an other entry with the same prio*/
			while((temp1 != locks[ldesc].lqhead) && (q[temp].qkey == q[temp1].qkey))
			{
				/* if the same prio entry is a reader, we need not consider this */
				/* as remove_other_reads would take care of this                 */
				if(proctab[temp1].locktype[ldesc] == READ)
					ret = temp;
				/* else if the same prio entry is a writer, check for the time difference*/
				else
				{
					diff_in_time = proctab[temp1].pintime - proctab[temp].pintime;
					//kprintf("\n\t[RELEASEALL.C: 137] Difference in time between '%s' and '%s' = %d\n",proctab[temp].pname,proctab[temp1].pname,diff_in_time);
					/* if the time difference is within 1s (1000ms), run the WRITE entry*/
					if(diff_in_time < 1000)
						ret = temp1;
					/* else run the READ entry */
					else
						ret = temp;
					break;
				}
				temp1 = q[temp1].qprev;
			}
		}
		/* to find max_w_prio */
		temp = q[locks[ldesc].lqtail].qprev;
		{
			while(temp!=locks[ldesc].lqhead && proctab[temp].locktype[ldesc]!=WRITE)/* iterate till u find a writer - this will be the writer with highest prio */
				temp = q[temp].qprev;
			*max_w_prio = q[temp].qkey;
		}

	}
	return ret;
}

/* remove and run all the READ processes in the wait queue of ldesc */
/* which have a prio less than the current READ and greater than the max prio of the waiting WRITERs */
void remove_other_reads(int ldesc, int border_index, int max_prio)
{
	int temp1;
	int temp = border_index;

	while(temp != locks[ldesc].lqhead && q[temp].qkey >= max_prio)
	{
		if(proctab[temp].locktype[ldesc]==READ)
		{
			temp1=q[temp].qprev;
			locks[ldesc].lreaders++;
			//kprintf("\n\t[RELEASALL.C: 164] Will run '%s' next and '%s' is the last one in the queue: number of readers = %d\n",proctab[temp].pname,proctab[q[locks[ldesc].lqtail].qprev].pname,locks[ldesc].lreaders);
			dequeue(temp);
			ready(temp,RESCHNO);
			locks[ldesc].lprocs[temp] = READ;
			temp=temp1;
		}
		else
			temp = q[temp].qprev;
	}
}
