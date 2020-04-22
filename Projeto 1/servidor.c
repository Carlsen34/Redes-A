/*
Herick Valsecchi Carlsen 15159619
João Pedro Favara 16061921
Raissa Furlan Davinha 15032006
Leonardo Blanco Natis 15296858
Kaíque Ferreira Fávero 15118698
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CODE_SIZE 200

/*
 * Servidor UDP
 */
int main(argc, argv)
int argc;
char **argv;

{
   int sockint,s, namelen, client_address_size;
   unsigned short port;
   struct sockaddr_in client, server;
   char buf[MAX_CODE_SIZE], answer[2000];
   FILE *fp;

    

   /*
    * Cria um socket UDP (dgram). 
    */
   if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
       perror("socket()");
       exit(1);
   }

   /*
    * Define a qual endere�o IP e porta o servidor estar� ligado.
    * Porta = 0 -> faz com que seja utilizada uma porta qualquer livre.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endere�os IP
    */

   port = ntohs(atoi(argv[1])); 
   server.sin_family      = AF_INET;   /* Tipo do endere�o             */
   server.sin_port        = port;         /* Escolhe uma porta dispon�vel */
   server.sin_addr.s_addr = INADDR_ANY;/* Endere�o IP do servidor      */

   /*
    * Liga o servidor � porta definida anteriormente.
    */
   if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
   {
       perror("bind()");
       exit(1);
   }

   /* Consulta qual porta foi utilizada. */
   namelen = sizeof(server);
   if (getsockname(s, (struct sockaddr *) &server, &namelen) < 0)
   {
       perror("getsockname()");
       exit(1);
   }

   /* Imprime qual porta foi utilizada. */
   printf("Porta utilizada � %d\n", ntohs(server.sin_port));

   /*
    * Recebe uma mensagem do cliente.
    * O endere�o do cliente ser� armazenado em "client".
    */
   do{
    memset(answer, 0, sizeof(answer));
   client_address_size = sizeof(client);
   if(recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &client, &client_address_size) <0)
   {
       perror("recvfrom()");
       exit(1);
   }
   printf("command receive from client => %s\n", buf);

   /*
    * Imprime a mensagem recebida, o endere�o IP do cliente
    * e a porta do cliente 
    */
    fp = popen(buf, "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        exit(1);
    }

    fread(answer, (sizeof(answer) + 1), 1, fp);

    // Adiciona o \0 para enviar exatos 2000 caractéres
    answer[1999] = '\0';

    /* close */
    pclose(fp);


   if (sendto(s, answer, (strlen(answer)), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
   {
       perror("sendto()");
       exit(2);
   }

    }while (strcmp (buf,"exit") != 0);

   /*
    * Fecha o socket.
    */
   close(s);
}
