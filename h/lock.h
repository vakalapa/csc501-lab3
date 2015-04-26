/*LOCK.h contains 
*/

#ifndef _LOCK_H_
#define _LOCK_H_
#include <proc.h>

#ifndef	NLOCKS	
#define NLOCKS 50
#endif

#define READ 0
#define WRITE 1
#define NONE 2

#define LFREE  '\03'
#define LUSED  '\04'

struct lentry{
	char lstate; // This gives the current state of the lock
	int lreaders; //number of readers
	int lwriters;  //number of writers
	int ltype; //Current access type
	int  lqhead;    // q index of head of list
    int  lqtail;    // q index of tail of list
    int  liter;     // lock iteration number
    int lprocs[NPROC];

};

#define isbadlock(l) (l<0 || l>=NLOCKS)

extern struct lentry locks[];
extern int nextlock;
extern int lockiter;

extern unsigned long ctr1000;


void linit(void);
int  lcreate(void);
int  ldelete(int lockdescriptor);
int  lock(int ldes1, int type, int priority);
int  releaseall(int numlocks, int ldes1, ...);


#endif

