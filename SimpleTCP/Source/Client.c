#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //Socket data types
#include <sys/socket.h> //socket(), connect(), send(), and recv()
#include <netinet/in.h> //IP Socket data types
#include <arpa/inet.h>  //for sockaddr_in and inet_addr()
#include <string.h>     //memset()
#include <unistd.h>     //close() 

int main(int argc, char* argv[]){

   //Checking Valid # of Arguments
   if((argc != 4)&&(argc != 3)){
      fprintf(stderr, "Error: wrong input formate, argc != 3 && 4\n");
      return EXIT_FAILURE;
   }
   
   //Variables
   int byte_size = 0;      //tract how many byte received
   char recvBuff[1024];    //store received data
   char* sendBuff;         //stores what to sent over the network
   char PW[64];            //store user input of password
   int shutDown = 0;       //flag for request to shut down server
   int sockfd = 0;         //socket for connection
   
   //Initializing Buffer to 0s
   memset(recvBuff, '0' ,sizeof(recvBuff));
   
   //Check Shut Down Server argument
   if((argc == 4) && !strcmp(argv[1], "-s")){
      shutDown = 1;
   }
   
   //Set Request to Sever, Ask for Password to Shut Down Sever
   if(shutDown == 0){
      sendBuff = "Time?";
   }else{
      fprintf(stdout,"Password: ");      
      fgets(PW, 64, stdin);
      sendBuff = "Shut!";
   }
   
   //Create Socket for Connection
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if(sockfd < 0){
      fprintf(stderr, "Error : Could not create socket \n");
      return EXIT_FAILURE;
   }
   
   //Server Address and Other Information for Connection
   struct sockaddr_in servAddr;
   servAddr.sin_family = AF_INET;
   
   //Information Location Based on Arguments
   if(shutDown == 0){                                    //ask for time
      servAddr.sin_port = htons(atoi(argv[2]));
      servAddr.sin_addr.s_addr = inet_addr(argv[1]);
   }else{                                                //shut down
      servAddr.sin_port = htons(atoi(argv[3]));
      servAddr.sin_addr.s_addr = inet_addr(argv[2]);
   }
   
   //Connect with Socket 
   if(connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr))< 0){
      fprintf(stderr, "Error : Connect Failed \n");
      return EXIT_FAILURE;
   }

   //Send Buffer Data
   send(sockfd, sendBuff, strlen(sendBuff), 0);
   
   //Receive Data 
   byte_size = recv(sockfd,recvBuff,1024,0);
   recvBuff[byte_size] = '\0';
   
   //Shut Down procedure
   if(strcmp(recvBuff, "Password")==0){
      send(sockfd, PW, strlen(PW), 0);
      byte_size = recv(sockfd,recvBuff,1024,0);
      recvBuff[byte_size] = '\0';
      if(strcmp(recvBuff, "Wrong")==0){
         fprintf(stdout, " The Password is Wrong, Server will not shutdown\n");
      }else if(strcmp(recvBuff, "Right")==0){
         fprintf(stdout, " The Server is Shut Down\n");
      }else{
         fprintf(stderr, "Error: The return of the Server for Password is neither Right or Wrong\n");
      }
   }
   
   //Prints out Data
   if(shutDown == 0){
      fprintf(stdout, "The time now is %s", recvBuff);
   }
   
   //Close Socket
   close(sockfd);
   
   return 0;
}
