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
#include <pthread.h>
#include <limits.h>
#include <dirent.h>
#include <libgen.h>



/*
 * Servidor TCP
 */

#define commandSizes 200
#define MaxNAME 20
#define MaxMsg 80
#define MaxArray 10


	typedef struct Obj {
		char Name[MaxNAME];
		char Msg[MaxMsg];
		int Opcao;	//Informar ao servidor qual procedimento foi realizado
	} Obj;

	struct args{
		int ns;
	};

	struct args parameters;
	pthread_t thread_id[MaxArray];
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* A mutex protecting ObjStore. */
	unsigned short port;
	char sendbuf[12];
	char recvbuf[12];
	Obj objectStore[MaxArray];
	Obj receiveMsg;
	int arrayObjCount = 0;
	int countClients = 0;

	char command[commandSizes];

void retorno_cliente(int ns, char retornoMsg[200]) {
    /* Envia uma mensagem ao cliente atraves do socket conectado */
    if (send(ns, retornoMsg, strlen(retornoMsg)+1, 0) < 0)
    {
        perror("Send()");
        exit(7);
    }
}

//opcao 1
void opcao_1(Obj rcv, int ns){
    if (arrayObjCount < 10) {
		int i = 0;
		int saved = 0;
		pthread_mutex_lock(&mutex);
		do{
			if((strcmp("", objectStore[i].Name)) == 0) {
				strcpy(objectStore[i].Name, rcv.Name);
				strcpy(objectStore[i].Msg, rcv.Msg);
				arrayObjCount++;
				saved = 1;
			}
			i++;

		}while(saved != 1 && i < MaxArray);
		pthread_mutex_unlock(&mutex);

        retorno_cliente(ns, "\nMensagem salva com sucesso!\n");

    } else {
        retorno_cliente(ns ,"Nao foi possivel inserir uma nova mensagem\n");
    }
}

//opcao 2
void opcao_2(int ns){
	int objAuxCount = 0;
	Obj objAux[MaxArray]; // array auxiliar para pegar quais mensagens foram apagadas
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < MaxArray; i++) {
		if((strcmp("", objectStore[i].Name)) != 0) {
			strcpy(objAux[objAuxCount].Name, objectStore[i].Name);
			strcpy(objAux[objAuxCount].Msg, objectStore[i].Msg);
			objAuxCount++;
		}
	}
	pthread_mutex_unlock(&mutex);

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
}

//opcao 3
void opcao_3(Obj rcv, int ns){
	printf("\nOpcao 3");
	printf("\nOpcao 3 - parameters.ns: %i", parameters.ns);

	int mensagensRemovidas = 0; // contadora para receber quantas mensagens foram apagadas
	Obj objAux[MaxArray]; // array auxiliar para pegar quais mensagens foram apagadas

	pthread_mutex_lock(&mutex);
	for(int i = 0; i < MaxArray; i++) {
		if((strcmp(rcv.Name,objectStore[i].Name)) == 0) {
			strcpy(objAux[mensagensRemovidas].Name, objectStore[i].Name);
			strcpy(objAux[mensagensRemovidas].Msg, objectStore[i].Msg);

			strcpy(objectStore[i].Name, "");
			strcpy(objectStore[i].Msg, "");

			arrayObjCount--;
			mensagensRemovidas++;
		}
	}
	pthread_mutex_unlock(&mutex);
	printf("\nOpcao 3 - FIM Mutex");

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
	printf("\nOpcao 3 - FIM Funcao");

}

void listar(int ns) {
	DIR *dir;
    struct dirent *dp;
	char cwd[PATH_MAX];
	char stop[] = "stop";
	char copy[256];

	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("Current working dir: %s\n", cwd);
	}

    if ((dir = opendir(cwd)) == NULL) {
        perror ("Cannot open .");
        exit (1);
    }

	while (dir) {
		if ((dp = readdir(dir)) != NULL) {
			strcpy(copy, dp->d_name);
			if (send(ns, &copy, (sizeof(copy)), 0) < 0) {
				perror("Send()");
				exit(7);
			}
		} else {
			if (send(ns, &stop, (sizeof(stop)), 0) < 0) {
				perror("Send()");
				exit(7);
			}
			printf("Fechou a leitura do diretorio ... \n");
			closedir(dir);
			break;
		}
	}
}


void *recebe_comando(void* parameters){
	struct args args = *((struct args*) parameters);
    bool variavelLoop = false;

    do {
        /* Recebe uma mensagem do cliente atraves do novo socket conectado */
		memset(&command, 0, sizeof(command));
		if (recv(args.ns, &command, sizeof(command), 0) == -1)
        {
            perror("Recv()");
            exit(6);
        }

		char * token = strtok(command, " ");
		char value[3][200] ;
		int i = 0;

		while( token != NULL ) {
            strcpy(value[i],token);
			printf("Token[%i] = %s\n", i, token);
            token = strtok(NULL, " ");
            i++;
		}

        if ((strcmp(value[0], "listar")) == 0) {
            listar(args.ns);
        }

    } while(!variavelLoop);
	return NULL;
}

int main(int argc, char **argv)
{
	int s;                     /* Socket para aceitar conexoes       */
	int ns;                    /* Socket conectado ao cliente        */
	struct sockaddr_in client; 
	struct sockaddr_in server; 
	int namelen;
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
		parameters.ns = ns;
		printf("command01");

		if (pthread_create(&thread_id[countClients], NULL, recebe_comando, (void* )&parameters))
        {
            printf("ERRO: impossivel criar uma thread\n");
            exit(-1);
        }
		printf("Thread[%i] criada\n", countClients);
		countClients++;
        pthread_detach(thread_id[countClients]);
	}
}
