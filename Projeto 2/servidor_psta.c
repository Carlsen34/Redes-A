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
#include <netdb.h>
#include <sys/stat.h>


/*
 * Servidor TCP
 */

#define commandSizes 200
#define MaxArray 10
#define FILESIZE 4096
#define DIGITOSTELEFONE 9
#define STRINGSIZE 90

struct args{
	int ns;
	int s;
	int thread_id;
};

struct cliente {
	char telefone[DIGITOSTELEFONE];
	char ip[STRINGSIZE];
	int porta;
	int thread_id;
};

struct args parameters;
pthread_t thread_id[MaxArray];
struct cliente contatos[MaxArray];
unsigned short port;
int countClients = 0;
int countContatos = 0;
int s_file, ns_file;

char command[commandSizes];

void print_contatos() {
	for (int i = 0; i < MaxArray; i++) {
		printf("%i) PORTA: %d - TELEFONE: %s - IP: %s\n", contatos[i].thread_id, contatos[i].porta, contatos[i].telefone, contatos[i].ip);
	}
}

int conectar_file(char hostname[], int porta) {
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(hostname);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) (porta);

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s_file = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if ((connect(s_file, (struct sockaddr *)&server, sizeof(server))) < 0)
    {
        perror("Connect()");
        exit(4);
    }

	printf("hostname: %s\n", hostname);
    printf("porta: %i\n", porta);


    return s_file;
}

long file_size(char *name) {
    FILE *fp = fopen(name, "rb"); //must be binary read to get bytes

    long size= 0 ;
    if(fp)
    {
        fseek (fp, 0, SEEK_END);
        size = ftell(fp);
        fclose(fp);
    }
    return size;
}


void encerrar(int ns, int s, int thread_id) {

	for (int i = 0; i < MaxArray; i++) {
		if (contatos[i].thread_id == thread_id) {
			strcpy(contatos[i].telefone, "");
			strcpy(contatos[i].ip, "");
			contatos[i].porta = -1;
			contatos[i].thread_id = -1;
			countContatos--;
			printf("Thread[%i] esta na porta %i e possui o telefone %s\n", contatos[i].thread_id, contatos[i].porta, contatos[i].telefone);
		}
	};

	close(ns);
	print_contatos();
	printf("Thread[%d] has finished ...\n", thread_id);
	pthread_exit(NULL);
}

void listar_contatos(int ns, int thread_id) {
	struct controle_status {
        char telefone[DIGITOSTELEFONE];
        char status[10];
    };

    struct controle_status status;
	char telefone[DIGITOSTELEFONE];
	int countContatosCliente = 0;

	if (recv(ns, &countContatosCliente, sizeof(countContatosCliente), 0) == -1) {
		perror("Recv()");
		exit(6);
	}

	printf("countContatosCliente: %i\n", countContatosCliente);	

	for(int i = 0; i < countContatosCliente; i++) {

		if (recv(ns, telefone, sizeof(telefone), 0) == -1) {
			perror("Recv()");
			exit(6);
		}

		printf("telefone[%i]: %s - %lu\n", i, telefone, sizeof(telefone));

		int j = 0; 
		do {
			if ((strcmp(telefone, contatos[j].telefone)) == 0 && (contatos[j].porta != -1)) {	
				strcpy(status.status, "Online");
				strcpy(status.telefone, telefone);
				j++;
			} else {
				strcpy(status.status, "Offline");
				strcpy(status.telefone, telefone);
				j++;
			}
		} while(j < MaxArray && (strcmp(status.status, "Online") != 0));

		// printf("%i) Telefone: %s - %s\n", i, status.telefone, status.status);

		if (send(ns, &status, (sizeof(status)), 0) < 0) {
			perror("Send() 3");
			exit(7);
		}
	}
};

void adicionar_contato(int ns) {
	struct cliente aux_cliente;
	bool alteracao = false;

	// recebe a as infomacoes do cliente como a porta e o telefone
	if (recv(ns, &aux_cliente, sizeof(aux_cliente), 0) == -1)
	{
		perror("Recv()");
		exit(6);
	}

	strcpy(contatos[countContatos].telefone, aux_cliente.telefone);
	strcpy(contatos[countContatos].ip, "0");
	contatos[countContatos].porta = 0;
	contatos[countContatos].thread_id = 11;
	countContatos++;

};

void setup_array_contatos() {
	for (int i = 0; i < MaxArray; i++) {
		strcpy(contatos[i].telefone, "");
		strcpy(contatos[i].ip, "");
		contatos[i].porta = -1;
		contatos[i].thread_id = -1;
	}
}

