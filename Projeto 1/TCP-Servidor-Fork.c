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
#include <sys/syscall.h>


/*
 * Servidor TCP
 */

#define commandSizes 200
#define MaxArray 10

	struct args{
		int ns;
		int s;
		int thread_id;
	};

	struct args parameters;
	pthread_t thread_id[MaxArray];
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* A mutex protecting ObjStore. */
	unsigned short port;
	int countClients = 0;

	char command[commandSizes];

void encerrar(int ns, int s, int thread_id) {
	close(ns);
	// close(s);
	printf("Thread[%d] has finished ...\n", thread_id);
	pthread_exit(NULL);
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
			// printf("Token[%i] = %s\n", i, token);
            token = strtok(NULL, " ");
            i++;
		}

        if ((strcmp(value[0], "listar")) == 0) {
            listar(args.ns);
        }

		if ((strcmp(value[0], "encerrar")) == 0) {
            encerrar(args.ns, args.s, args.thread_id);
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
		parameters.s = s;
		parameters.thread_id = countClients;

		if (pthread_create(&thread_id[countClients], NULL, recebe_comando, (void* )&parameters))
        {
            printf("ERRO: impossivel criar uma thread\n");
            exit(-1);
        }
		printf("Thread[%i] criada\n", countClients);
		countClients++;
        // pthread_detach(thread_id[countClients]);
	}
}
