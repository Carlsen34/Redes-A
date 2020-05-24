/*
Herick Valsecchi Carlsen 15159619
João Pedro Favara 16061921
Raissa Furlan Davinha 15032006
Leonardo Blanco Natis 15296858
Kaíque Ferreira Fávero 15118698
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>

/*
 * Cliente TCP
 */
 
#define COMMAND 200
#define FILESIZE 4096
#define DIGITOSTELEFONE 9
#define STRINGSIZE 90

struct dados_cliente {
	char telefone[DIGITOSTELEFONE];
    char ip[STRINGSIZE];
	int porta;
};

struct agenda {
    char conteudo[DIGITOSTELEFONE]; 
    struct agenda *prox;
};

int s;

//variaveis para a criacao do socket de dados
char ip[STRINGSIZE]; // buffer temporario para string
int s_dados, ns_dados, port_dados;
struct sockaddr_in client_dados; 
struct sockaddr_in server_dados; 
struct dados_cliente cliente;
int namelen_dados;

char command[COMMAND];

long file_size(char *name) {
    FILE *fp = fopen(name, "a+"); //must be binary read to get bytes

    long size;
    if(fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fclose(fp);
    }
    return size;
}

void create_socket() {
    // int s_dados;                     /* Socket para aceitar conexoes       */
	// int *ns_dados = 0;                /* Socket conectado ao cliente        */
    unsigned short port;

    /*
     * O primeiro argumento (argv[1]) e a porta
     * onde o servidor aguardara por conexoes
     */

    port = (unsigned short) 0;

    /*
     * Cria um socket TCP (stream) para aguardar conexoes
     */
    if ((s_dados = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
	  perror("Socket()");
	  exit(2);
    }

   /*
    * Define a qual endereco IP e porta o servidor estara ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os enderecos IP
    */
    server_dados.sin_family = AF_INET;   
    server_dados.sin_port   = htons(port);       
    server_dados.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor a porta definida anteriormente.
     */
    if (bind(s_dados, (struct sockaddr *)&server_dados, sizeof(server_dados)) < 0)
    {
		perror("Bind() aqui ... ");
		exit(3);
    }

    /* Consulta qual porta foi utilizada. */
    namelen_dados = sizeof(server_dados);
    if (getsockname(s_dados, (struct sockaddr *) &server_dados, &namelen_dados) < 0) {
        perror("getsockname()");
        exit(1);
    }

	/* Imprime qual porta E IP foram utilizados. */
    port_dados = ntohs(server_dados.sin_port);
    printf("\nPorta utilizada (enviar): %d", ntohs(server_dados.sin_port));
    printf("\nIP utilizado (enviar): %d\n", ntohs(server_dados.sin_addr.s_addr));

    /*
     * Prepara o socket para aguardar por conexoes e
     * cria uma fila de conexoes pendentes.
     */
    if (listen(s_dados, 1) != 0)
    {
	  perror("Listen()");
	  exit(4);
    }


    //  while(ns_dados == 0)
    // {
	//   /*
	//   * Aceita uma conexao e cria um novo socket atraves do qual
	//   * ocorrera a comunicacao com o cliente.
	//   */
	//   namelen_dados = sizeof(client_dados);
	//   if ((ns_dados = accept(s_dados, (struct sockaddr *) &client_dados, (socklen_t *) &namelen_dados)) == -1) {
	// 	perror("Accept()");
	// 	exit(5);
	//   }
	// }

    // return ntohs(server_dados.sin_port);
}

void enviar(char comando[], char nome_local[], char nome_remoto[]) {
    FILE *fp;
    char str_aux[COMMAND];
    char bufsize[100];
    char file_name[strlen(nome_local)+1];
    int i, len, accum;
    long sizeFile = file_size(nome_local);
    char line[FILESIZE];

    strcpy(str_aux, comando);
    // enviar o comando enviar ...
    if (send(s, &str_aux, (sizeof(str_aux)), 0) < 0) {
        perror("Send() 1");
        exit(5);
    }

    // envia o nome que o arquivo sera salvo
    strcpy(str_aux, nome_remoto);
    if (send(s, &str_aux, (sizeof(str_aux)), 0) < 0) {
        perror("Send() 2");
        exit(5);
    }

    //enviar a porta onde sera conectado o servidor ...
    if (send(s, &port_dados, (sizeof(port_dados)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    while(ns_dados == 0)
    {
	  /*
	  * Aceita uma conexao e cria um novo socket atraves do qual
	  * ocorrera a comunicacao com o cliente.
	  */
	  namelen_dados = sizeof(client_dados);
	  if ((ns_dados = accept(s_dados, (struct sockaddr *) &client_dados, (socklen_t *) &namelen_dados)) == -1) {
		perror("Accept()");
		exit(5);
	  }
	}

    // le o nome do arquivo
    snprintf(file_name, strlen(nome_local)+1, "%s", nome_local);
    fp = fopen(file_name, "rb");
    // printf("file_name: %s\n", file_name);

    // enviar qual o tamanho do arquivo
    if (send(ns_dados, &sizeFile, (sizeof(sizeFile)), 0) < 0) {
        perror("Send() 3");
        exit(5);
    }

    if(fp) {
        int sent_bytes = 0;
        while((sent_bytes = fread(line, 1, FILESIZE, fp)) && (sizeFile > 0)) {
            // printf("sizeFile: %li - %i\n", sizeFile, sent_bytes);
            sizeFile -= sent_bytes;

            if (send(ns_dados, &line, sent_bytes, 0) < 0) {
                perror("Send()");
                exit(5);
            }
            memset(line, 0, sizeof(line));
        }
        printf("Finalizou o processo de envio ... \n");
    }

    fclose(fp);
    close(ns_dados);
    ns_dados = 0;
    printf("Fechou tudo!\n");
}

void receber(char comando[], char nome_remoto[], char nome_local[]) {
    FILE *fp;
    char str_aux[COMMAND];
    char bufsize[100];
    char file_name[200];
    int i, len, accum;
    long sizeFile = 0;
    char line[FILESIZE];

    strcpy(str_aux, comando);
    // enviar o comando receber ...
    if (send(s, &str_aux, (sizeof(str_aux)), 0) < 0) {
        perror("Send() 1");
        exit(5);
    }

    // envia o nome que o arquivo sera salvo
    strcpy(str_aux, nome_remoto);
    printf("str_aux: %s\n", str_aux);
    if (send(s, &str_aux, (sizeof(str_aux)), 0) < 0) {
        perror("Send() 2");
        exit(5);
    }

    //enviar a porta para o servidor se conectar
    printf("port: %i\n", port_dados);
    if (send(s, &port_dados, (sizeof(port_dados)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    while(ns_dados == 0)
    {
	  /*
	  * Aceita uma conexao e cria um novo socket atraves do qual
	  * ocorrera a comunicacao com o cliente.
	  */
	  namelen_dados = sizeof(client_dados);
	  if ((ns_dados = accept(s_dados, (struct sockaddr *) &client_dados, (socklen_t *) &namelen_dados)) == -1) {
		perror("Accept()");
		exit(5);
	  }
	}
    
    // recebe qual o tamanho do arquivo
    if (recv(ns_dados, &sizeFile, (sizeof(sizeFile)), 0) < 0) {
        perror("Send() 3");
        exit(5);
    }
    // printf("sizeFile: %li\n", sizeFile);

    // le o nome do arquivo
    // printf("nome_local: %s\n", nome_local);
    snprintf(file_name, strlen(nome_local), "%s", nome_local);
    // printf("file_name: %s\n", file_name);
    // printf("(sizeof(line)): %lu\n", (sizeof(line)));
    fp = fopen(file_name, "wb");

    if(fp) {
		accum = 0; // quantidade de dados acumuladods
		int sent_bytes = 0; // quantidade de dados recebidos
		while(accum < sizeFile) {
			if ((sent_bytes = recv(ns_dados, &line, (sizeof(line)), 0)) < 0) {
				perror("Send() 4");
				exit(5);
			}

            if(sent_bytes > 0) {
                printf("sent_bytes = %i\n", sent_bytes);
            }

			accum += sent_bytes;//track size of growing file

			fwrite(line, sent_bytes, 1, fp);
		}
	}

    fclose(fp);
    close(ns_dados);
    ns_dados = 0;
    printf("Fechou tudo!\n");
}

void encerrar() {
    close(s);
    close(s_dados);
    printf("Finalizando o cliente ...\n");
    exit(0);
}

void insere_lista(char telefone[], struct agenda *a) {
    struct agenda *atual, *proximo;
    struct agenda *novo;

    atual = a;
    proximo = a->prox;

    novo = malloc (sizeof (struct agenda));
    strcpy(novo->conteudo, telefone);
    printf("novo->conteudo: %s\n", novo->conteudo);

    while (atual->prox != NULL) {
        atual = proximo;
        proximo = atual->prox;
    }

    if (proximo == NULL) {
        novo->prox = atual->prox;
        atual->prox = novo;
    }
};

void listar_contatos() {
    FILE *fp;
    char nomeArquivo[20], file_name[20], line[10], status[10];
    int countTelefone = 0;

    struct agenda *contato_agenda;
    contato_agenda = malloc (sizeof (struct agenda));
    contato_agenda->prox = NULL;

    strcpy(nomeArquivo, cliente.telefone);
    strcat(nomeArquivo, ".txt");
    snprintf(file_name, (strlen(nomeArquivo) + 1), "%s", nomeArquivo);// pega o nome do arquivo de um uma variavel
    long int size = file_size(file_name);

    if (size == 0) {
        printf("Nao possui contatos cadastrados ...\n");
    } else {
        fp = fopen(file_name, "r");// abre o arquivo
        printf("TESTEETEETE ...\n");
        while(fgets(line,1024,fp)) {
            printf("%s\n",line);
            insere_lista(line, contato_agenda);
            countTelefone++;
        };

        fclose(fp);

        if (send(s, &countTelefone, (sizeof(countTelefone)), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        struct agenda *print;
        print = contato_agenda->prox;
        for(int i = 0; i < countTelefone; i++) {

            if (send(s, &print->conteudo, (sizeof(print->conteudo)), 0) < 0) {
                perror("Send() 1");
                exit(5);
            }

            if (recv(s, &status, (sizeof(status)), 0) < 0) {
                perror("Recv()");
                exit(6);
            }

            printf("%i) %s - %s\n", i+1, print->conteudo, status);

            print = print->prox;
        }

        free(contato_agenda);
        free(print);
    }

}

void listar(const char list_command[]) {
    char comando_listar[COMMAND], nomeFile[256];
    DIR *dir;
    struct dirent *dp;
    int count = 0;

    //envia o comando "listar"
    strcpy(comando_listar, list_command);
    if (send(s, &comando_listar, (sizeof(comando_listar)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    printf("port: %i\n", port_dados);
    // enviar para o servidor a porta q ocorerra a comunicacao com o servidor
    if (send(s, &port_dados, (sizeof(port_dados)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    while(ns_dados == 0)
    {
	  /*
	  * Aceita uma conexao e cria um novo socket atraves do qual
	  * ocorrera a comunicacao com o cliente.
	  */
	  namelen_dados = sizeof(client_dados);
	  if ((ns_dados = accept(s_dados, (struct sockaddr *) &client_dados, (socklen_t *) &namelen_dados)) == -1) {
		perror("Accept()");
		exit(5);
	  }
	}

    // Recebe a lista de arquivos que estao no servidor...
    do {
        if (recv(ns_dados, &nomeFile, (sizeof(nomeFile)), 0) < 0)
        {
            perror("Recv()");
            exit(6);
        }

        if((strcmp(nomeFile, "stop")) != 0) {
            printf("> %s\n", nomeFile);
        }

    }while((strcmp(nomeFile, "stop")) != 0);

    //close(s_dados);
    close(ns_dados);
    ns_dados = 0;
}

void conectar(char hostname[], char porta[]) {
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;

    // printf("hostname: %s\n", hostname);
    // printf("porta: %s\n", porta);

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(hostname);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(porta);

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    inet_ntop(AF_INET,&server.sin_addr,ip,STRINGSIZE);
    printf("server.sin_addr.s_addr: %s\n", ip);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }
}

void adicionar_contato() {
    FILE *fp;
    char telefone[8];
    char nomeArquivo[20], file_name[20];

    create_socket();
    printf("Insira o telefone do seu novo contato - 8 digitos: ");

    fpurge(stdin);
    fgets(telefone,sizeof(telefone),stdin);

    strcpy(nomeArquivo, cliente.telefone);
    strcat(nomeArquivo, ".txt");
    snprintf(file_name, (strlen(nomeArquivo) + 1), "%s", nomeArquivo);// pega o nome do arquivo de um uma variavel
    fp = fopen(file_name, "a");// abre o arquivo

    fprintf(fp, "%s", telefone);
	fclose(fp);

    printf("Novo contato salvo com sucesso!\n");
};

void criar_contato() {
    char telefone[DIGITOSTELEFONE];

    create_socket();
    printf("Insira o seu telefone - 8 digitos: ");

    fpurge(stdin);
    fgets(telefone,sizeof(telefone),stdin);

    cliente.porta = port_dados;
    strcpy(cliente.telefone, telefone);
    strcpy(cliente.ip, ip);

    printf("PORTA: %d\n", cliente.porta);
	printf("TELEFONE: %s\n", cliente.telefone);
	printf("IP: %s\n", cliente.ip);

    // enviar para o servidor a porta do socket de dados e do telefone
    if (send(s, &cliente, (sizeof(cliente)), 0) < 0) {
        perror("Send() 3");
        exit(5);
    }

    // printf("Insira o seu telefone: ");
};

// MAIN FUNCTION
int main(int argc, char **argv){
    conectar(argv[1], argv[2]);
    criar_contato();
    bool variavelLoop = true;
    int comando;

    do {

        variavelLoop = false;
        printf("MENU:\n"
            "1 - Adicionar novo contato\n"
            "2 - Ver contatos\n"
            "3 - Enviar mensagem\n"
            "4 - Sair da aplicacao\n\n"
            "Digite o numero da opcao: ");

        // memset(comando, 0, sizeof(comando));
        fpurge(stdin);
        scanf("%i", &comando);

        switch (comando) {
        case 1:
            // if (send(s, &comando, (sizeof(comando)), 0) < 0) {
            //     perror("Send() 1");
            //     exit(5);
            // }

            adicionar_contato();
            break;

        case 2:
            if (send(s, &comando, (sizeof(comando)), 0) < 0) {
                perror("Send() 1");
                exit(5);
            }
            listar_contatos();
            break;
        
        case 3:
            printf("Comando 3 ...\n");
            break;

        case 4:
            if (send(s, &comando, (sizeof(comando)), 0) < 0) {
                perror("Send()");
                exit(5);
            }
            encerrar();
            variavelLoop = false;
            break;

        default:
            printf("Comando invalido ... Por favor insira um novo comando\n");
            break;
        }

    } while(!variavelLoop);

    /* Fecha o socket */
    close(s);
    printf("Cliente terminou com sucesso.\n");
    exit(0);
}
