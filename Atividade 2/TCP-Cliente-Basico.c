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


char sendbuf[12];              
char recvbuf[12]; 
int s; 

typedef struct {
char Name[MaxNAME];
char Msg[MaxMsg];
int Opcao; //Informar ao servidor qual procedimento foi realizado
} Obj; 

// Procedimento para opcao 1
void adicionar_usuario_mensagens(){
    Obj obj;
    obj.Opcao = 1;
    char name[MaxNAME];
    char msg[MaxMsg];


    printf("Usuario: \n");
	scanf("%s",&name);

    printf("Mensagem: \n");
	scanf("%s",&msg);

    strcpy(obj.Name,name);
	strcpy(obj.Msg,msg);


	acessar_servidor(obj);
	// adicionar usuario e mensagens a ser cadastradas
}

// Procedimento para opcao 2
void encontrar_usuario_mensagens(){
    Obj obj;
    obj.Opcao = 2;
        printf("Mensagens cadastradas: \n");   
	// encontrar todos os usuarios e suas mensagens cadastradas
}

// Procedimento para opcao 3
void apagar_usuario_mensagens(){
    Obj obj;
    obj.Opcao = 3;
    char name[MaxNAME];

    printf("Usuario: \n");
    scanf("%s",&name);
    strcpy(obj.Name,name);


    acessar_servidor(obj);


	// encontrar usuario e apagar a mensagem ! *obs : retornar a mensagem removida

}

    // procedimento para enviar e receber mensagem do servidor
void acessar_servidor(Obj obj ){

    strcpy(sendbuf, obj.Name);


    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf)+1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    printf("Mensagem enviada ao servidor: %s\n", sendbuf);

    /* Recebe a mensagem do servidor no buffer de recepcao */
    if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
    {
        perror("Recv()");
        exit(6);
    }
    printf("Mensagem recebida do servidor: %s\n", recvbuf);


}




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


