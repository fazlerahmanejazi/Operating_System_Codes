#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/shm.h>

#define N 5
#define LEFT i
#define RIGHT (i+1)%N
#define THINKING 0
#define HUNGRY 1
#define TAKENLEFT 2
#define EATING 3

typedef int semaphore;
struct sembuf pop, vop ;

semaphore* semFork;
semaphore* mutex;
int mutexID;
int* count;
int semForkID;
int countID;

struct sembuf pop, vop ;

#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */

void philosopher(int i);
void take_forks(int i);
void put_forks(int i);
void test(int i);

void philosopher(int i)
{
	srand(10);
	int thinkTime = rand()%5+1;
	int eatTime = rand()%5+1;

	while(1)
	{
		printf("Philosopher %d starting thinking\n",i);
		sleep(thinkTime);  //thinking

		take_forks(i);

		printf("Philosopher %d starts eating\n",i);
		sleep(eatTime);  //eating
		printf("Philosopher %d ends eating and releases forks %d (left) and %d (right)\n",i,LEFT,RIGHT);

		put_forks(i);
	}

}

void take_forks(int i)
{
	int pickLag = rand()%5+1;
	P(semFork[LEFT]);
	(*count)++;
	printf("Philosopher %d grabs fork %d (left)\n",i,LEFT);
	sleep(pickLag);
	P(semFork[RIGHT]);
}

void put_forks(int i)
{
	V(semFork[LEFT]);
	(*count)--;
	V(semFork[RIGHT]);
}


int main()
{
	int i;

	semForkID = shmget(10, N*sizeof(semaphore), 0777|IPC_CREAT);
	semFork = (semaphore *)shmat(semForkID, 0, 0);

	mutexID = shmget(11, 1*sizeof(semaphore), 0777|IPC_CREAT);
	mutex = (semaphore *)shmat(mutexID, 0, 0);

	countID = shmget(12, 1*sizeof(int), 0777|IPC_CREAT);
	count = (int *)shmat(countID, 0, 0);

	*count = 0;

	for(i=0;i<N;i++)
	{
		semFork[i] = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
		semctl(semFork[i], 0, SETVAL, 1);
	}

	*mutex = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	semctl(*mutex, 0, SETVAL, 1);

	pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;

	if(fork()==0)
	{
		philosopher(0);
	}
	else if(fork()==0)
	{
		philosopher(1);
	}
	else if(fork()==0)
	{
		philosopher(2);
	}
	else if(fork()==0)
	{
		philosopher(3);
	}
	else if(fork()==0)
	{
		philosopher(4);
	}
	else
	{
		while(1)
		{
			if(*count==5)
			{
				printf("Parent detects deadlock, going to initiate recovery\n");
				int ri = rand()%5;
				V(semFork[ri]);
				(*count)--;
				printf("Parent preempts Philosopher %d\n", ri);
			}
			sleep(1);
		}
	}

	int status;
	wait(&status);
}