#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>

/*
 * Servidor TCP
 */


#define MaxNAME 20
#define MaxMsg 80
#define MaxArray 10


typedef struct {
char Name[MaxNAME];
char Msg[MaxMsg];
int Opcao; //Informar ao servidor qual procedimento foi realizado
} Obj; 

unsigned short port;
char sendbuf[12];
char recvbuf[12];
Obj objStore[MaxArray];
Obj objStoreAux[MaxArray];
Obj receiveMsg;
struct sockaddr_in client; 
struct sockaddr_in server; 
int s;                     /* Socket para aceitar conexoes       */
int ns;                    /* Socket conectado ao cliente        */
int namelen;
int arrayMsgCount = 0;

void retorno_cliente(char retornoMsg[200]) {
    /* Envia uma mensagem ao cliente atraves do socket conectado */
    if (send(ns, retornoMsg, strlen(retornoMsg)+1, 0) < 0)
    {
        perror("Send()");
        exit(7);
    }
}

void opcao_1(Obj rcv){
   
    if (arrayMsgCount < 10) {
        printf("\nNome recebido do cliente:%s\n", receiveMsg.Name);
        printf("\nMensagem recebida do cliente:%s\n", receiveMsg.Msg);

        objStore[arrayMsgCount] = rcv;
        arrayMsgCount++;

        retorno_cliente("Mensagem salva com sucesso!\n");
    } else {
        retorno_cliente("Nao foi possivel inserir uma nova mensagem\n");
    }

}

void opcao_2(){
    if (send(ns, &arrayMsgCount, sizeof(arrayMsgCount), 0) < 0)
    {
        perror("Send()");
        exit(7);
    }

    for(int i = 0; i < arrayMsgCount; i++) {
        if (send(ns, &objStore[i], sizeof(objStore[i]), 0) < 0)
        {
            perror("Send()");
            exit(7);
        }
    }
}


void opcao_3(){
    for(int i = 0; i < arrayMsgCount; i++) {
        printf("\nMensagem Removida:%s\n",objStore[i].Msg);
    if (send(ns, objStore[i].Msg, strlen(objStore[i].Msg)+1, 0) < 0)
    {
        perror("Send()");
        exit(7);
    }

    }

}


void recebe_envia_mensagem(){
     bool variavelLoop = false;
    do {
        /* Recebe uma mensagem do cliente atraves do novo socket conectado */
        if (recv(ns, &receiveMsg, sizeof(receiveMsg), 0) == -1)
        {
            perror("Recv()");
            exit(6);
        }

        switch (receiveMsg.Opcao){
        case 1:
        opcao_1(receiveMsg);
            break;
        case 2:
        opcao_2();
            break;
        case 3:
        opcao_3();
            break;
        case 4: 
            variavelLoop = true;
        default:
        retorno_cliente("Opcao invalida \n");
        }
    } while(!variavelLoop);
}


int main(int argc, char **argv)
{

    /*
     * O primeiro argumento (argv[1]) e a porta
     * onde o servidor aguardara por conexoes
     */
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
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
    printf("\nPorta utilizada � %d\n", ntohs(server.sin_port));
    printf("\nIP utilizado � %d\n", ntohs(server.sin_addr.s_addr));

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

    /*
     * Aceita uma conexao e cria um novo socket atraves do qual
     * ocorrera a comunicacao com o cliente.
     */
    namelen = sizeof(client);
    if ((ns = accept(s, (struct sockaddr *)&client, (socklen_t *)&namelen)) == -1)
    {
        perror("Accept()");
        exit(5);
    }

    recebe_envia_mensagem();

    /* Fecha o socket conectado ao cliente */
    close(ns);

    /* Fecha o socket aguardando por conex�es */
    close(s);

    printf("\nServidor terminou com sucesso.\n");
    exit(0);
}


