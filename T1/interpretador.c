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


#define MAX_NAME 13
#define MAX_MEM 1024
#define OPENMODE (O_WRONLY)
#define FIFO "fifo1"

//sinais auxiliares

#define SIGUSR3 SIGIO
#define SIGUSR4 SIGURG

int pidEscalonador = -1;

int sharedMemoryIdx = -1;


FILE *execFile; // Entrada para o interpretador


void signHandler(int signal);

void initProc(int);


int main(int argc, char * const argv[], char * const envp[])
{
	int status;
	int fpfifo, i ;
	// char * arr[3]


	pidEscalonador = fork();

	if(pidEscalonador < 0)
	{
		puts("Erro ao criar o processo do Escalonador");
		exit(-3);
	}
	else if(pidEscalonador == 0)
	{
// Aux Func
		//char sharedMemoryC[MAX_NAME + 1];

		//snprintf(sharedMemoryC, MAX_NAME + 1, "%d", sharedMemoryIdx);

		execve("./escalonador",argv, 0);

		//exit(-4);
		waitpid(pidEscalonador, &status, 0);
		//sleep(2);
	}



	if (access(FIFO, F_OK) == -1)
	{
		if(mkfifo (FIFO, S_IRUSR | S_IWUSR) != 0)
		{
			fprintf(stderr, "Erro ao criar fifo %s\n", FIFO) ;
			return -1 ;
		}
	}


	puts("Abrindo fifo interpretador") ;
	if((fpfifo = open(FIFO, OPENMODE)) < 0)
	{
		fprintf(stderr, "Erro ao abrir a fifo%s\n", FIFO) ;
		return -2 ;
	}
	

	//signal(SIGINT, signHandler);

	execFile = fopen("execFile.txt", "r");

	if(execFile == NULL) 
	{
		puts("Erro ao abrir arquivo execFile.txt");
		exit(-1);
	}

	// if(pipe(fd) < 0) 
	// {
	// 	puts("Erro ao abrir os pipes");
	// 	exit(-2);
	// }

	

	// signal(SIGINT, signHandler);
	sleep(1);

	// printf("%d\n", pidEscalonador );
	
	if(pidEscalonador != -1) 
	{
		initProc(fpfifo);

		waitpid(pidEscalonador, &status, 0);
	}

	fclose(execFile);

	//shmctl(sharedMemoryIdx, IPC_RMID, 0);



	return 0;
}

void signHandler(int signal)
{
	if(pidEscalonador != -1) 
	{
		kill(pidEscalonador, signal);
	}

	fclose(execFile);
	// shmctl
	exit(0);
}

void initProc(int fpfifo)
{
	char progName[MAX_NAME + 1];
	char next;
	int param1 = 0, param2 = 0;

	// String que guarda os parametros e o pid
	char aux[MAX_NAME];
	
	// ponteiro do tipo caracter para o espaco de memoria compartilhada do escalonador
	char * smEscalonador;

	smEscalonador = (char *) shmat(sharedMemoryIdx, 0 , 0);

	while(fscanf(execFile, " Run %s", progName) == 1)
	{
		next = fgetc(execFile); //" %c", &next); // Lendo caracter a caracter
	
		while(next == ' ')
		{
			next = fgetc(execFile);
		}

		if(next == 'P') //Prioridades
		{
			char cmmd[8] ;// parametros de comando de prioridade(P , int)
			fscanf(execFile, "R=%d", &param1);

			//snprintf(aux, MAX_NAME, " %d", param1);
			//strcat(smEscalonador, aux);
			snprintf(cmmd, 8, "%c%s%d", next, progName, param1) ;
			printf("Prioridades\n");
			printf("%s PR = %d enviado!\n", progName, param1); fflush(NULL);
			//Passa o comando de prioridade para o arquivo compartilhado
			write(fpfifo, cmmd, strlen(cmmd)) ;
			sleep(1);
			//kill(pidEscalonador, SIGUSR1);

		}
		else if (next == 'I') //Real-Time
		{
			fscanf(execFile, "=%d D=%d", &param1, &param2);

			snprintf(aux, MAX_NAME, " %d", param1);

			strcat(smEscalonador, aux);
			
			snprintf(aux, MAX_NAME, " %d", param2);

			strcat(smEscalonador, aux);

			kill(pidEscalonador, SIGUSR2);

			printf("RealTime\n");
		}
		else
		{
			strcat(smEscalonador, aux);

			// aux[0] = '\0';

			// execve(progName, NULL, NULL);
			
			// fscanf(execFile, " %c", &next);
			// next = fgetc(execFile);
			printf("%s enviado para o Escalonador\n", progName);
			
			kill(pidEscalonador, SIGUSR3);
		}

		// Pausa para ajudar visualizacao
		sleep(2);
	}

	// SIGUSR4 eh usado par indicar termino da leitura do arquivo
	kill(pidEscalonador, SIGUSR4);

	// Detach da variavel da memoria compartilhada 
	shmdt(smEscalonador);

}