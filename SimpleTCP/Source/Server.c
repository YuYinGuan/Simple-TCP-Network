#include <stdio.h>
#include <stdlib.h>
#include <time.h>       //Time function
#include <sys/types.h>  //Socket data types
#include <sys/socket.h> //socket(), connect(), send(), and recv()
#include <netinet/in.h> //IP Socket data types
#include <arpa/inet.h>  //for sockaddr_in and inet_addr()
#include <string.h>     //memset()
#include <unistd.h>     //close() 

int main(int argc, char* argv[]){

   //Checking Valid # of Arguments
   if(argc != 2){
      fprintf(stderr, "Error: wrong input formate, argc != 2\n");
      return EXIT_FAILURE;
   }
   
   //Variables
   char recvBuff[64];
   time_t dateTime;              //used for checking for time 
   char* sendBuff;               //holds what to sent
   char* rightPW = "Right";      //response to shutdown password
   char* wrongPW = "Wrong";      //response to shutdown password
   char* namePW = "Password";    //request sent to client for password
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
   struct sockaddr_in serv_addr; 
   serv_addr.sin_family = AF_INET;    
   serv_addr.sin_addr.s_addr = INADDR_ANY; //what ip to use
   serv_addr.sin_port = htons(atoi(argv[1]));    
 
   //Binding Information to Socket
   bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));


	//Listening on Socket, Blocks
      if(listen(listenfd, 5) < 0){
         fprintf(stderr, "Error: Failed to listen\n");
         return EXIT_FAILURE;
      }
   while(1){
      //Accept Connection
      connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
   
      //Receive Data of Max 64 Byte
      byte_size = recv(connfd,recvBuff,64,0);
      recvBuff[byte_size] = '\0';

      if(strcmp(recvBuff, "Time?")==0){
         //Retrieve Current Time
         time(&dateTime);
         sendBuff = ctime(&dateTime);
         //Send the Time Across 
         send(connfd, sendBuff, strlen(sendBuff), 0);
      
      }else if(strcmp(recvBuff, "Shut!")==0){
         send(connfd, namePW, strlen(namePW), 0);
         byte_size = recv(connfd,recvBuff,64,0);
         recvBuff[byte_size-1] = '\0';
         if(strcmp(recvBuff, "shutdown")==0){
            send(connfd, rightPW, strlen(rightPW), 0);
            break;
         }else{
            send(connfd, wrongPW, strlen(wrongPW), 0);
         }
      }

      //Close Given Socket
      close(connfd); 
   }
   close(listenfd);

   return 0;
}
