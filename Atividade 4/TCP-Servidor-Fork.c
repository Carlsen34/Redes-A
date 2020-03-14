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


/*
 * Servidor TCP
 */

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
		struct sockaddr_in client; 
	};

	struct args parameters;
	pthread_t thread_id[5];
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* A mutex protecting ObjStore. */
	unsigned short port;
	char sendbuf[12];
	char recvbuf[12];
	Obj objectStore[MaxArray];
	Obj receiveMsg;
	int arrayObjCount = 0;
	int countClients = 0;
	struct sockaddr_in client; 
	struct sockaddr_in server; 
	int s;                     /* Socket para aceitar conexoes       */
	int ns;                    /* Socket conectado ao cliente        */
	int namelen;
	pid_t pid, fid;


void retorno_cliente(struct args parameters, char retornoMsg[200]) {
    /* Envia uma mensagem ao cliente atraves do socket conectado */
    if (send(parameters.ns, retornoMsg, strlen(retornoMsg)+1, 0) < 0)
    {
        perror("Send()");
        exit(7);
    }
}

//opcao 1
void opcao_1(Obj rcv, struct args parameters){
	printf("args: %d\n", parameters.ns);
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

        retorno_cliente(parameters, "\nMensagem salva com sucesso!\n");
    } else {
        retorno_cliente(parameters ,"Nao foi possivel inserir uma nova mensagem\n");
    }
}

//opcao 2
void opcao_2(struct args parameters){
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

    if (send(parameters.ns, &objAuxCount, sizeof(objAuxCount), 0) < 0) {
        perror("Send()");
        exit(7);
    }

	if ( objAuxCount >= 1) {
		for(int i = 0; i < objAuxCount; i++) {
			if (send(parameters.ns, &objAux[i], sizeof(objAux[i]), 0) < 0)
			{
				perror("Send()");
				exit(7);
			}
		}
	}
}

//opcao 3
void opcao_3(Obj rcv, struct args parameters){
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

	// enviar a quantidade de mensagens apagadas
	if (send(parameters.ns, &mensagensRemovidas, sizeof(mensagensRemovidas), 0) < 0) {
		perror("Send()");
		exit(7);
	}

	// enviar quais foram as mensagens apagadas
	for(int i = 0; i < mensagensRemovidas; i++) {
		if (send(parameters.ns, &objAux[i], sizeof(objAux[i]), 0) < 0) {
			perror("Send()");
			exit(7);
		}
	}
}


void *recebe_envia_mensagem(void* parameters){
	struct args *args = (struct args*) parameters;
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
        	opcao_1(receiveMsg, *args);
            break;
        case 2:
        	opcao_2(*args);
            break;
        case 3:
        	opcao_3(receiveMsg, *args);
            break;
        case 4:
            variavelLoop = true;
        default:
        retorno_cliente(*args, "\nOpcao invalida");
        }
    } while(!variavelLoop);
	return NULL;
}

int main(int argc, char **argv)
{
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
	  	printf("ns => %d\n", ns);
		parameters.ns = ns;
		parameters.client = client;

		if (pthread_create(&thread_id[countClients], NULL, recebe_envia_mensagem, (void* )&parameters))
        {
            printf("ERRO: impossivel criar uma thread\n");
            exit(-1);
        }
		countClients++;
        pthread_detach(thread_id[countClients]);
	}
}
