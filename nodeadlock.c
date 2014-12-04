#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/shm.h>

#define N 5
#define LEFT (i-1+N)%N
#define RIGHT (i+1)%N
#define THINKING 0
#define HUNGRY 1
#define EATING 2
typedef int semaphore;
struct sembuf pop, vop ;

semaphore* semid;
semaphore* mutex;
int mutexID;
int* state;
int stateID;
int semID;

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
	int thinkTime = rand()%5+1;
	int eatTime = rand()%5+1;

	while(1)
	{
		printf("Philosopher %d begins thinking\n",i);
		sleep(thinkTime);  //thinking
		printf("Philosopher %d ends thinking\n",i);

		take_forks(i);

		printf("Philosopher %d begins eating\n",i);
		sleep(eatTime);  //eating
		printf("Philosopher %d ends eating\n",i);

		put_forks(i);
	}

}

void take_forks(int i)
{
	P(*mutex);             //enter critical region
	state[i] = HUNGRY;    //record fact that philosopher is hungry
	test(i);              //try to acquire 2 forks
	V(*mutex);             //exit critical region
	P(semid[i]);          //block if forks not acquired
}

void put_forks(int i)
{
	P(*mutex);
	state[i] = THINKING;
	test(LEFT);
	test(RIGHT);
	V(*mutex);
}

void test(int i)
{
	if(state[i]==HUNGRY && state[LEFT]!=EATING && state[RIGHT]!=EATING)
	{
		state[i] = EATING;
		V(semid[i]);
	}
}

int main()
{
	int i;
	
	srand(10);
	

	stateID = shmget(10, N*sizeof(int), 0777|IPC_CREAT);
	state = (int *)shmat(stateID, 0, 0);
	
	for (i = 0; i < N; i++)
	{
		state[i] = THINKING;
	}

	semID = shmget(11, N*sizeof(semaphore), 0777|IPC_CREAT);
	semid = (semaphore *)shmat(semID, 0, 0);

	mutexID = shmget(12, 1*sizeof(semaphore), 0777|IPC_CREAT);
	mutex = (semaphore *)shmat(mutexID, 0, 0);

	for(i=0;i<N;i++)
	{
		semid[i] = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
		semctl(semid[i], 0, SETVAL, 0);
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

	int status;
	wait(&status);
}