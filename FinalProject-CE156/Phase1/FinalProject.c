#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  //Socket data types
#include <sys/socket.h> //socket(), connect(), send(), and recv()
#include <netinet/in.h> //IP Socket data types
#include <arpa/inet.h>  //for sockaddr_in and inet_addr()
#include <string.h>     //memset()
#include <unistd.h>      
#include <netdb.h>
#include <errno.h>

int main(int argc, char* argv[]){

   fprintf(stderr, "\r\n/******************Yu Yin's Proxy********************/\r\n");
      
   //Checking Valid # of Arguments
   if(argc != 2){
      fprintf(stderr, "Error: wrong input formate, argc != 2\n");
      return EXIT_FAILURE;
   }
   
   //Variables
   pid_t pid;
   int byte_size = 0;            //counts bytes received
   int listenfd = 0, connfd = 0;       
   struct hostent* host;
   
   FILE *permitted_sites;
   FILE *logging;
   
   //checking if server list file is able to be opened
   if((permitted_sites = fopen("permitted_sites.txt", "r"))== NULL){
      fprintf(stderr,"ERROR: Unable to open read file.\n");
      return EXIT_FAILURE;       //exit fail if can't open server list
   }
   
   //create and check if file can be written into
   if((logging = fopen("logging.txt", "w"))== NULL){                                          /////////argv[1]
      fprintf(stderr,"ERROR: Unable to open output file.\n");
      return EXIT_FAILURE;    //exit fail if can't write to file
   }
   
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
         
         
   fprintf(stderr, "\r\n/******************Ready to Accept Connections********************/\r\n");
   
   
   
   
   

   //Logging:The proxy server should maintain a log file to track all requests received. It 
   //should print one line for each request with the following information: 
      //- Date and time the request was received 
      //- Type of request and HTTP version 
      //- Requesting host name 
      //- URI and server address 
      //- Action taken by proxy (forwarded, filtered, etc.) 
      //- Type of errors detected, if any
   
   
   //Persistent connections: 
      //Note that the browser will use persistent connections by default 
      //if it is using HTTP 1.1. In this case, either the client (browser) or the server may initiate 
      //the close. The proxy server must be able to deal with this by closing the connection on 
      //the other side when the TCP connection on one side closes.
   
   
   
   
   
   
   
   
   
   
   
   
   
   while(1){
      //Accept Connection
      connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
      if (connfd < 0){
         fprintf(stderr, "Problem in accepting connection\n");
      }
      
      //make new process
      pid = fork();
      if (pid == 0){ //if child
         char recvBuff[500];
         char method[50],URL[200],protocol[50], host_name[200],host_path[500];
         int newsocket = 0;             //socket number for listen
         int connectFlag = 0;     
         int permittedFlag = 0;
         int host_port;
         int connFlag = 1;
         char *token;
         char line[250];
         
         struct sockaddr_in host_addr;
         host_addr.sin_family = AF_INET;    
         
         memset(recvBuff, '0',500);
         memset(method, '0' ,50);
         memset(URL, '0' ,200);
         memset(protocol, '0' ,50);
         memset(host_name, '0' ,200);
         memset(host_path, '0' ,500);

         //Receive Data
         byte_size = recv(connfd,recvBuff,500,0);
         
         if(sscanf(recvBuff, "%s %s %s", method,URL,protocol)!=3){
            send(connfd, "400 : BAD REQUEST\n", 18, 0);
         }

         //checks if the URL is correctly parsed. If so, set the port appropriately, otherwise, send out BAD REQUEST
         if(strncasecmp(URL, "http://", 7 )==0){
            if(sscanf( URL, "http://%[^:/]:%d%s", host_name, host_port, host_path ) == 3 ){
               host_addr.sin_port = htons(host_port);    
            }else if(sscanf( URL, "http://%[^/]%s", host_name, host_path ) == 2 ){
               host_addr.sin_port = htons(80);
            }else if( sscanf( URL, "http://%[^:/]:%d", host_name, host_port ) == 2 ){
               host_addr.sin_port = htons(host_port); 
               *host_path = '\0';
            }else if( sscanf( URL, "http://%[^/]", host_name ) == 1 ){
               host_addr.sin_port = htons(host_port); 
               *host_path = '\0';
            }else{
               send(connfd, "400 : BAD REQUEST\n", 18, 0);
               connFlag = 0;
            }
         }else{
            send(connfd, "400 : BAD REQUEST\n", 18, 0);
            connFlag = 0;
         }
         
         //Checks if it is methods allowed to be handled, if not send out not implemented responds
         if(((strncasecmp(method, "GET", 3)!=0)) && ((strncasecmp(method, "HEAD", 4)!=0)) && ((strncasecmp(method, "POST", 4)!=0))){
            send(connfd, "501 : NOT IMPLEMENTED\n", 22, 0);
            connFlag = 0;
         }
         
         fseek(permitted_sites,0,SEEK_SET);
         while(fgets(line, sizeof(line), permitted_sites)!=NULL){
            token = strtok(line, " \n\r");   //to get rid of the space, \r or \n
            if(strcmp(token, host_name) == 0){
               permittedFlag = 1;
               break;
               //fprintf(stdout, "token == host_name\n");
            }
            //fprintf(stdout, "Read Line From permitted_sites.txt: %s\nstrcmp(token, host_name) = %d\n", line,strcmp(line, host_name));
         }
         
         //check if site is permitted, if not send forbidden URL and set connection flag to be false
         if(permittedFlag == 0){
            send(connfd, "403 : FORBIDDEN URL\n", 20, 0);
            connFlag = 0;
         }
         
         //check if the protocol is HTTP 1.1 or 1.0, if not send out 400 BAD REQUEST
         if(((strncasecmp(protocol, "HTTP/1.1", 8)==0) || (strncasecmp(protocol, "HTTP/1.0", 8)==0))&&(connFlag == 1)){
            host = gethostbyname(host_name);
            memcpy(&host_addr.sin_addr, host->h_addr_list[0],sizeof(struct in_addr));
           
           
            fprintf(stdout, "I received %d bytes:\n%s\n\nmethod = %s\nURL = %s\nprotocol = %s\nhost_name = %s\nhost_path = %s\n", byte_size,recvBuff, method,URL,protocol,host_name,host_path);
            
            
            newsocket = socket(AF_INET, SOCK_STREAM, 0);
            connectFlag = connect(newsocket, (struct sockaddr*)&host_addr, sizeof(struct sockaddr));
            
            if(connectFlag < 0){
               fprintf(stdout, "Error in connecting to remote server\n");
            }
            fprintf(stdout, "\nConnected to %s  IP - %s\n", host_name, inet_ntoa(host_addr.sin_addr));
            
            
            
            byte_size = send(newsocket, recvBuff, strlen(recvBuff), 0);
            if(byte_size<0){
               fprintf(stdout, "Error writing to socket:%s \n",strerror(errno));
            }else{
               do{
                  memset(recvBuff, '0',500);
                  byte_size = recv(newsocket, recvBuff, 500, 0);
                  if (byte_size>0){
                     send(connfd, recvBuff, byte_size, 0);
                  }
               }while(byte_size > 0);
            }
            fprintf(stdout, "The end\n");
            
            close(connectFlag);
            close(newsocket);
         }else{
            if(permittedFlag == 1){
               send(connfd, "400 : BAD REQUEST\n", 18, 0);
            }
         }
      }
      close(connfd);
   }
   close(listenfd);

   return 0;
}
