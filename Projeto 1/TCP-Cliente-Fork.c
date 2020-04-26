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

int s;

char command[COMMAND];

int conectar_file(char hostname[], char porta[]) {
    int s_file;
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;
    char teste[200];

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

    /*
     * Cria um socket TCP (stream)
     */
    if ((s_file = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if (connect(s_file, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }

    return s_file;
}

void enviar(char comando[]) {
    FILE *fp;
    char comando_enviar[COMMAND];
    char bufsize[100];

    strcpy(comando_enviar, comando);
    if (send(s, &comando_enviar, (strlen(comando_enviar)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    memset(comando_enviar, 0, sizeof(comando_enviar));
    if (recv(s, &comando_enviar, sizeof(comando_enviar), 0) == -1)
    {
        perror("Recv()");
        exit(6);
    }

    printf("Recebeu retorno do servidor, %s\n", comando_enviar);
    if(strcmp(comando_enviar,"ok") == 0) {
        int s_file;
        s_file = conectar_file("localhost", "5001");

        fp = fopen("teste.txt", "r+");

        fseek(fp, 0L, SEEK_END);
        long int size_file = ftell(fp);
        printf("size_file = %li\n", size_file);

        fseek(fp, 0, SEEK_SET);
        fread(bufsize, 100, 1, fp); 
        printf("bufsize (%i) : %s\n",strlen(bufsize), bufsize);
        if (send(s_file, &size_file, (sizeof(size_file)), 0) < 0) {
            perror("Send()");
            exit(5);
        }

        if (send(s_file, &bufsize, (sizeof(bufsize)), 0) < 0) {
            perror("Send()");
            exit(5);
        }

        fclose(fp);
        close(s_file);
        printf("Fechou tudo!\n");

    }

}

void encerrar(const char list_command[]) {
    char comando_encerrar[COMMAND], nomeFile[256];
    strcpy(comando_encerrar, list_command);
    if (send(s, &comando_encerrar, (sizeof(comando_encerrar)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    close(s);
    printf("Finalizando o cliente ...\n");
    exit(0);
}

void listar(const char list_command[]) {
    char comando_listar[COMMAND], nomeFile[256];
    strcpy(comando_listar, list_command);
    DIR *dir;
    struct dirent *dp;
    int count = 0;

    if (send(s, &comando_listar, (strlen(comando_listar)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    // Recebe a lista de arquivos que estao no servidor...
    do{
        if (recv(s, &nomeFile, (sizeof(nomeFile)), 0) < 0)
        {
            perror("Recv()");
            exit(6);
        }

        printf("> %s\n", nomeFile);
    }while((strcmp(nomeFile, "stop")) != 0);
}




void conectar(char hostname[], char porta[]) {
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

int baixar(char list_command[]){
    char localizacao[200], pacote[BUFSIZ],
     nomeDoArquivo[200], mensagem[200];
    int qntd = 0;
	ssize_t len;

//verifica se existe nome do arquivo para enviar
    if (list_command[1] == NULL)
    {
        printf("Digite o nume do arquivo para receber\n");
        exit(6);
    }
    
    //define nome do arquivo a ser criado
    if(list_command[2] != NULL) {
        strcpy(nomeDoArquivo, list_command[2]);
    } else {
        strcpy(nomeDoArquivo, list_command[1]);
    }
    
    strncpy(mensagem, list_command[0], 1024);
    strcat(mensagem," ");
    strcat(mensagem, list_command[1]);

    //enviar comando e nome do arquivo
    if (send(s, &mensagem, (strlen(mensagem)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    //receber arquivo 
    //primeiro recebo validacao e tamanho
    if (recv(s, &pacote, BUFSIZ, 0) < 0)
    {
		fprintf(stderr, "Arquivo nao encontrado no servirdor\n");
        perror("Recv()");
        exit(6);
    }

    strtok(pacote,"\n");
    char *token = strtok(pacote, " ");
    if (strcmp(token, "404") == 0)
    {
        fprintf(stderr, "Arquivo nao encontrado no servirdor\n");
        return(1);
    }
    
    token = strtok(NULL, " ");
    qntd = atoi(token);
	FILE *received_file;
    received_file = fopen(nomeDoArquivo, "w");

    //segundo recebo os pacotes que formam o arquivo
    while (len = recv(s, &pacote, BUFSIZ, 0) > 0 && qntd > 0)
    {
        fwrite(&pacote,sizeof(char),len, received_file);
        qntd -= len;
        fprintf(stdout, "Recebidos %d bytes e aguardamos :- %d bytes\n", len, qntd);
        if (qntd <= 0) {
            break;
        }
    }

	fclose(received_file);
    return(0);
}

// MAIN FUNCTION
int main(int argc, char **argv)
{
    bool variavelLoop = true;

    do {
        variavelLoop = false;
        printf("Insira um comando:\n");
        printf("opcoes:\n");
        printf("conectar <nome do servidor> [<porta do servidor>]\n");
        printf("listar");
        printf("receber <arquivo remoto> [<arquivo local>]\n");
        printf("enviar <arquivo local> [<arquivo remoto>]\n");
        printf("encerrar\n");

        memset(command, 0, sizeof(command));
        __fpurge(stdin);
        fgets(command,sizeof(command),stdin);
        strtok(command,"\n");

        char *token = strtok(command, " ");

        int i = 0;
        char value[3][COMMAND];

        while( token != NULL ) {
            strcpy(value[i],token);
            token = strtok(NULL, " ");
            i++;
        }


        if ((strcmp(value[0], "conectar")) == 0) {
            conectar(value[1], value[2]);
        }


        if ((strcmp(value[0], "listar")) == 0) {
            printf("Listar foi chamado\n");
            listar(value[0]);
        }

	    if ((strcmp(value[0], "receber")) == 0) {
            baixar(value);
        }

        if ((strcmp(value[0], "encerrar")) == 0) {
            encerrar(value[0]);
        }

        if ((strcmp(value[0], "enviar")) == 0) {
            enviar(command);
        }

    } while(!variavelLoop);

    /* Fecha o socket */
    close(s);

    printf("Cliente terminou com sucesso.\n");
    exit(0);

}


