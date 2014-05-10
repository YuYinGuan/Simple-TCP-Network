#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#define bufMaxSize 60000 

//#define DebugMode1       //init & transfer has small delay
//#define DebugMode2       //transfer has long delay for timeout and init has small delay
//#define DebugMode3       //init has long delay for timeout


int main(int argc, char* argv[]) {

   //Check for correct user Arguments
   if(argc != 2) {
      fprintf(stderr, "Usage: %s <port>\n", argv[0]);
      return EXIT_FAILURE;
   }
   
//+++++++++++++++++++++++++++++++++++++++++Variables+++++++++++++++++++++++++++++++

   
   //General Variable & Variables for select() function
   int i, byteSize, listenfd, maxfd, nready, connfd, sockfd, client[FD_SETSIZE];
   fd_set allset,rset;
   int portCon[FD_SETSIZE], maxi; 
   char *token;
   
   //Variables for File and Bytes to read
   FILE *reqFile;
   int startByte, reqByteSize;
   
   //Buffer Declarations
   char buf[bufMaxSize];
   char *noFileError = "No Such File";
   char fileBytes[bufMaxSize];
   char readBuf[bufMaxSize];
   char tempSendBuf[1024];
   
   //Variable for connection sock address
   struct sockaddr_in servAddr, cliAddr, tempAddr;
   int len = sizeof(struct sockaddr_in);
      
   //Variables for Timer of Select
   struct timeval tv;
   tv.tv_sec;
   tv.tv_usec;
   
//+++++++++++++++++++++++++++++++++++++++++ INITIALIZATION +++++++++++++++++++++++++++++++

   // initialize socket for listening
   if ((listenfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
      fprintf(stderr, "Error: Failed to Create Socket\n");
      return EXIT_FAILURE;
   }

   //Set all client to have -1 as no client & port for different connections
   for(i = 0; i<FD_SETSIZE; i++){
      portCon[i] = i+4000;
      client[i] = -1;
   }
   
   //Initialize the listenfd port, Ip other info of socket
   memset((char *) &servAddr, 0, sizeof(struct sockaddr_in));
   servAddr.sin_family = AF_INET;
   servAddr.sin_port = htons(atoi(argv[1]));
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   
   //Bind the listen Socket to these information
   if (bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) == -1) {
      fprintf(stderr, "Error: Failed to Bind Socket\n");
      return EXIT_FAILURE;
   }

   //Setup the group of fd for select()
   FD_ZERO(&allset);
   FD_SET(listenfd, &allset);    //add the listenfd to the list
   maxfd = listenfd;
   maxi = -1;

//************************************** MAIN LOOP ************************************************//
   while(1){
      //Reset Timer Values
      tv.tv_sec = 10;
      tv.tv_usec = 0;
      
      //reset buffers
      memset(buf, 0, sizeof(buf));
      memset(fileBytes, 0, sizeof(fileBytes));
      memset(readBuf, 0, sizeof(readBuf));
      
      //Copy all fd into another one for manipulation
      rset = allset;

      //select with timeout of 10 second
      nready = select(maxfd +1, &rset, NULL, NULL,&tv);
      
      //if select errored, best to end execution
	   if (nready == -1){
	       perror("select()");
          return EXIT_FAILURE;
	   }else if(nready){           
	       //printf("Data is available now.\n");   //For debugging
	   }else{
         for(i =0; i<= maxi; i++){     //if no one talked for a while, disconnect all connection
            close(client[i]);
            FD_CLR(client[i], &allset);
            client[i] = -1;  
         }
      }
      
   //+++++++++++++++++++++++++++++++++++++++++ RECEIVED FROM LISTEN SOCKET +++++++++++++++++++++++++++++++
      if(FD_ISSET(listenfd, &rset)){
      
         //For debugging, creates lag for dropped packets
#ifdef DebugMode1
               usleep(200000);
#endif

#ifdef DebugMode2
               usleep(200000);
#endif

#ifdef DebugMode3
               usleep(1100000);
#endif

         //Find a slot on client array that is empty
         for(i=0; i<FD_SETSIZE; i++){

            if(client[i] < 0){
            
               //Receive the avaible bytes
               byteSize = recvfrom(listenfd, buf, bufMaxSize, 0, (struct sockaddr *) &cliAddr, &len);
               buf[byteSize]='\0';
               
               //++++++++++++++++++++++++++++ NEW SOCKET FOR CONNECTION ++++++++++++++++++++++++++
               //Creates New Socket
               if ((connfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                  fprintf(stderr, "Error: Failed to Create Socket %d\n",i);
                  printf("ERROR: %s\n", strerror(errno));
                  return EXIT_FAILURE;
               }
               
               //Set up socket address for port other different from Listenfd
               memset((char *) &tempAddr, 0, sizeof(struct sockaddr_in));
               tempAddr.sin_family = AF_INET;
               tempAddr.sin_port = portCon[i];
               tempAddr.sin_addr.s_addr = htonl(INADDR_ANY);
               
               //++++++++++++++++++++++++++++ REQUEST CHECK & HANDLE ++++++++++++++++++++++++++
               
               //Check if it is a request for init connection
               token = strtok(buf,":");
               if(!strcmp(token, "File Name")){       //if it is Init for connection
                  
                  //Bind the new port and info to this socket
                  if (bind(connfd, (struct sockaddr *) &tempAddr, sizeof(tempAddr)) == -1) {
                     fprintf(stderr, "Error: Failed to Bind Socket with Port %d\n", i);
                     printf("ERROR: %s\n", strerror(errno));
                     return EXIT_FAILURE;
                  }   
                  
                  //Check the file name
                  token = strtok(NULL,":");
                  reqFile = fopen(token, "r");
                  
                  //Check if there is such a file
                  if(reqFile==NULL ){           //If no such file, send no such file back and Skip the rest
                     sendto(connfd, noFileError, strlen(noFileError), 0, (struct sockaddr *) &cliAddr, len);
                     continue;                  
                  }else{      //if there is such a file
                  
                     //Look for the end of the file in Byte and send it to the Client
                     fseek(reqFile, 0, SEEK_END);       
                     snprintf(fileBytes, 100, "File Size:%ld", ftell(reqFile));
                     sendto(connfd, fileBytes, strlen(fileBytes), 0, (struct sockaddr *) &cliAddr, len);
                     
                     //Close the file since it is opened
                     fclose(reqFile);
                  }
                  //Save this socket for future use
                  client[i] = connfd;
               }
               break;  
            }
         }
         
      //++++++++++++++++++++++++++++ Variable Admin ++++++++++++++++++++++++++
      
         //If too many client, server exit and crashes
         if(i == FD_SETSIZE){
            fprintf(stderr, "ERROR: Too Many Clients");
            return EXIT_FAILURE;
         }

         //Update sockets to allset
         FD_SET(connfd, &allset);
         if(connfd > maxfd){
            maxfd = connfd;
         }
         
         //update amount of sockets
         if(i>maxi){
            maxi = i;
         }
         
         //check if all available socket is readed, if so just skip the rest
         if(--nready <=0){
            continue;
         }
      }
      
   //+++++++++++++++++++++++++++++++++++++++++ RECEIVED FROM CLIENT SOCKET +++++++++++++++++++++++++++++++   
      for(i =0; i<= maxi; i++){
      
         //If there is no client, just skip this
         if((sockfd = client[i])<0){
            continue;
         }
         
         //if there is client available
         if(FD_ISSET(sockfd, &rset)){
         
            //read the mesg and parse it
            byteSize = recvfrom(sockfd, buf, bufMaxSize, 0, (struct sockaddr *) &cliAddr, &len);
            token = strtok(buf,":");

            //if it is an end connection request, end the connection and clean up    
            if(!strcmp(token, "END")){
               close(sockfd);
               FD_CLR(sockfd, &allset);
               client[i] = -1;
            }
            
            //if Request for Data
            if(!strcmp(token, "File Data")){
            
#ifdef DebugMode1
               usleep(200000);
#endif
#ifdef DebugMode2
               usleep(5000000);
#endif
               
               //parse the mesg and open up the request file
               token = strtok(NULL,":");
               reqFile = fopen(token, "r");
               
               //If no such files, then send no such file to Client
               if(reqFile==NULL ){
                  sendto(sockfd, noFileError, strlen(noFileError), 0, (struct sockaddr *) &cliAddr, len);
                  continue;
               }else{               //if there is such file
               
                  //Parse the data to get the start byte and amount of bytes from that point it is requesting
                  token = strtok(NULL,":");
                  startByte = atoi(token);
                  token = strtok(NULL,":");
                  reqByteSize = atoi(token);
                  
                  //Move file pointer to that byte and read for requested amount and store it
                  fseek(reqFile, startByte, SEEK_SET);
                  byteSize = fread(readBuf, sizeof(char), reqByteSize,reqFile);
                  snprintf(tempSendBuf, 1024, "%d:%d:%s",startByte, reqByteSize, readBuf);
                  
                  //Sent the Data to the Client
                  if (sendto(sockfd, tempSendBuf, strlen(tempSendBuf), 0, (struct sockaddr *) &cliAddr, len) == -1) {
                     perror("sendto()");
                     return EXIT_FAILURE;
                  }
                  
                  //Close the open file
                  fclose(reqFile);
               }
            }
            
            //if there is no more available socket to be read, skip the for loop
            if(--nready <=0){
               break;
            }
         }
      }
   }
   
   //if it get here at any point in any ways, clean up  #should never hit here
   close(listenfd);
   return 0;
}