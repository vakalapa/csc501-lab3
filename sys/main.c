/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

//void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
 #define LOOP 20

int reader1, reader2, reader3, writer1, writer2, writer3;


void proc(char c,int lock1, int waitprio, int locktype) {
	int i;
	int count = 0;

	lock(lock1, locktype, waitprio);

	while (count++ < LOOP) {
		kprintf("%c", c);
		for (i = 0; i < 10000000; i++);//this is to introduce latency between prints
	}

	sleep(1); /* So that other processes go ahead and try to acquire the lock */

	releaseall(1,lock1);
}

void proc_r(char c,int lock1, int lock2, int waitprio, int locktype) {
	int i;
	int count = 0;

	lock(lock1, locktype, waitprio);

	while (count++ < LOOP) {
		kprintf("%c", c);
		for (i = 0; i < 10000000; i++);//this is to introduce latency between prints
	}

	sleep(1); /* So that other processes go ahead and try to acquire the lock */

	kprintf("\n\tLock %d and Lock %d release by Proc %c success = %d [1: Successful, -1:Unsuccessful]\n",lock1,lock2,c,releaseall(2,lock1,lock2));
}

void proc_w(char c,int lock1, int lock2, int waitprio, int locktype) {
	int i;
	int count = 0;

	lock(lock1, locktype, waitprio);

	while (count++ < LOOP) {
	kprintf("%c", c);
	for (i = 0; i < 10000000; i++);//this is to introduce latency between prints
	}

	sleep(1); /* So that other processes go ahead and try to acquire the lock */

	kprintf("\n\tLock %d and Lock %d release by Proc %c success = %d [1: Successful, -1:Unsuccessful]\n",lock1,lock2,c,releaseall(2,lock1,lock2));
}

void proc_lock_deleted(char c,int lock1, int waitprio, int locktype) {
	int i;
	int count = 0;

	int l_ret = lock(lock1, locktype, waitprio);
	kprintf("\n\tLock %d acquire by Proc %c = %d [1: OK, -1: SYSERR]\n",lock1,c,l_ret);

	if(l_ret != OK)
		return;

	while (count++ < LOOP) {
		kprintf("%c", c);
		for (i = 0; i < 10000000; i++);//this is to introduce latency between prints
	}

	releaseall(1,lock1);
}

void proc_release_delete(char c,int lock1, int waitprio, int locktype) {
	int i;
	int count = 0;

	int l_ret = lock(lock1, locktype, waitprio);

	if(l_ret != OK)
		return;

	while (count++ < LOOP) {
		kprintf("%c", c);
		for (i = 0; i < 10000000; i++);//this is to introduce latency between prints
	}

	sleep(1); /* Control goes to main here and main deletes the lock */

	int rel_ret = releaseall(1,lock1);

	kprintf("\n\tRelease of Lock %d by Proc %c = %d [1: OK, -1: SYSERR]\n",lock1,c,rel_ret);
}

/* This process skips the critical section if a lock it is waiting on is deleted */
void proc_skip_crit_lock_del(char c,int lock1, int waitprio, int locktype) {
	int i;
	int count = 0;

	int lret = lock(lock1, locktype, waitprio);

	/* Skip critical section if lock is deleted*/
	if(lret == DELETED)
	{
		kprintf("\n\tSkipping critical section of Proc %c as Lock %d it is waiting on is deleted\n",c,lock1);
		return;
	}

	while (count++ < LOOP) {
		kprintf("%c", c);
		for (i = 0; i < 10000000; i++);//this is to introduce latency between prints
	}

	releaseall(1,lock1);
}

void proc_del_lock(char c,int lock1, int waitprio, int locktype) {
	int i;
	int count = 0;

	lock(lock1, locktype, waitprio);

	while (count++ < LOOP) {
		kprintf("%c", c);
		for (i = 0; i < 10000000; i++);//this is to introduce latency between prints
	}
	sleep(1); /* So that other processes go ahead and try to acquire the lock */
	ldelete(lock1); /* Delete the lock here so that the reader should return an error */
	sleep(1);

	releaseall(1,lock1);
}


