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
#define SEGMENT 5000 //approximate target size of small file
#define FILESIZE 4096


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

long file_size(char *name) {
    FILE *fp = fopen(name, "rb"); //must be binary read to get bytes

    long size;
    if(fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fclose(fp);
    }
    return size;
}

void enviar(char comando[], char nome_local[]) {
    FILE *fp;
    char comando_enviar[COMMAND];
    char bufsize[100];
    char file_name[strlen(nome_local)+1];
    int segments = 0, i , len, accum;
    long sizeFile = file_size(nome_local);
    segments = sizeFile/SEGMENT + 1;//ensure end of file
    char line[FILESIZE];

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
        s_file = conectar_file("localhost", "21");

        snprintf(file_name, strlen(nome_local)+1, "%s", nome_local);
        fp = fopen(file_name, "rb");

        if (send(s_file, &sizeFile, (sizeof(sizeFile)), 0) < 0) {
            perror("Send()");
            exit(5);
        }

        printf("segments: %li\n", sizeFile);
        if(fp) {
            int sent_bytes = 0;
            while((sent_bytes = fread(line, 1, FILESIZE, fp)) && (sizeFile > 0))
            {
                printf("sizeFile: %li - %i\n", sizeFile, sent_bytes);
                sizeFile -= sent_bytes;

                if (send(s_file, &line, sent_bytes, 0) < 0) {
                    perror("Send()");
                    exit(5);
                }
                memset(line, 0, sizeof(line));
            }
            printf("Finalizou o processo de envio ... \n");
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
    while((strcmp(nomeFile, "stop")) != 0) {
        if (recv(s, &nomeFile, (sizeof(nomeFile)), 0) < 0)
        {
            perror("Recv()");
            exit(6);
        }

        printf("> %s\n", nomeFile);
    };
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

// MAIN FUNCTION
int main(int argc, char **argv){
    bool variavelLoop = true;

    do {
        variavelLoop = false;
        printf("Insira um comando:\n");

        memset(command, 0, sizeof(command));
        fpurge(stdin);
        fgets(command,sizeof(command),stdin);

        char comando[strlen(command)]; 
        strcpy(comando, command);

        char *token = strtok(command, " ");

        int i = 0;
        char value[3][COMMAND];

        while( token != NULL ) {
            strcpy(value[i],token);
            printf("Token[%i] = %s\n", i, token);
            token = strtok(NULL, " ");
            i++;
        }

        printf("command: %s\n", value[0]);
        strtok(value[0],"\n");

        if ((strcmp(value[0], "conectar")) == 0) {
            conectar(value[1], value[2]);
        }

        if ((strcmp(value[0], "listar")) == 0) {
            printf("Listar foi chamado\n");
            listar(value[0]);
        }

        if ((strcmp(value[0], "encerrar")) == 0) {
            encerrar(value[0]);
        }

        if ((strcmp(value[0], "enviar")) == 0) {
            enviar(comando, value[1]);
        }

    } while(!variavelLoop);

    /* Fecha o socket */
    close(s);

    printf("Cliente terminou com sucesso.\n");
    exit(0);

}


