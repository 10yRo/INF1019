#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_NAME_SIZE 64
#define N_MAX_PROCS 200
#define OPENMODE (O_RDONLY)
#define FIFO "fifo1"

//sinais auxiliares 

#define SIGUSR3 SIGIO
#define SIGUSR4 SIGURG

typedef enum scheduling 
{
	real_time, priorities, round_robin
} Scheduling;


typedef struct job 
{
	char name[MAX_NAME_SIZE + 1];
	int pid;
	Scheduling sched;
	int param1;
	int param2;
} Job;


// ponteiro do tipo caracter para a memoria compartilhada do interpretador
char * shmInterpreter;

// contadores
int real_timeJobCounter = 0;
int prioritiesJobCounter = 0;
int round_robinJobCounter = 0; 

//booleana para processos faltando ou nao
int jobsRemainingBool = 1;

Job realTimeQueue[N_MAX_PROCS];
Job prioritiesQueue[N_MAX_PROCS];
Job roundRobinQueue[N_MAX_PROCS];

Job currentJob = {.name = "", .pid = -1, .sched = -1, .param1 = -1, .param2 = -1};


u_long startTime = 0;
u_long currentTime = 0;


int nextScheduling(int * idx);

void escalonamento(int policy, int i);

void Priority(void);

void StopJob(Job currJob);

void ContinueJob(Job currJob);

void RecebePR(int sinal);

int main(int argc, char * argv[])
{
	//int shmKey = atoi(argv[1]);

	int idx = -1;

	int policy = -1;

	//shmInterpreter = (char * ) shmat(shmKey, 0, 0);

	//signal(SIGUSR1, RecebePR);

	printf("Iniciando Escalonador\n"); fflush(NULL);

	startTime = time(NULL);

	int fpfifo ;

	char ch[20]  ;
	//criando fifo se nao existe
	if (access(FIFO, F_OK) == -1)
	{
		if(mkfifo (FIFO, S_IRUSR | S_IWUSR) != 0)
		{
			fprintf(stderr, "Erro ao criar fifo %s\n", FIFO) ;
			return -1 ;
		}
	}

	puts("Abrindo fifo escalonador") ;
	if((fpfifo = open(FIFO, OPENMODE)) < 0)
	{
		fprintf(stderr, "Erro ao abrir a fifo%s\n", FIFO) ;
		return -2 ;
	}



	while(jobsRemainingBool) {

		read(fpfifo, &ch, sizeof(ch)) ;

		printf("escalonador leu: %s\n", ch) ;

		currentTime = ((u_long) time(NULL) - startTime ) % 60;

		policy = nextScheduling(&idx);

		escalonamento(policy, idx);

	}

	puts("Terminando escalonador"); fflush(NULL);
	//shmdt(shmInterpreter);

	return 0;
}


void RecebePR(int sinal) {
	Job novo;
	//int i;

	// if (ChecaNumProc())
	// 	return;
	
	novo.sched = priorities;
	
	sscanf(shmInterpreter, " %s %d", novo.name, &novo.param1);

	printf("Recebeu %s PR = %d\n", novo.name, novo.param1); fflush(NULL);
	
	if (novo.param1 < 1 || novo.param1 > 7) {
		printf("Prioridade Invalida!\n");
		fflush(NULL);
	}
	else {
		// novo.pid = IniciaPrograma(novo.name);
	
		//insere
		prioritiesQueue[prioritiesJobCounter] = novo;
		prioritiesJobCounter++;
		
		//ordena
		// qsort(PR, numPR, sizeof(Processo), OrdenaPR);
		/*for (i=0; i<numPR; i++) { //checando ordenacao dos processos
			printf("[ESC s=%ld] PR[%d].PR = %d\n", segAtual, i, PR[i].a1); fflush(stdout);
		}*/
	}
}

int nextScheduling(int * idx) 
{
	int i;

	if(real_timeJobCounter != 0) {
		for(i = 0; i < real_timeJobCounter; i++) 
		{
			if(currentTime < realTimeQueue[i].param1 || 
				currentTime >= realTimeQueue[i].param1
				+ realTimeQueue[i].param2)
			{
				return real_time; //ou continue; ?
			}
			*idx = i;
// ou return real_time; ?
		}
	}
	if(prioritiesJobCounter != 0) 
	{
		return priorities;
	}
	if (round_robinJobCounter != 0)
	{
		return round_robin;
	}
	return -1;
}

void escalonamento(int policy, int i)
{
	switch(policy){
		case real_time:
			// 
			break;
		case priorities:
			Priority();
			break;
		case round_robin:

			break;
	}
}

void Priority(void) 
{
	int finished;

	if((currentJob.sched != priorities) || 
		(currentJob.sched == priorities && 
			currentJob.param1 != prioritiesQueue[0].param1))
	{
		if(currentJob.pid != -1) 
		{
			StopJob(currentJob);
		}
		currentJob = prioritiesQueue[0];
		ContinueJob(currentJob);
	}

}

void StopJob(Job currJob)
{
	printf("Interrupção do Processo %s", currJob.name); fflush(NULL);
	kill(currJob.pid, SIGSTOP);
}

void ContinueJob(Job currJob)
{
	printf("Prosseguimento do Processo %s", currJob.name); fflush(NULL);
	kill(currJob.pid, SIGCONT);
}