int main()
{
	int i;
	int count=0;
	int lockdesc;
#if 1
	/* Test Case 1: readers and writers with same priorities, with readers being called first and then writers*/
	kprintf("\n\n########## Test Case1: same priority readers and writers: readers called first and then writers\n");
	lockdesc = lcreate();
	reader1 = create(proc, 2000, 30, "Reader 1", 4, 'A', lockdesc, 50, READ);
	reader2 = create(proc, 2000, 30, "Reader 2", 4, 'B', lockdesc, 50, READ);
	writer1 = create(proc, 2000, 30, "Writer 1", 4, 'C', lockdesc, 50, WRITE);
	writer2 = create(proc, 2000, 30, "Writer 2", 4, 'D', lockdesc, 50, WRITE);

	resume(reader1);
	resume(reader2);

	resume(writer1);
	resume(writer2);

	sleep(4); /* To make sure that all the readers and writers are done before deleting the lockdesc */

	ldelete(lockdesc);
	while (count++ < LOOP) {
		kprintf("M");

		for (i = 0; i < 10000000; i++);
	}
	kill(reader1);
	kill(reader2);
	kill(writer1);
	kill(writer2);
	/***************************** End of Test Case 1 *****************************/

	count = 0;
	/* Test Case 2: readers and writers with different priorities */
	kprintf("\n\n########## Test Case2: Readers and Writers with different priorities\n");
	lockdesc = lcreate();
	reader1 = create(proc, 2000, 30, "Reader 1", 4, 'A', lockdesc, 50, READ);
	reader2 = create(proc, 2000, 20, "Reader 2", 4, 'B', lockdesc, 20, READ);
	reader3 = create(proc, 2000, 30, "Reader 3", 4, 'C', lockdesc, 30, READ);
	writer1 = create(proc, 2000, 50, "Writer 4", 4, 'D', lockdesc, 50, WRITE);
	writer2 = create(proc, 2000, 30, "Writer 5", 4, 'E', lockdesc, 30, WRITE);
	writer3 = create(proc, 2000, 100, "Writer 6", 4, 'F', lockdesc, 100, WRITE);

	resume(writer3); /* prio = 100 - highest*/
	resume(writer2); /* prio = 30*/

	resume(reader1); /* prio = 50 > the previous writer*/

	resume(reader2);
	resume(reader3);
	resume(writer1);

	sleep(5);//to make sure that main waits till all the readers and writers are done before deleting the lockdesc
	ldelete(lockdesc);
	while (count++ < LOOP) {
		kprintf("M");

		for (i = 0; i < 10000000; i++);
	}

	kill(reader1);
	kill(reader2);
	kill(reader3);
	kill(writer1);
	kill(writer2);
	kill(writer3);

	/***************************** End of Test Case 2 *****************************/
#endif
	count = 0;
	/* Test Case 3:  Testing the case of time difference between equal priority reader and writer */
	kprintf("\n\n########## Test Case3:  Testing the case of time difference between equal priority reader and writer \n");
	lockdesc = lcreate();
	reader1 = create(proc, 2000, 30, "Reader 1", 4, 'A', lockdesc, 50, READ);
	reader2 = create(proc, 2000, 20, "Reader 2", 4, 'B', lockdesc, 60, READ);
	reader3 = create(proc, 2000, 30, "Reader 3", 4, 'C', lockdesc, 70, READ);
	writer1 = create(proc, 2000, 50, "Writer 1", 4, 'D', lockdesc, 50, WRITE);
	writer2 = create(proc, 2000, 30, "Writer 2", 4, 'E', lockdesc, 55, WRITE);
	writer3 = create(proc, 2000, 100, "Writer 3", 4, 'F', lockdesc, 100, WRITE);

	resume(writer3); /* prio = 100 - highest*/

	resume(reader1);
	resume(writer2); /* prio = 30*/

	resume(reader2);
	resume(reader3);

	sleep(1); /* So that this writer comes at least 1sec after the previous reader with the same priority */
	//for (i = 0; i < 1000000000; i++);
	resume(writer1);

	//wakeup();
	sleep(5);//to make sure that main waits till all the readers and writers are done before deleting the lockdesc
//	ldelete(lockdesc);

	while (count++ < LOOP) {
		kprintf("M");

		for (i = 0; i < 10000000; i++);
	}

	kill(reader1);
	kill(reader2);
	kill(reader3);
	kill(writer1);
	kill(writer2);
	kill(writer3);

	/***************************** End of Test Case 3 *****************************/
#if 1
	count = 0;
	/* Test Case 4:  Testing the case where a process tries to release a lockdesc it doesn't hold */
	kprintf("\n\n########## Test Case4:  Testing the case where a process tries to release a lockdesc it doesn't hold \n");

	int lock_r = lcreate(); /* lockdesc for the reader */
	int lock_w = lcreate(); /* lockdesc for the writer */
	reader1 = create(proc_r, 2000, 30, "Reader 1", 5, 'A', lock_r,lock_w, 50, READ);
	writer1 = create(proc_w, 2000, 50, "Writer 1", 5, 'B', lock_w,lock_r, 50, WRITE);

	resume(reader1);/* should return error while trying to release lock_w */
	resume(writer1);/* should return error while trying to release lock_r */

	sleep(3);//to make sure that main waits till all the readers and writers are done before deleting the lockdesc
	ldelete(lock_r);
	ldelete(lock_w);

	while (count++ < LOOP) {
		kprintf("M");

		for (i = 0; i < 10000000; i++);
	}

	kill(reader1);
	kill(writer1);

	/***************************** End of Test Case 4 *****************************/

	count = 0;
	/* Test Case 5:  Testing multiple deletes of the same lockdesc */
	kprintf("\n\n########## Test Case5:  Testing multiple deletes of the same lockdesc \n");

	lockdesc = lcreate();

	int del_ret = ldelete(lockdesc);
	kprintf("\n\tLock %d deleted first time = %d [1: OK, -1: SYSERR]\n",lockdesc,del_ret);
	del_ret = ldelete(lockdesc); /* Should get a SYSERR here as the lockdesc is already deleted */
	kprintf("\n\tLock %d deleted second time = %d [1: OK, -1: SYSERR]\n",lockdesc,del_ret);

	while (count++ < LOOP) {
		kprintf("M");

		for (i = 0; i < 10000000; i++);
	}

	kill(reader1);

	/***************************** End of Test Case 5 *****************************/

	count = 0;
	/* Test Case 6:  Testing lockdesc acquire after the lockdesc is deleted */
	kprintf("\n\n########## Test Case6:  Testing lockdesc acquire after the lockdesc is deleted \n");

	lockdesc = lcreate();
	reader1 = create(proc_lock_deleted, 2000, 30, "Reader 1", 4, 'A', lockdesc, 50, READ);

	del_ret = ldelete(lockdesc);

	resume(reader1); /* Should give a SYSERR here as lockdesc is already deleted and this proc cannot acquire it */

	while (count++ < LOOP) {
		kprintf("M");

		for (i = 0; i < 10000000; i++);
	}

	kill(reader1);

	/***************************** End of Test Case 6 *****************************/

	count = 0;
	/* Test Case 7:  Testing lockdesc release after the lockdesc has been deleted */
	kprintf("\n\n########## Test Case7:  Testing lockdesc release after the lockdesc has been deleted \n");

	lockdesc = lcreate();
	reader1 = create(proc_release_delete, 2000, 30, "Reader 1", 4, 'A', lockdesc, 50, READ);

	resume(reader1);

	ldelete(lockdesc);

	sleep(1); /* So that reader1 gets back the control before getting killed */

	while (count++ < LOOP) {
		kprintf("M");

		for (i = 0; i < 10000000; i++);
	}

	kill(reader1);

	/***************************** End of Test Case 7 *****************************/

	count = 0;
	/* Test Case 8:  Testing behavior of critical waiting process when a lockdesc it is waiting on is deleted */
	kprintf("\n\n########## Test Case8:  Testing behavior of critical waiting process when a lockdesc it is waiting on is deleted \n");

	lockdesc = lcreate();
	reader1 = create(proc_skip_crit_lock_del, 2000, 30, "Reader 1", 4, 'R', lockdesc, 50, READ);
	writer1 = create(proc_del_lock, 2000, 50, "Writer 4", 4, 'W', lockdesc, 50, WRITE);

	resume(writer1);
	resume(reader1);
	sleep(2);
	while (count++ < LOOP) {
		kprintf("M");

		for (i = 0; i < 10000000; i++);
	}

	kill(reader1);
	kill(writer1);

	/***************************** End of Test Case 8 *****************************/
#endif
kprintf("\n\nHello World, Xinu lives\n\n");
return 0;
}
