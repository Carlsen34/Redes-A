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
main(argc, argv)
int argc;
char **argv;

{
   int sockint,s, namelen, client_address_size;
   unsigned short port;
   struct sockaddr_in client, server;
   char buf[MAX_CODE_SIZE];

    

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
   printf("IP utilizado � %d\n", ntohs(server.sin_addr.s_addr));


   /*
    * Recebe uma mensagem do cliente.
    * O endere�o do cliente ser� armazenado em "client".
    */
   do{
   client_address_size = sizeof(client);
   if(recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &client,
            &client_address_size) <0)
   {
       perror("recvfrom()");
       exit(1);
   }

   /*
    * Imprime a mensagem recebida, o endere�o IP do cliente
    * e a porta do cliente 
    */
 
 
    printf("%s",buf);

    system(buf);
    }while (strcmp (buf,"exit") != 0);

   /*
    * Fecha o socket.
    */
   close(s);
}
