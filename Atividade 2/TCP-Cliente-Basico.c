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

/*
 * Cliente TCP
 */

#define MaxNAME 20
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

// procedimento para enviar e receber mensagem do servidor
void acessar_servidor(Obj obj){

    // strcpy(sendbuf, obj.Name);


    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, &obj, (sizeof(obj)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    printf("Mensagem enviada ao servidor\n");

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


    printf("Usuario: \n");
    memset(name, 0, sizeof(name));
	scanf("%s", name);

    printf("Mensagem: \n");
    memset(msg, 0, sizeof(msg));
    scanf("%s", msg);

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
        printf("Usuario: %s  ", objStore.Name);
        printf("Mensagem: %s\n", objStore.Msg);
    }
}

// Procedimento para opcao 2 -  Ler mensagens 
void encontrar_usuario_mensagens(){
    Obj obj;
    obj.Opcao = 2;
    strcpy(obj.Name,"");
    strcpy(obj.Msg,"");

    int sizeOfObjStore = 0;

    if (send(s, &obj, (sizeof(obj)), 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    printf("Mensagem enviada ao servidor - opcao 2\n");

    if (recv(s, &sizeOfObjStore, sizeof(sizeOfObjStore), 0) < 0)
    {
        perror("Recv()");
        exit(6);
    }

    printf("sizeOfObjStore: %d\n", sizeOfObjStore);
    printMessages(sizeOfObjStore);

	// encontrar todos os usuarios e suas mensagens cadastradas
};

// Procedimento para opcao 3 - Apagar mensagens 
void apagar_usuario_mensagens(){
    Obj obj;
    obj.Opcao = 3;
    char name[MaxNAME];

    printf("Usuario: \n");
    scanf("%19s", name);
    strcpy(obj.Name,name);


    acessar_servidor(obj);
	// encontrar usuario e apagar a mensagem ! *obs : retornar a mensagem removida

};


// MAIN FUNCTION
int main(int argc, char **argv)
{
    unsigned short port;       
             
    struct hostent *hostnm;    
    struct sockaddr_in server; 
    int choice;
    bool variavelLoop = true;


    /*
     * O primeiro argumento (argv[1]) e o hostname do servidor.
     * O segundo argumento (argv[2]) e a porta do servidor.
     */
    if (argc != 3)
    {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(argv[2]);

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


    /*
     * Criar Switch Case + Input de dados
     */
    do{
    variavelLoop = false;
    printf(" 1 - Cadastrar mensagem \n 2 - Ler mensagens \n 3 - Apagar mensagens \n 4 - Sair da Aplicacao \n");
    scanf("%d", &choice);



    switch (choice){
    case 1:
	adicionar_usuario_mensagens();
        break;
    case 2:
	encontrar_usuario_mensagens();
        break;
    case 3:
	apagar_usuario_mensagens();
        break;
    case 4:
        printf("Fim de aplicação \n");
	variavelLoop = true;

        break;
    default:
	printf("Opcao invalida \n");

    }
}while(!variavelLoop);

    /* Fecha o socket */
    close(s);

    printf("Cliente terminou com sucesso.\n");
    exit(0);

}


