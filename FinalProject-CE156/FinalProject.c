#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //Socket data types
#include <sys/socket.h> //socket(), connect(), send(), and recv()
#include <netinet/in.h> //IP Socket data types
#include <arpa/inet.h>  //for sockaddr_in and inet_addr()
#include <string.h>     //memset()
#include <unistd.h>      

int main(int argc, char* argv[]){

   //Checking Valid # of Arguments
   if(argc != 2){
      fprintf(stderr, "Error: wrong input formate, argc != 2\n");
      return EXIT_FAILURE;
   }
   
   //Variables
   pid_t pid;
   char recvBuff[1500];
   char sendBuff[1500];               //holds what to sent
   int byte_size = 0;            //counts bytes received
   int listenfd = 0;             //socket number for listen
   int connfd = 0;            
   
   memset(recvBuff, '0' ,sizeof(recvBuff));

   //Create Listening Socket
   listenfd = socket(AF_INET, SOCK_STREAM, 0);
   if(listenfd < 0){
      fprintf(stderr, "Error : Could not create socket \n");
      return EXIT_FAILURE;
   }
   
   //Initializing Server Information
   struct sockaddr_in proxy_addr; 
   proxy_addr.sin_family = AF_INET;    
   proxy_addr.sin_addr.s_addr = INADDR_ANY; //what ip to use
   proxy_addr.sin_port = htons(atoi(argv[1]));    
 
   //Binding Information to Socket
   bind(listenfd, (struct sockaddr*)&proxy_addr,sizeof(proxy_addr));

      //Listening on Socket, Blocks
   if(listen(listenfd, 5) < 0){
      fprintf(stderr, "Error: Failed to listen\n");
      return EXIT_FAILURE;
   }
         
   while(1){
      //Accept Connection
      connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
      if (connfd < 0){
         fprintf(stderr, "Problem in accepting connection\n");
      }
      
      //make new process
      pid = fork();
      if (pid == 0){ //if child
      
         //Receive Data
         byte_size = recv(connfd,recvBuff,1500,0);
         recvBuff[byte_size] = '\0';
         
         fprintf(stdout, "I received %d bytes: \n %s", byte_size, recvBuff);
         
      }else{
         close(connfd);
      }
   }
   close(listenfd);

   return 0;
}
