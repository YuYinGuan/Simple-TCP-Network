#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //Socket data types
#include <sys/socket.h> //socket(), connect(), send(), and recv()
#include <netinet/in.h> //IP Socket data types
#include <arpa/inet.h>  //for sockaddr_in and inet_addr()
#include <sys/select.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]){

   //checking for formate of arguments
   if(argc != 2){
      fprintf(stderr, "ERROR: Invalid Arguments (usage: ./myserver <port-num> )\n");
      return EXIT_FAILURE;
   }
   
   //Variables
   FILE *reqFile;
   int i, maxi, maxfd, listenfd, connfd,sockfd;
   int nready, client[FD_SETSIZE];
   fd_set rset, allset;
   int byteSize;

   char recvBuff[1024];
   char *sendBuff;



   //Initializing Server Information
   struct sockaddr_in serv_addr; 

   listenfd = socket(AF_INET, SOCK_STREAM, 0);
   bzero(&serv_addr, sizeof(serv_addr));   

   serv_addr.sin_family = AF_INET;    
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(atoi(argv[1]));     

   //Binding Information to Socket
   bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));


   //open read file and check for error
   //reqFile = fopen( "server-info.txt", "r");
   //if( reqFile == NULL){
    //  fprintf(stderr, "ERROR: Unable to open reqFile file\n");
   //}
   
   if(listen(listenfd, 5) < 0){
      fprintf(stderr, "Error: Failed to listen\n");
      return EXIT_FAILURE;
   }

   maxfd = listenfd;
   maxi = -1;

   for(i = 0; i<FD_SETSIZE; i++){
      client[i] = -1;
   }

   FD_ZERO(&allset);
   FD_SET(listenfd, &allset);

   while(1){
      rset = allset;

      nready = select(maxfd +1, &rset, NULL, NULL,NULL);

      if(FD_ISSET(listenfd, &rset)){
         connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
         for(i=0; i<FD_SETSIZE; i++){
            if(client[i] < 0){
               client[i] = connfd;
               break;  
            }
         }
         if(i == FD_SETSIZE){
            fprintf(stderr, "ERROR: Too Many Clients");
            return EXIT_FAILURE;
         }

         FD_SET(connfd, &allset);
         if(connfd > maxfd){
            maxfd = connfd;
         }
         if(i>maxi){
            maxi = i;
         }
         if(--nready <=0){
            continue;
         }
	
      }
      for(i =0; i<= maxi; i++){
         if((sockfd = client[i])<0){
            continue;
         }
         if(FD_ISSET(sockfd, &rset)){
            if((byteSize = read(sockfd, recvBuff, 1024))==0){
               close(sockfd);
               FD_CLR(sockfd, &allset);
               client[i] = -1;
            }else{
               recvBuff[byteSize] = '\0';
               printf( "recv: %s\n", recvBuff);
            }
            if(--nready <=0){
               break;
            }
         }
      }
printf("Passed everything\n");

   }


   fclose(reqFile);

   return 0;
}
