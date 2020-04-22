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

/*
 * Cliente TCP
 */
 
#define MaxNAME 20
#define COMMAND 200
#define MaxMsg 80
#define MaxArray 10

typedef struct {
char Name[MaxNAME];
char Msg[MaxMsg];
int Opcao; //Informar ao servidor qual procedimento foi realizado
} Obj; 

char sendbuf[12];
char recvbuf[200];
int s;
Obj objStore;
char command[COMMAND];

// procedimento para enviar e receber mensagem do servidor
void acessar_servidor(Obj obj){
    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, &obj, (sizeof(obj)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    printf("\nMensagem enviada ao servidor");

    /* Recebe a mensagem do servidor no buffer de recepcao */
    if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
    {
        perror("Recv()");
        exit(6);
    }
    printf("%s\n", recvbuf);
};

// Procedimento para opcao 1 -  Cadastrar mensagem
void adicionar_usuario_mensagens(){
    Obj obj;
    obj.Opcao = 1;
    char name[MaxNAME];
    char msg[MaxMsg];

    printf("\nUsuario: \n");
    fpurge(stdin);
    //getchar();
    memset(name, 0, sizeof(name));
    fgets(name,sizeof(name),stdin);
    strtok(name,"\n");

    printf("\nMensagem: \n");
    fpurge(stdin);
    //getchar();
    memset(msg, 0, sizeof(msg));
    fgets(msg,sizeof(msg),stdin);
    strtok(name,"\n");


    strcpy(obj.Name,name);
	strcpy(obj.Msg,msg);

	acessar_servidor(obj);
	// adicionar usuario e mensagens a ser cadastradas
};

void printMessages(int sizeOfObjStore) {

    for(int i = 0; i < sizeOfObjStore; i++) {
        memset(&objStore, 0, sizeof(objStore));
        if (recv(s, &objStore, sizeof(objStore), 0) < 0)
        {
            perror("Recv()");
            exit(6);
        }
        printf("Usuario: %s ", objStore.Name);
        printf("\tMensagem: %s", objStore.Msg);
    }
}

// Procedimento para opcao 2 -  Ler mensagens 
void encontrar_usuario_mensagens(){
    Obj obj;
    obj.Opcao = 2;
    strcpy(obj.Name,"");
    strcpy(obj.Msg,"");
    int sizeOfObjStore = 0;

    // envia um Obj somente com a opcao 2
    if (send(s, &obj, (sizeof(obj)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    // Recebe o valor de quantas posicoes existem no array de objStore
    if (recv(s, &sizeOfObjStore, sizeof(sizeOfObjStore), 0) < 0)
    {
        perror("Recv()");
        exit(6);
    }

    printf("\nMensagens cadastrada(s): %i\n", sizeOfObjStore);
    // if (sizeOfObjStore >= 1) {
        printMessages(sizeOfObjStore); // encontrar todos os usuarios e suas mensagens cadastradas
    // }
};

// Procedimento para opcao 3 - Apagar mensagens 
void apagar_usuario_mensagens(){
    Obj obj;
    obj.Opcao = 3;
    char name[MaxNAME];
    int sizeOfObjStore2 = 0;

    printf("\nUsuario: \n");
    scanf("%19s", name);
    strcpy(obj.Name,name);

    // envia um Obj somente com a opcao 2
    if (send(s, &obj, (sizeof(obj)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    // Recebe o valor de quantas posicoes existem no array de objStore
    if (recv(s, &sizeOfObjStore2, sizeof(sizeOfObjStore2), 0) < 0)
    {
        perror("Recv()");
        exit(6);
    }

    printf("\nMensagens apagada(s): %i\n", sizeOfObjStore2);

    printMessages(sizeOfObjStore2);  // encontrar usuario e apagar a mensagem ! *obs : retornar a mensagem removida
};

void encerrar() {
    Obj obj;
    obj.Opcao = 4;
    strcpy(obj.Name,"");
    strcpy(obj.Msg,"");
    int sizeOfObjStore = 0;

    if (send(s, &obj, (sizeof(obj)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
}

void listar(const char list_command[]) {
    char comando_listar[200], nomeFile[256];
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

    printf("hostname: %s", hostname);
    printf("porta: %s", porta);

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
int main(int argc, char **argv)
{
    bool variavelLoop = true;

    do {
        variavelLoop = false;
        printf("Insira um comando:\n");
        memset(command, 0, sizeof(command));
        fpurge(stdin);
        fgets(command,sizeof(command),stdin);
        strtok(command,"\n");

        char *token = strtok(command, " ");

        int i = 0;
        char value[3][200];

        while( token != NULL ) {
            strcpy(value[i],token);
            // printf("Token[%i] = %s\n", i, token);
            token = strtok(NULL, " ");
            i++;
        }

        if ((strcmp(value[0], "listar")) == 0) {
            printf("Listar foi chamado");
            listar(value[0]);
        }

        if ((strcmp(value[0], "conectar")) == 0) {
            conectar(value[1], value[2]);
        }



    } while(!variavelLoop);

    /* Fecha o socket */
    close(s);

    printf("Cliente terminou com sucesso.\n");
    exit(0);

}


