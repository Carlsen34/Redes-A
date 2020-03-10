/*
Herick Valsecchi Carlsen 15159619
João Pedro Favara 16061921
Raissa Furlan Davinha 15032006
Leonardo Blanco Natis 15296858
Kaíque Ferreira Fávero 15118698
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h> /* for msgget(), msgctl() */ //faltando sys
#include <sys/msg.h> /* for msgget(), msgctl() */ //faltando sys
#include <sys/shm.h>
#include <sys/sem.h>


/*
 * Servidor TCP
 */

#define MaxNAME 20
#define MaxMsg 80
#define MaxArray 10
#define SHM_KEY 0x1024
#define SEM_PERMS 0666
#define SEM_KEY_A 9034

	struct sembuf semaphore_lock_op[1];   // Struct containing a lock operation
	struct sembuf semaphore_unlock_op[1]; // Struct containing an unlock operation

	typedef struct Obj {
		char Name[MaxNAME];
		char Msg[MaxMsg];
		int Opcao;	//Informar ao servidor qual procedimento foi realizado
	} Obj;

	typedef struct {
		struct Obj store[MaxArray];
		int arrayObjCount;	//Informar qual a proxima posicao para ser utilizada
	} ObjStore;

	int semaphore_id;
    unsigned short port;
    char sendbuf[12];
    char recvbuf[12];
	ObjStore *objStore;
	Obj receiveMsg;
	int shmid;
	struct sockaddr_in client; 
    struct sockaddr_in server; 
    int s;                     /* Socket para aceitar conexoes       */
    int ns;                    /* Socket conectado ao cliente        */
    int namelen;
    pid_t pid, fid;

