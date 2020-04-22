/*
Herick Valsecchi Carlsen 15159619
João Pedro Favara 16061921
Raissa Furlan Davinha 15032006
Leonardo Blanco Natis 15296858
Kaíque Ferreira Fávero 15118698
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 * Cliente UDP
 */

#define MAX_CODE_SIZE 200


int main(argc, argv)
int argc;
char **argv;

{

   int s, server_address_size;
   unsigned short port;
   struct sockaddr_in server, client;
   char buf[MAX_CODE_SIZE], res[2000];

    char *comando_remoto = malloc(MAX_CODE_SIZE);
    if (comando_remoto == NULL) {
        printf("No memory\n");
        return 1;
    }

   /* 
    * O primeiro argumento (argv[1]) � o endere�o IP do servidor.
    * O segundo argumento (argv[2]) � a porta do servidor.
    */
   if(argc != 3)
   {
      printf("Use: %s enderecoIP porta\n",argv[0]);
      exit(1);
   }
  port = htons(atoi(argv[2]));

   /*
    * Cria um socket UDP (dgram).
    */
   if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
       perror("socket()");
       exit(1);
   }

   /* Define o endere�o IP e a porta do servidor */
   server.sin_family      = AF_INET;            /* Tipo do endere�o         */
   server.sin_port        = port;               /* Porta do servidor        */
   server.sin_addr.s_addr = inet_addr(argv[1]); /* Endere�o IP do servidor  */

   do{
    memset(res, 0, sizeof(res));
    printf("> ");
   fgets(comando_remoto,MAX_CODE_SIZE,stdin);
   comando_remoto[strlen(comando_remoto)-1] = '\0';
   strcpy(buf, comando_remoto);

   /* Envia a mensagem no buffer para o servidor */
   if (sendto(s, buf, (strlen(buf)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0)
   {
       perror("sendto()");
       exit(2);
   }

    if(recvfrom(s, res, sizeof(res), 0, (struct sockaddr *) &server, &server_address_size) <0)
   {
       perror("recvfrom()");
       exit(1);
   }
   printf("%s\n", res);
 
   } while (strcmp (comando_remoto,"exit") != 0);

   /* Fecha o socket */
   close(s);
   free(comando_remoto);
}
