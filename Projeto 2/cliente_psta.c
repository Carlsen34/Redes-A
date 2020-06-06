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
#include <pthread.h>

/*
 * Cliente TCP
 */
 
#define COMMAND 200
#define FILESIZE 4096
#define DIGITOSTELEFONE 9
#define STRINGSIZE 90
#define TAMANHOMENSAGEM 200

struct dados_cliente {
	char telefone[DIGITOSTELEFONE];
    char ip[STRINGSIZE];
	int porta;
};

struct agenda {
    char conteudo[DIGITOSTELEFONE]; 
    struct agenda *prox;
};

struct mensagem_dados {
    char telefone_remetente[DIGITOSTELEFONE];
    char conteudo[TAMANHOMENSAGEM];
    char type[10];
    long tamanho;
};

struct grupos {
    char conteudo[20];
    struct grupos *prox;
};

int s_server;
int s_cliente , ns_cliente = 0;

//variaveis para a criacao do socket de dados
char ip[STRINGSIZE]; // buffer temporario para string
int s_dados, ns_dados, port_dados;
pthread_t thread_ServerCliente;
struct sockaddr_in client_dados, cliente_mensagem; 
struct sockaddr_in server_dados; 
struct dados_cliente cliente;
int namelen_dados, namelen_mensagem;

char path[106], nome_pasta[106];
struct stat st = {0};

char command[COMMAND];

long file_size(char *name) {
    FILE *fp = fopen(name, "ar"); //must be binary read to get bytes

    long size;
    if(fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fclose(fp);
    }
    return size;
}

void criar_pasta_dados() {
    strcpy(nome_pasta, cliente.telefone);
    strcat(nome_pasta, "_dados");
    strcat(path, "/");
    strcat(path, nome_pasta);

    // cria o diretorio de dados
    if (stat(path, &st) == -1) {
        mkdir(nome_pasta, 0777);
    }
}

void enviar(struct mensagem_dados arquivo) {
    FILE *fp;
    char str_aux[COMMAND];
    char nameFolder[100];
    char file_name[200];
    int i, len, accum;
    long sizeFile = arquivo.tamanho;
    char line[FILESIZE];

    // char path[106], nome_pasta[106];
    // struct stat st = {0};

    // strcpy(nome_pasta, cliente.telefone);
    // strcat(nome_pasta, "_dados");
    // strcat(path, "/");
    // strcat(path, nome_pasta);

    // // cria o diretorio de dados
    // if (stat(path, &st) == -1) {
    //     mkdir(nome_pasta, 0777);
    // }

    strcpy(nameFolder, nome_pasta);
    strcat(nameFolder, "/");
    strcat(nameFolder, arquivo.conteudo);
    printf("nome_pasta: %s\n\n", nameFolder);


    // le o nome do arquivo
    snprintf(file_name, strlen(nameFolder)+1, "%s", nameFolder);
    fp = fopen(file_name, "rb");
    printf("file_name: %s\n", file_name);


    if(fp) {
        int sent_bytes = 0;
        while((sent_bytes = fread(line, 1, FILESIZE, fp)) && (sizeFile > 0)) {
            // printf("sizeFile: %li - %i\n", sizeFile, sent_bytes);
            sizeFile -= sent_bytes;

            if (send(s_cliente, &line, sent_bytes, 0) < 0) {
                perror("Send()");
                exit(5);
            }
            memset(line, 0, sizeof(line));
        }
        printf("Finalizou o processo de envio ... \n");
    }

    fclose(fp);
    // close(ns);
    printf("Fechou tudo!\n");
}