void setup_contato(int ns, int thread_id) {
	struct controle_status {
        char telefone[DIGITOSTELEFONE];
        char status[10];
    };
	struct controle_status status;
	strcpy(status.status, "ok");

	struct cliente aux_cliente;
	bool alteracao = false;


	do { 
		strcpy(status.status, "ok");
		// recebe a as infomacoes do cliente como a porta e o telefone
		if (recv(ns, &aux_cliente, sizeof(aux_cliente), 0) == -1)
		{
			perror("Recv()");
			exit(6);
		}

		// verifica se nao possui nenhum contato com esse telefone
		for (int j = 0; j < MaxArray; j++) {
				if ((strcmp(contatos[j].telefone, aux_cliente.telefone)) == 0) {
					strcpy(status.status, "erro");
					strcpy(status.telefone, aux_cliente.telefone);
				}
		}

		if (strcmp(status.status, "ok") == 0) {
			int i = 0;
			do {
				printf("Thread[%i] esta na porta %i e possui o telefone %s\n", contatos[i].thread_id, contatos[i].porta, contatos[i].telefone);
				if ((strcmp(contatos[i].telefone, "")) == 0 && (contatos[i].porta == -1)) {
					strcpy(contatos[i].telefone, aux_cliente.telefone);
					strcpy(contatos[i].ip, aux_cliente.ip);
					contatos[i].porta = aux_cliente.porta;
					contatos[i].thread_id = thread_id;
					strcpy(status.telefone, aux_cliente.telefone);
					alteracao = true;
					countContatos++;
				}
				i++;

			}while((i < MaxArray) && (!alteracao));
			printf("Thread[%i] esta na endereco %s:%i e possui o telefone %s\n", thread_id, aux_cliente.ip, aux_cliente.porta, aux_cliente.telefone);
		}

		if (send(ns, &status, sizeof(status), 0) == -1) {
			perror("Recv()");
			exit(6);
		}

		print_contatos();
	} while(strcmp(status.status, "ok") != 0);
};

void chat(int ns, int thread_id) {
	struct contato {
		char telefone[DIGITOSTELEFONE];
		char ip[STRINGSIZE];
		int porta;
	};

	struct contato aux_contato;
	char telefone[DIGITOSTELEFONE];
	char status[10] = "";
	int i = 0;

	if (recv(ns, &telefone, sizeof(telefone), 0) == -1) {
		perror("Recv()");
		exit(6);
	}
		printf("Texto ... TEL: %s\n", telefone);

	do {
		printf("%i - status: %s", i, status);
		printf("DoWhile ... TEL: %s\n", contatos[i].telefone);
		if ((strcmp(telefone, contatos[i].telefone)) == 0) {	
			printf("ENTROU AQUI -> %i - status: %s", i, status);
			strcpy(status, "stop");

			strcpy(aux_contato.telefone, contatos[i].telefone);
			strcpy(aux_contato.ip, contatos[i].ip);
			aux_contato.porta = contatos[i].porta;

			printf("Telefone: %s\n", aux_contato.telefone);
			printf("Porta: %i\n", aux_contato.porta);
			printf("IP: %s\n", aux_contato.ip);

			if (send(ns, &aux_contato, (sizeof(aux_contato)), 0) < 0) {
				perror("Send() 3");
				exit(7);
			}
			i++;
		}
		i++;
	} while(i < MaxArray && (strcmp(status, "stop") != 0));

	if((strcmp(status, "stop") != 0)) {
		//default
		strcpy(aux_contato.telefone, "00000000");
		strcpy(aux_contato.ip, "0");
		aux_contato.porta = 0;

		if (send(ns, &aux_contato, (sizeof(aux_contato)), 0) < 0) {
			perror("Send() 3");
			exit(7);
		}
	}
}

void *recebe_comando(void* parameters){
	struct args args = *((struct args*) parameters);
	setup_contato(args.ns, args.thread_id);
    bool variavelLoop = false;
	int comando;

    do {
        /* Recebe uma mensagem do cliente atraves do novo socket conectado */
		// memset(&comando, 0, sizeof(comando));
		if (recv(args.ns, &comando, sizeof(comando), 0) == -1)
        {
            perror("Recv()");
            exit(6);
        }

		switch (comando) {
        case 1:
			printf("entrou aqui ...");
            // adicionar_contato(args.ns);
            break;

        case 2:
            listar_contatos(args.ns, args.thread_id);
            break;
        
        case 3:
			chat(args.ns, args.thread_id);
			printf("VOLTOU DO CHAT ...\n");
            break;

        case 5:
			encerrar(args.ns, args.s, args.thread_id);
            variavelLoop = false;
            break;

        default:
            printf("Comando invalido ... Por favor insira um novo comando\n");
            break;
        }

    } while(!variavelLoop);
	return NULL;
}

int main(int argc, char **argv){
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

	setup_array_contatos();

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

		if (pthread_create(&thread_id[countClients], NULL, recebe_comando, (void* )&parameters)) {
            printf("ERRO: impossivel criar uma thread\n");
            exit(-1);
        }
		printf("Thread[%i] criada\n", countClients);
		countClients++;
        // pthread_detach(thread_id[countClients]);
	}
}