void setup_semaforo(int *semaphore_id, int semaphore_key)
{
    // Creating the semaphore
    if ((*semaphore_id = semget(semaphore_key, 1, IPC_CREAT | SEM_PERMS)) == -1)
    {
        fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
        exit(1);
    }
    // Initializing it as unlocked
    if (semop(*semaphore_id, semaphore_unlock_op, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel inicializar o semaforo!");
        exit(1);
    }
}

void lockSemaphore(int semaphore_id)
{
#ifdef PROTECT
    if (semop(semaphore_id, semaphore_lock_op, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso! Erro: %s", strerror(errno));
        exit(1);
    }
    else
    {
        //fprintf(stdout, "\n<Locked semaphore with id: %d>\n", semaphore_id);
        //fflush(stdout);
    }
#endif
}
void unlockSemaphore(int semaphore_id)
{
#ifdef PROTECT
    if (semop(semaphore_id, semaphore_unlock_op, 1) == -1)
    {
        fprintf(stdout, "chamada semop() falhou, impossivel liberar o recurso!");
        exit(1);
    }
    else
    {
        //fprintf(stdout, "\n<Unlocked semaphore with id: %d>\n", semaphore_id);
        //fflush(stdout);
    }
#endif
}

void removeSemaphore(int semaphore_id)
{
    if (semctl(semaphore_id, 0, IPC_RMID, 0) != 0)
    {
        fprintf(stderr, "Impossivel remover o conjunto de semaforos! Error: %s\n semaphore id: %d", strerror(errno), semaphore_id);
        exit(1);
    }
    else
    {
        fprintf(stdout, "\nConjunto de semaforos removido com sucesso!");
    }
}

void setup_shared_memory() {

	memset(&objStore, 0, sizeof(objStore));
	memset(&shmid, 0, sizeof(shmid));
	
	if ((shmid = shmget(SHM_KEY, sizeof(ObjStore), 0644|IPC_CREAT)) == -1) {
		perror("Shared memory error");
		exit(0);
	}

	objStore = shmat(shmid, NULL, 0);
	if (objStore == (void *) -1) {
		perror("Shared memory attach error");
		exit(0);
	}

	for(int i = 0; i < MaxArray; i++) {
		strcpy(objStore->store[i].Name, "");
		strcpy(objStore->store[i].Msg, "");
	}
	objStore->arrayObjCount = 0;


	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		perror("shmctl");
		exit(1);
	}
};

void retorno_cliente(char retornoMsg[200]) {
    /* Envia uma mensagem ao cliente atraves do socket conectado */
    if (send(ns, retornoMsg, strlen(retornoMsg)+1, 0) < 0)
    {
        perror("Send()");
        exit(7);
    }
}

//opcao 1
void opcao_1(Obj rcv){
	lockSemaphore(semaphore_id);
    if (objStore->arrayObjCount < 10) {
		int i = 0;
		int saved = 0;
		do{
			if((strcmp("", objStore->store[i].Name)) == 0) {
				strcpy(objStore->store[i].Name, rcv.Name);
				strcpy(objStore->store[i].Msg, rcv.Msg);
				objStore->arrayObjCount++;
				saved = 1;
			}
			i++;

		}while(saved != 1 && i < MaxArray);
	unlockSemaphore(semaphore_id);

        retorno_cliente("\nMensagem salva com sucesso!\n");
    } else {
        retorno_cliente("Nao foi possivel inserir uma nova mensagem\n");
    }
}

//opcao 2
void opcao_2(){
	lockSemaphore(semaphore_id);
	int objAuxCount = 0;
	Obj objAux[MaxArray]; // array auxiliar para pegar quais mensagens foram apagadas
	for(int i = 0; i < MaxArray; i++) {
		if((strcmp("", objStore->store[i].Name)) != 0) {
			strcpy(objAux[objAuxCount].Name, objStore->store[i].Name);
			strcpy(objAux[objAuxCount].Msg, objStore->store[i].Msg);
			objAuxCount++;
		}
	}

    if (send(ns, &objAuxCount, sizeof(objAuxCount), 0) < 0) {
        perror("Send()");
        exit(7);
    }

	if ( objAuxCount >= 1) {
		for(int i = 0; i < objAuxCount; i++) {
			if (send(ns, &objAux[i], sizeof(objAux[i]), 0) < 0)
			{
				perror("Send()");
				exit(7);
			}
		}
	}
	unlockSemaphore(semaphore_id);
}

//opcao 3
void opcao_3(Obj rcv){
	int mensagensRemovidas = 0; // contadora para receber quantas mensagens foram apagadas
	Obj objAux[MaxArray]; // array auxiliar para pegar quais mensagens foram apagadas

	lockSemaphore(semaphore_id);
	for(int i = 0; i < MaxArray; i++) {
		if((strcmp(rcv.Name,objStore->store[i].Name)) == 0) {
			strcpy(objAux[mensagensRemovidas].Name, objStore->store[i].Name);
			strcpy(objAux[mensagensRemovidas].Msg, objStore->store[i].Msg);

			strcpy(objStore->store[i].Name, "");
			strcpy(objStore->store[i].Msg, "");

			objStore->arrayObjCount--;
			mensagensRemovidas++;
		}
	}
	unlockSemaphore(semaphore_id);


	// enviar a quantidade de mensagens apagadas
	if (send(ns, &mensagensRemovidas, sizeof(mensagensRemovidas), 0) < 0) {
		perror("Send()");
		exit(7);
	}

	// enviar quais foram as mensagens apagadas
	for(int i = 0; i < mensagensRemovidas; i++) {
		if (send(ns, &objAux[i], sizeof(objAux[i]), 0) < 0) {
			perror("Send()");
			exit(7);
		}
	}
}


void recebe_envia_mensagem(){
     bool variavelLoop = false;
    do {
        /* Recebe uma mensagem do cliente atraves do novo socket conectado */
		memset(&receiveMsg, 0, sizeof(receiveMsg));
		if (recv(ns, &receiveMsg, sizeof(receiveMsg), 0) == -1)
        {
            perror("Recv()");
            exit(6);
        }
		// printf("receiveMsg.Opcao >>> %i\n", receiveMsg.Opcao);

        switch (receiveMsg.Opcao){
        case 1:
        	opcao_1(receiveMsg);
            break;
        case 2:
        	opcao_2();
            break;
        case 3:
        	opcao_3(receiveMsg);
            break;
        case 4:
            variavelLoop = true;
        default:
        retorno_cliente("\nOpcao invalida");
        }
    } while(!variavelLoop);
}


int main(int argc, char **argv)
{
	semaphore_lock_op[0].sem_num = 0;
    semaphore_lock_op[0].sem_op = -1;
    semaphore_lock_op[0].sem_flg = 0;
    semaphore_unlock_op[0].sem_num = 0;
    semaphore_unlock_op[0].sem_op = 1;
    semaphore_unlock_op[0].sem_flg = 0;

    /*
     * O primeiro argumento (argv[1]) e a porta
     * onde o servidor aguardara por conexoes
     */
    if (argc != 2)
    {
	  fprintf(stderr, "\nUse: %s porta", argv[0]);
	  exit(1);
    }

    port = (unsigned short) atoi(argv[1]);

    /*
     * Cria um socket TCP (stream) para aguardar conexoes
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
	  perror("Socket()");
	  exit(2);
    }

   /*
    * Define a qual endereco IP e porta o servidor estara ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os enderecos IP
    */
    server.sin_family = AF_INET;   
    server.sin_port   = htons(port);       
    server.sin_addr.s_addr = INADDR_ANY;

/* Imprime qual porta E IP foram utilizados. */
    printf("\nPorta utilizada: %d", ntohs(server.sin_port));
    printf("\nIP utilizado: %d\n", ntohs(server.sin_addr.s_addr));

	/*
     * Liga o servidor a porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
	perror("Bind()");
	exit(3);
    }

    /*
     * Prepara o socket para aguardar por conexoes e
     * cria uma fila de conexoes pendentes.
     */
    if (listen(s, 1) != 0)
    {
	  perror("Listen()");
	  exit(4);
    }
	
	setup_semaforo(&semaphore_id, SEM_KEY_A);
	setup_shared_memory(); // Faz o setup para fazer a memoria compartilhada
   

    while(1)
    {
	  /*
	  * Aceita uma conexao e cria um novo socket atraves do qual
	  * ocorrera a comunicacao com o cliente.
	  */
	  namelen = sizeof(client);
	  if ((ns = accept(s, (struct sockaddr *) &client, (socklen_t *) &namelen)) == -1)
	  {
		perror("Accept()");
		exit(5);
	  }
	  
	  if ((pid = fork()) == 0) {
		/*
		 * Processo filho 
		 */
	      
		/* Fecha o socket aguardando por conexoes */
		close(s);

		/* Processo filho obtem seu proprio pid */
		fid = getpid();
	
		recebe_envia_mensagem();

		/* Fecha o socket conectado ao cliente */
		close(ns);

		/* Processo filho termina sua execucao */
		printf("[%d] Processo filho terminado com sucesso.\n", fid);
		exit(0);
	  }
	  else
	  {  
		/*
		 * Processo pai 
		 */
		
		if (pid > 0)
		{
		    printf("Processo filho criado: %d\n", pid);

		    /* Fecha o socket conectado ao cliente */
		    close(ns);
		}
		else
		{
		    perror("Fork()");
		    exit(7);
		}
	  }
    }
}