void receber_arquivo(struct mensagem_dados arquivo) {
    FILE *fp;
    char str_aux[COMMAND];
    char nameFolder[100];
    char file_name[200];
    int i, len, accum;
    long sizeFile = arquivo.tamanho;
    char line[FILESIZE];

    // char path[106], nome_pasta[106];
    // struct stat st = {0};

    // strcpy(nome_pasta, cliente.telefone);
    // strcat(nome_pasta, "_dados");
    // strcat(path, "/");
    // strcat(path, nome_pasta);

    // // cria o diretorio de dados
    // if (stat(path, &st) == -1) {
    //     mkdir(nome_pasta, 0777);
    // }

    strcpy(nameFolder, nome_pasta);
    strcat(nameFolder, "/");
    strcat(nameFolder, arquivo.conteudo);
    printf("nome_pasta: %s\n\n", nameFolder);

    // printf("sizeFile: %li\n", sizeFile);

    // le o nome do arquivo
    // printf("nome_local: %s\n", nome_local);
    snprintf(file_name, strlen(nameFolder)+1, "%s", nameFolder);
    printf("file_name: %s\n", file_name);
    // printf("(sizeof(line)): %lu\n", (sizeof(line)));
    fp = fopen(file_name, "wb");

    if(fp) {
		accum = 0; // quantidade de dados acumuladods
		int sent_bytes = 0; // quantidade de dados recebidos
		while(accum < sizeFile) {
			if ((sent_bytes = recv(ns_cliente, &line, (sizeof(line)), 0)) < 0) {
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
    close(ns_cliente);
    printf("Fechou tudo!\n");
}

void *thread_server() {
    struct mensagem_dados tipo_mensagem;
    while(true) {
        /*
        * Aceita uma conexao e cria um novo socket atraves do qual
        * ocorrera a comunicacao com o cliente.
        */
        namelen_mensagem = sizeof(cliente_mensagem);
        if ((ns_cliente = accept(s_dados, (struct sockaddr *) &cliente_mensagem, (socklen_t *) &namelen_mensagem)) == -1) {
            perror("Accept() 1234");
            exit(5);
        }

        if (recv(ns_cliente, &tipo_mensagem, (sizeof(tipo_mensagem)), 0) < 0) {
            perror("Recv() xxxx ");
            exit(6);
        }

        if ((strcmp(tipo_mensagem.type, "mensagem") == 0)) {
            printf("\0337\n\033[B");
            printf("\033[L**************** y *****************\n");
            printf("\033[LRemetente: %s\n",tipo_mensagem.telefone_remetente);
            printf("\033[LMensagem: %s\n", tipo_mensagem.conteudo);
            printf("\033[L***************** x  ****************\n");
            printf("\0338\033[5A");
            fflush(stdout);
            // fflush(stdout);
            // printf("Mensagem Recebida ...\n\n");

            // printf("Remetente: %s\n", tipo_mensagem.telefone_remetente);
            // printf("Mensagem: %s\n", tipo_mensagem.conteudo);
        }
        
        if ((strcmp(tipo_mensagem.type, "arquivo") == 0)){
            printf("Arquivo Recebido ...\n\n");
            printf("Remetente: %s\n", tipo_mensagem.telefone_remetente);
            printf("Conteudo: %s\n", tipo_mensagem.conteudo);
            printf("Tamanho: %li\n\n", tipo_mensagem.tamanho);
            receber_arquivo(tipo_mensagem);
        }
	}
}

void encerrar() {
    pthread_cancel(thread_ServerCliente);
    close(ns_cliente);
    close(ns_dados);
    close(s_dados);
    close(s_server);
    printf("Finalizando o cliente ...\n");
    exit(0);
}

void insere_lista_grupos(char telefone[], struct grupos *a) {
    struct grupos *atual, *proximo;
    struct grupos *novo;

    atual = a;
    proximo = a->prox;

    novo = malloc (sizeof (struct grupos));
    strcpy(novo->conteudo, telefone);
    // printf("novo->conteudo: %s\n", novo->conteudo);

    while (atual->prox != NULL) {
        // printf("atual->conteudo: %s\n", atual->conteudo);
        atual = proximo;
        proximo = atual->prox;
    }

    if (proximo == NULL) {
        // printf("atual->conteudo: %s\n", atual->conteudo);
        novo->prox = atual->prox;
        atual->prox = novo;
    }
};

void insere_lista(char telefone[], struct agenda *a) {
    struct agenda *atual, *proximo;
    struct agenda *novo;

    atual = a;
    proximo = a->prox;

    novo = malloc (sizeof (struct agenda));
    strcpy(novo->conteudo, telefone);
    // printf("novo->conteudo: %s\n", novo->conteudo);

    while (atual->prox != NULL) {
        // printf("atual->conteudo: %s\n", atual->conteudo);
        atual = proximo;
        proximo = atual->prox;
    }

    if (proximo == NULL) {
        // printf("atual->conteudo: %s\n", atual->conteudo);
        novo->prox = atual->prox;
        atual->prox = novo;
    }
};

void listar_contatos() {
    struct controle_status {
        char telefone[DIGITOSTELEFONE];
        char status[10];
    };

    struct controle_status status;
    FILE *fp;
    char str[DIGITOSTELEFONE];
    size_t len;
    char *line = NULL;
    char nomeArquivo[20], file_name[20];
    int countTelefone = 0, read = 0;

    struct agenda *contato_agenda;
    contato_agenda = malloc (sizeof (struct agenda));
    contato_agenda->prox = NULL;
    strcpy(contato_agenda->conteudo, "0000");


    strcpy(nomeArquivo, cliente.telefone);
    strcat(nomeArquivo, ".txt");
    snprintf(file_name, (strlen(nomeArquivo) + 1), "%s", nomeArquivo);// pega o nome do arquivo de um uma variavel
    printf("file_name: %s\n", file_name);
    long int size = file_size(file_name);
    printf("size: %ld\n", size);

    if (size == 0) {
        printf("Nao possui contatos cadastrados ...\n");
    } else {
        fp = fopen(file_name, "r");// abre o arquivo

        int comando = 2;
        if (send(s_server, &comando, (sizeof(comando)), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        while ((read = getline(&line, &len, fp)) != -1) {
            strtok(line, "\n");
            insere_lista(line, contato_agenda);
            countTelefone++;
        };

        fclose(fp);

        if (send(s_server, &countTelefone, sizeof(countTelefone), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        struct agenda *print;
        print = contato_agenda->prox;
        for(int i = 0; i < countTelefone; i++) {
            char conteudo[DIGITOSTELEFONE];
            strcpy(conteudo, print->conteudo);

            // printf("print->conteudo[%i]: %s - %lu\n", i, conteudo, sizeof(conteudo));
            if (send(s_server, conteudo, sizeof(conteudo), 0) < 0) {
                perror("Send() 1");
                exit(5);
            }

            if (recv(s_server, &status, sizeof(status), 0) < 0) {
                perror("Recv()");
                exit(6);
            }

            if ((strcmp(status.status, "Online")) == 0) {
                printf("\033[0;32m %i) %s - %s\n", i+1, status.telefone, status.status);
                printf("\033[0;37m");
            } else {
                printf("\033[0;31m %i) %s - %s\n", i+1, status.telefone, status.status);
                printf("\033[0;37m");
            }

            print = print->prox;
        }

        free(contato_agenda);
        free(print);
    }

}

void conectar(char hostname[], char porta[], int *s) {
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;

    printf("hostname: %s\n", hostname);
    printf("porta: %s\n", porta);

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

    inet_ntop(AF_INET, &server.sin_addr, ip, STRINGSIZE);
    printf("server.sin_addr.s_addr: %s\n", ip);

    /*
     * Cria um socket TCP (stream)
     */
    if ((*s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if (connect(*s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }
}

void enviar_mensagem() {
    char telefone_mensagem[DIGITOSTELEFONE];
    struct dados_cliente dados;
    char mensagem[TAMANHOMENSAGEM];
    char porta[32];

    printf("Digite o telefone para enviar a mensagem:\n");
    fpurge(stdin);
    fgets(telefone_mensagem,sizeof(telefone_mensagem),stdin);
    strtok(telefone_mensagem, "\n");

    if (send(s_server, &telefone_mensagem, (sizeof(telefone_mensagem)), 0) < 0) {
        perror("Send() 1");
        exit(5);
    }

    if (recv(s_server, &dados, (sizeof(dados)), 0) < 0) {
        perror("Recv()");
        exit(6);
    }

    // printf("Telefone: %s\n", dados.telefone);
    // printf("Porta: %i\n", dados.porta);
    // printf("IP: %s\n", dados.ip);

    if (dados.porta == 0) {
        printf("Contato offline ... Tente mais tarde\n");
    } else {
        printf("Digite a mesnagem:\n");
        fpurge(stdin);
        fgets(mensagem,sizeof(mensagem),stdin);
        strtok(mensagem, "\n");

        struct mensagem_dados mensagem_dados;
        strcpy(mensagem_dados.type, "mensagem");
        strcpy(mensagem_dados.telefone_remetente, cliente.telefone);
        strcpy(mensagem_dados.conteudo, mensagem);

        sprintf(porta, "%i", dados.porta);
        conectar(dados.ip, porta, &s_cliente);

        if (send(s_cliente, &mensagem_dados, (sizeof(mensagem_dados)), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        close(s_cliente);

        // printf("Texto ... TEL: %s\nMENSAGEM: %s\n", telefone_mensagem, mensagem);
    }
}

void enviar_arquivo() {
    char telefone_arquivo[DIGITOSTELEFONE];
    struct dados_cliente dados;
    char file_name[200];
    char arquivo[100];
    char path[106], nome_pasta[106];
    struct stat st = {0};
    char porta[32];

    printf("Digite o telefone para enviar o arquivo:\n");
    fpurge(stdin);
    fgets(telefone_arquivo,sizeof(telefone_arquivo),stdin);
    strtok(telefone_arquivo, "\n");

    if (send(s_server, &telefone_arquivo, (sizeof(telefone_arquivo)), 0) < 0) {
        perror("Send() 1");
        exit(5);
    }

    if (recv(s_server, &dados, (sizeof(dados)), 0) < 0) {
        perror("Recv()");
        exit(6);
    }

    printf("Telefone: %s\n", dados.telefone);
    printf("Porta: %i\n", dados.porta);
    printf("IP: %s\n", dados.ip);

    if (dados.porta == 0) {
        printf("Contato offline ... Tente mais tarde\n");
    } else {

        strcpy(nome_pasta, cliente.telefone);
        strcat(nome_pasta, "_dados");
        strcat(path, "/");
        strcat(path, nome_pasta);

        // cria o diretorio de dados
        if (stat(path, &st) == -1) {
            mkdir(nome_pasta, 0777);
        }

        printf("Digite o nome do arquivo:\n");
        fpurge(stdin);
        fgets(arquivo,sizeof(arquivo),stdin);
        strtok(arquivo, "\n");

        strcat(nome_pasta, "/");
        strcat(nome_pasta, arquivo);
        printf("nome_pasta: %s\n\n", nome_pasta);
        snprintf(file_name, strlen(nome_pasta)+1, "%s", nome_pasta);
        long size_file = file_size(file_name);
	    printf("size_file: %li\n", size_file);

        struct mensagem_dados arquivo_dados;
        strcpy(arquivo_dados.type, "arquivo");
        strcpy(arquivo_dados.telefone_remetente, cliente.telefone);
        strcpy(arquivo_dados.conteudo, arquivo);
        arquivo_dados.tamanho = size_file;

        sprintf(porta, "%i", dados.porta);
        conectar(dados.ip, porta, &s_cliente);

        if (send(s_cliente, &arquivo_dados, (sizeof(arquivo_dados)), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        enviar(arquivo_dados);
        close(s_cliente);

        printf("Texto ... TEL: %s\nARQUIVO: %s\n", telefone_arquivo, arquivo);
    }
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
    if (getsockname(s_dados, (struct sockaddr *) &server_dados, (socklen_t *) &namelen_dados) < 0) {
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

    if (pthread_create(&thread_ServerCliente, NULL, thread_server, NULL)) {
        printf("ERRO: impossivel criar uma thread\n");
        exit(-1);
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
    strtok(telefone, "\n");

    strcpy(nomeArquivo, cliente.telefone);
    strcat(nomeArquivo, ".txt");
    snprintf(file_name, (strlen(nomeArquivo) + 1), "%s", nomeArquivo);// pega o nome do arquivo de um uma variavel
    fp = fopen(file_name, "a");// abre o arquivo

    fprintf(fp, "%s\n", telefone);
	fclose(fp);

    printf("Novo contato salvo com sucesso!\n");
};

void enviar_mensagem_grupo(char grupo[]) {
    struct dados_cliente dados;
    FILE *fp;
    char nameFolder[200], file_name[200];
    size_t len;
    char *line = NULL;
    int read = 0;
    char mensagem[2000], porta[32], telefone[DIGITOSTELEFONE];
    memset(nameFolder, 0, sizeof(nameFolder));
    memset(file_name, 0, sizeof(file_name));
    
    printf("Digite a mesnagem:\n");
    fpurge(stdin);
    fgets(mensagem,sizeof(mensagem),stdin);

    strcpy(nameFolder, nome_pasta);
    strcat(nameFolder, "/");
    strcat(nameFolder, grupo);
    strcat(nameFolder, ".txt");
    snprintf(file_name, (strlen(nameFolder) + 1), "%s", nameFolder);// pega o nome do arquivo de um uma variavel
    printf("file_name: %s\n", file_name);

    fp = fopen(file_name, "r");// abre o arquivos

    while ((read = getline(&line, &len, fp)) != -1) {
        strtok(line, "\n");
        printf("line: %s\n", line);
        
        int comando = 3; // para acessar a funcao 3 do servidor
        if (send(s_server, &comando, (sizeof(comando)), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        strcpy(telefone, line);
        if (send(s_server, &telefone, (sizeof(telefone)), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        if (recv(s_server, &dados, (sizeof(dados)), 0) < 0) {
            perror("Recv()");
            exit(6);
        }

        if (dados.porta != 0) {
            struct mensagem_dados mensagem_dados;
            strcpy(mensagem_dados.type, "mensagem");
            strcpy(mensagem_dados.telefone_remetente, cliente.telefone);
            strcpy(mensagem_dados.conteudo, mensagem);

            sprintf(porta, "%i", dados.porta);
            conectar(dados.ip, porta, &s_cliente);

            if (send(s_cliente, &mensagem_dados, (sizeof(mensagem_dados)), 0) < 0) {
                perror("Send() 1");
                exit(5);
            }

            close(s_cliente);
        }
    };
    fclose(fp);

}

void enviar_arquivo_grupo(char grupo[]) {
    struct dados_cliente dados;
    FILE *fp;
    char nameFolder[200], file_name[200], arquivo[100];
    size_t len;
    char *line = NULL;
    int read = 0;
    char mensagem[2000], porta[32], telefone[DIGITOSTELEFONE];
    memset(nameFolder, 0, sizeof(nameFolder));
    memset(file_name, 0, sizeof(file_name));
    
    printf("Digite o nome do arquivo:\n");
    fpurge(stdin);
    fgets(arquivo,sizeof(arquivo),stdin);
    strtok(arquivo, "\n");

    strcpy(nameFolder, nome_pasta);
    strcat(nameFolder, "/");
    strcat(nameFolder, arquivo);
    printf("nameFolder: %s\n\n", nameFolder);
    snprintf(file_name, strlen(nameFolder)+1, "%s", nameFolder);
    long size_file = file_size(file_name);
    printf("size_file: %li\n", size_file);

    struct mensagem_dados arquivo_dados;
    strcpy(arquivo_dados.type, "arquivo");
    strcpy(arquivo_dados.telefone_remetente, cliente.telefone);
    strcpy(arquivo_dados.conteudo, arquivo);
    arquivo_dados.tamanho = size_file;

    memset(nameFolder, 0, sizeof(nameFolder));
    memset(file_name, 0, sizeof(file_name));

    strcpy(nameFolder, nome_pasta);
    strcat(nameFolder, "/");
    strcat(nameFolder, grupo);
    strcat(nameFolder, ".txt");
    snprintf(file_name, (strlen(nameFolder) + 1), "%s", nameFolder);// pega o nome do arquivo de um uma variavel
    printf("file_name: %s\n", file_name);
    fp = fopen(file_name, "r");// abre o arquivos

    while ((read = getline(&line, &len, fp)) != -1) {
        strtok(line, "\n");
        printf("line: %s\n", line);
        
        int comando = 3; // para acessar a funcao 3 do servidor
        if (send(s_server, &comando, (sizeof(comando)), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        strcpy(telefone, line);
        if (send(s_server, &telefone, (sizeof(telefone)), 0) < 0) {
            perror("Send() 1");
            exit(5);
        }

        if (recv(s_server, &dados, (sizeof(dados)), 0) < 0) {
            perror("Recv()");
            exit(6);
        }

        printf("Telefone: %s\n", dados.telefone);
        printf("Porta: %i\n", dados.porta);
        printf("IP: %s\n", dados.ip);

        if (dados.porta != 0) {
            sprintf(porta, "%i", dados.porta);
            conectar(dados.ip, porta, &s_cliente);

            if (send(s_cliente, &arquivo_dados, (sizeof(arquivo_dados)), 0) < 0) {
                perror("Send() 1");
                exit(5);
            }

            enviar(arquivo_dados);
            close(s_cliente);

            // printf("Texto ... TEL: %s\nMENSAGEM: %s\n", telefone_mensagem, mensagem);
        }
                // enviar_mensagem();
    };


	fclose(fp);
}

void menu_enviar_grupo() {
    struct grupos *agenda_grupos;
    agenda_grupos = malloc (sizeof (struct grupos));
    agenda_grupos->prox = NULL;
    strcpy(agenda_grupos->conteudo, "0000");


    FILE *fp;
    bool variavelLoop = false;
    char telefone[DIGITOSTELEFONE];
    char nameFolder[200];
    char nome_grupo[20], file_name[200];
    int comando, qntdContatos;
    size_t len;
    char *line = NULL;
    int read = 0, countGrupos = 0, numeroGrupo = 0;

    strcpy(nameFolder, nome_pasta);
    strcat(nameFolder, "/grupos.txt");
    snprintf(file_name, (strlen(nameFolder) + 1), "%s", nameFolder);// pega o nome do arquivo de um uma variavel
    long int size = file_size(file_name);
    printf("size: %ld\n", size);

    if (size == 0) {
        printf("Nao possui grupos cadastrados ...\n");
    } else {
        fp = fopen(file_name, "r");// abre o arquivo

        while ((read = getline(&line, &len, fp)) != -1) {
            strtok(line, "\n");
            insere_lista_grupos(line, agenda_grupos);
            countGrupos++;
        };

        fclose(fp);

        printf("Lista de Grupos: \n");
        struct grupos *print;
        print = agenda_grupos->prox;
        
        for(int i = 0; i < countGrupos; i++) {

            printf("%i) %s\n", i+1, print->conteudo);

            print = print->prox;
        }

        printf("Digite o grupo para enviar mensagem: \n");
        scanf("%i", &numeroGrupo);

        print = agenda_grupos->prox;
        for(int i = 0; i < countGrupos; i++) {

            if (numeroGrupo == i+1) {
                memset(nome_grupo, 0, sizeof(nome_grupo));
                strcpy(nome_grupo, print->conteudo);
            }

            print = print->prox;
        }

        free(agenda_grupos);
        free(print);

        printf("Grupo Selecionado: %s\n", nome_grupo);

        // enviar_mensagem_grupo(nome_grupo);

        do {
            printf("CHAT GRUPO - Selecione o tipo de mensagem:\n"
                "1 - Texto\n"
                "2 - Arquivo\n"
                "3 - Sair do Chat\n\n"
                "Digite o numero da opcao: ");

            fpurge(stdin);
            scanf("%i", &comando);

            switch (comando) {
            case 1:
                enviar_mensagem_grupo(nome_grupo);
                break;

            case 2:
                enviar_arquivo_grupo(nome_grupo);
                break;

            case 3:
                variavelLoop = true;
                break;

            default:
                printf("Comando invalido ... Por favor insira um novo comando\n");
                break;
            }

        } while(!variavelLoop);
    }
}

void criar_grupo() {
    FILE *fp;
    char telefone[DIGITOSTELEFONE];
    char nameFolder[200];
    char nome_grupo[20], file_name[200];
    int comando, qntdContatos;
    bool variavelLoop = false;

    printf("Digite o nome do grupo: ");
    fpurge(stdin);
    fgets(nome_grupo,sizeof(nome_grupo),stdin);
    strtok(nome_grupo, "\n");

    strcpy(nameFolder, nome_pasta);
    strcat(nameFolder, "/grupos.txt");
    snprintf(file_name, (strlen(nameFolder) + 1), "%s", nameFolder);// pega o nome do arquivo de um uma variavel

    fp = fopen(file_name, "a");// abre o arquivo
    fprintf(fp, "%s\n", nome_grupo);
	fclose(fp);

    printf("Digite quantos contatos tera o grupo: ");
    scanf("%i", &qntdContatos);
    
    memset(nameFolder, 0, sizeof(nameFolder));
    memset(file_name, 0, sizeof(file_name));
    
    strcpy(nameFolder, nome_pasta);
    strcat(nameFolder, "/");
    strcat(nameFolder, nome_grupo);
    strcat(nameFolder, ".txt");
    snprintf(file_name, (strlen(nameFolder) + 1), "%s", nameFolder);// pega o nome do arquivo de um uma variavel
    fp = fopen(file_name, "a");// abre o arquivo

    for (int i = 0; i < qntdContatos; i++) {
        printf("Digite o %i telefone do grupo: ", i);
        fpurge(stdin);
        fgets(telefone,sizeof(telefone),stdin);
        strtok(telefone, "\n");
        fprintf(fp, "%s\n", telefone);
        printf("\n");
    }

	fclose(fp);
}

void menu_chat_tipo_mensagem() {
    bool variavelLoop = false;
    int comando;

    do {
        printf("CHAT - Selecione o tipo de mensagem:\n"
            "1 - Texto\n"
            "2 - Arquivo\n"
            "3 - Sair do Chat\n\n"
            "Digite o numero da opcao: ");

        fpurge(stdin);
        scanf("%i", &comando);

        switch (comando) {
        case 1:
            comando = 3; // para acessar a funcao 3 do servidor
            if (send(s_server, &comando, (sizeof(comando)), 0) < 0) {
                perror("Send() 1");
                exit(5);
            }
            enviar_mensagem();
            break;

        case 2:
            comando = 3; // para acessar a funcao 3 do servidor
            if (send(s_server, &comando, (sizeof(comando)), 0) < 0) {
                perror("Send() 1");
                exit(5);
            }
            enviar_arquivo();
            break;

        case 3:
            variavelLoop = true;
            break;

        default:
            printf("Comando invalido ... Por favor insira um novo comando\n");
            break;
        }

    } while(!variavelLoop);

}

void menu_chat() {
    bool variavelLoop = false;
    int comando;

    do {
        printf("CHAT - Enviar mensagem para:\n"
            "1 - Individual\n"
            "2 - Grupo\n"
            "3 - Sair do Chat\n\n"
            "Digite o numero da opcao: ");

        fpurge(stdin);
        scanf("%i", &comando);

        switch (comando) {
        case 1:
            menu_chat_tipo_mensagem();
            break;

        case 2:
            printf("GRUPO ...\n");
            menu_enviar_grupo();
            break;

        case 3:
            variavelLoop = true;
            break;

        default:
            printf("Comando invalido ... Por favor insira um novo comando\n");
            break;
        }

    } while(!variavelLoop);

}

void menu_adicionar_contato() {
    bool variavelLoop = false;
    int comando;

    do {
        printf("MENU CONTATOS - Criar novo contato:\n"
            "1 - Individual\n"
            "2 - Grupo\n"
            "3 - Sair do Menu Contatos\n\n"
            "Digite o numero da opcao: ");

        fpurge(stdin);
        scanf("%i", &comando);

        switch (comando) {
        case 1:
            adicionar_contato();
            break;

        case 2:
            printf("GRUPO ...\n");
            criar_grupo();
            break;

        case 3:
            variavelLoop = true;
            break;

        default:
            printf("Comando invalido ... Por favor insira um novo comando\n");
            break;
        }

    } while(!variavelLoop);

}

void criar_contato() {
    char telefone[DIGITOSTELEFONE];
    bool variavelLoop = true;
    struct controle_status {
        char telefone[DIGITOSTELEFONE];
        char status[10];
    };

    struct controle_status status;

    create_socket();

    do {
        printf("Insira o seu telefone - 8 digitos: ");

        fpurge(stdin);
        fgets(telefone,sizeof(telefone),stdin);
        strtok(telefone, "\n");

        cliente.porta = port_dados;
        strcpy(cliente.telefone, telefone);
        strcpy(cliente.ip, ip);

        // printf("PORTA: %d\n", cliente.porta);
        // printf("TELEFONE: %s\n", cliente.telefone);
        // printf("IP: %s\n", cliente.ip);

        // enviar para o servidor a porta do socket de dados e do telefone
        if (send(s_server, &cliente, (sizeof(cliente)), 0) < 0) {
            perror("Send() 3");
            exit(5);
        }

        if (recv(s_server, &status, (sizeof(status)), 0) < 0) {
            perror("Send() 3");
            exit(5);
        }

        printf("status.status: %s\n", status.status);
        printf("status.telefone: %s\n", status.telefone);

        if ((strcmp(status.status, "ok") == 0)) {
            variavelLoop = false;
            printf("\033[0;34mCliente cadastrado com sucesso!\n");
            printf("\033[0;37m");
        } else { 
            printf("\033[0;31mJa existe um usuairo cadastrado no sistema com esse telefone!\n");
            printf("\033[0;37m");
        }

    } while(variavelLoop);

    // printf("Insira o seu telefone: ");
};

// MAIN FUNCTION
int main(int argc, char **argv){
    conectar(argv[1], argv[2], &s_server);
    criar_contato();
    criar_pasta_dados();
    bool variavelLoop = false;
    int comando;

    do {
        //system("clear");
        // system("/bin/zsh");
            printf("MENU:\n\a"
            "1 - Adicionar novo contato\n"
            "2 - Ver contatos\n"
            "3 - Enviar mensagem\n"
            "4 - Sair da aplicacao\n\n"
            "Digite o numero da opcao: ");

            // printf("\0337\n\033[");
            // printf("\033[L**************** y *****************\n");
            // printf("\033[LRemetente: %s\n","BATATA");
            // printf("\033[LMensagem: %s\n","BATATA");
            // printf("\033[L***************** x  ****************\n");
            // // printf("\033[LDigite o numero da opcao:\n");
            // // printf("\033[1F");
            // printf("\0338\033[5A");
            // fflush(stdout);

        fpurge(stdin);
        scanf("%i", &comando);

        switch (comando) {
        case 1:
            menu_adicionar_contato();
            break;

        case 2:
            listar_contatos();
            break;
        
        case 3:
            menu_chat();
            break;

        case 4:
            if (send(s_server, &comando, (sizeof(comando)), 0) < 0) {
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
    close(s_server);
    printf("Cliente terminou com sucesso.\n");
    exit(0);
}
