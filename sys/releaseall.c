// releaseall.c
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include <lock.h>

LOCAL void admitreader(int ldes);
LOCAL void unblock(int ldes, int item);

int releaseall(numlocks, ldes1)
    int numlocks;
    int ldes1;
{
return SYSERR;
    	}

