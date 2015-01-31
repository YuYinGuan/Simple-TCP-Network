#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //Socket data types
#include <sys/socket.h> //socket(), connect(), send(), and recv()
#include <netinet/in.h> //IP Socket data types
#include <arpa/inet.h>  //for sockaddr_in and inet_addr()
#include <string.h>     //memset()
#include <unistd.h>     //close() 

int main(int argc, char* argv[]){

   //Variables
   int byte_size = 0;      //tract how many byte received
   char recvBuff[1024];    //store received data
   char* sendBuff = "Test Message";         //stores what to sent over the network
   char PW[64];            //store user input of password
   int shutDown = 0;       //flag for request to shut down server
   int sockfd = 0;         //socket for connection
   
   //Initializing Buffer to 0s
   memset(recvBuff, '0' ,sizeof(recvBuff));
   
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if(sockfd < 0){
      fprintf(stderr, "Error : Could not create socket \n");
      return EXIT_FAILURE;
   }
   
   //Server Address and Other Information for Connection
   struct sockaddr_in servAddr;
   servAddr.sin_family = AF_INET;
   servAddr.sin_port = htons(atoi(argv[2]));
   servAddr.sin_addr.s_addr = inet_addr(argv[1]);
  
   //Connect with Socket 
   if(connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr))< 0){
      fprintf(stderr, "Error : Connect Failed \n");
      return EXIT_FAILURE;
   }

   //Send Buffer Data
   send(sockfd, sendBuff, strlen(sendBuff), 0);
   
   //Close Socket
   close(sockfd);
   
   return 0;
}
