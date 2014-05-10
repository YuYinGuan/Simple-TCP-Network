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
   char buf[bufMaxSize];
   struct sockaddr_in servAddr, cliAddr, tempAddr;
   int len = sizeof(struct sockaddr_in);
   int i, byteSize, listenfd, port, maxfd, nready, connfd, sockfd, client[FD_SETSIZE];
   fd_set allset,rset;
   int portCon[FD_SETSIZE], maxi; 
   char *ackBuf = "ACK";
   char *noFileError = "No Such File";
   char *token;
   FILE *reqFile;
   char fileBytes[bufMaxSize];
   int startByte, reqByteSize;
   char readBuf[bufMaxSize];
   char tempSendBuf[1024];
      
   //Setup Timer for Select
   struct timeval tv;
   tv.tv_sec;
   tv.tv_usec;
   
   for(i = 0; i<FD_SETSIZE; i++){
      client[i] = -1;
   }

    /* initialize socket */
   if ((listenfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
      fprintf(stderr, "Error: Failed to Create Socket\n");
      return EXIT_FAILURE;
   }

    /* bind to server port */
   port = atoi(argv[1]);
   
   
   for(i = 0; i<FD_SETSIZE; i++){
      portCon[i] = i+4000;
      client[i] = -1;
   }
   memset((char *) &servAddr, 0, sizeof(struct sockaddr_in));
   
   servAddr.sin_family = AF_INET;
   servAddr.sin_port = htons(port);
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   
   if (bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) == -1) {
      fprintf(stderr, "Error: Failed to Bind Socket\n");
      return EXIT_FAILURE;
   }

   FD_ZERO(&allset);
   FD_SET(listenfd, &allset);
   
   maxfd = listenfd;
   maxi = -1;

   while(1){
      
      tv.tv_sec = 10;
      tv.tv_usec = 0;
      
      memset(buf, 0, sizeof(buf));
      memset(fileBytes, 0, sizeof(fileBytes));
      memset(readBuf, 0, sizeof(readBuf));
      
      rset = allset;
      
      nready = select(maxfd +1, &rset, NULL, NULL,&tv);
      
      //if select errored, best to end execution
	   if (nready == -1){
	       perror("select()");
          return EXIT_FAILURE;
	   }else if(nready){           
	       printf("Data is available now.\n");   //For debugging
	   }else{
         for(i =0; i<= maxi; i++){     //if no one talked for a while, disconnect all connection
            close(client[i]);
            FD_CLR(client[i], &allset);
            client[i] = -1;  
         }
      }
      
      if(FD_ISSET(listenfd, &rset)){
         
#ifdef DebugMode1
               usleep(200000);
#endif

#ifdef DebugMode2
               usleep(200000);
#endif

#ifdef DebugMode3
               usleep(1100000);
#endif



         for(i=0; i<FD_SETSIZE; i++){
            if(client[i] < 0){
               byteSize = recvfrom(listenfd, buf, bufMaxSize, 0, (struct sockaddr *) &cliAddr, &len);
               buf[byteSize]='\0';
               
               if ((connfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                  fprintf(stderr, "Error: Failed to Create Socket %d\n",i);
                  printf("ERROR: %s\n", strerror(errno));
                  return EXIT_FAILURE;
               }
               
               memset((char *) &tempAddr, 0, sizeof(struct sockaddr_in));
               tempAddr.sin_family = AF_INET;
               tempAddr.sin_port = portCon[i];
               tempAddr.sin_addr.s_addr = htonl(INADDR_ANY);
               
               if (bind(connfd, (struct sockaddr *) &tempAddr, sizeof(tempAddr)) == -1) {
                  fprintf(stderr, "Error: Failed to Bind Socket with Port %d\n", i);
                  printf("ERROR: %s\n", strerror(errno));
                  return EXIT_FAILURE;
               }   
               
               token = strtok(buf,":");
               
               if(!strcmp(token, "File Name")){
            
                  token = strtok(NULL,":");
                  reqFile = fopen(token, "r");
                  
                  if(reqFile==NULL ){
                     sendto(connfd, noFileError, strlen(noFileError), 0, (struct sockaddr *) &cliAddr, len);
                     continue;
                  }else{
                     fseek(reqFile, 0, SEEK_END);       
                     snprintf(fileBytes, 100, "File Size:%ld", ftell(reqFile));
                     sendto(connfd, fileBytes, strlen(fileBytes), 0, (struct sockaddr *) &cliAddr, len);
                     fclose(reqFile);
                  }
               }
               
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
         
            byteSize = recvfrom(sockfd, buf, bufMaxSize, 0, (struct sockaddr *) &cliAddr, &len);
            
            token = strtok(buf,":");

            if(!strcmp(token, "END")){
               close(sockfd);
               FD_CLR(sockfd, &allset);
               client[i] = -1;
            }
            
            if(!strcmp(token, "File Data")){
            
#ifdef DebugMode1
               usleep(200000);
#endif
#ifdef DebugMode2
               usleep(5000000);
#endif
               
               token = strtok(NULL,":");
               reqFile = fopen(token, "r");
               if(reqFile==NULL ){
                  sendto(sockfd, noFileError, strlen(noFileError), 0, (struct sockaddr *) &cliAddr, len);
                  continue;
               }else{
                  token = strtok(NULL,":");
                  startByte = atoi(token);


                  token = strtok(NULL,":");
                  reqByteSize = atoi(token);
                  
                  fseek(reqFile, startByte, SEEK_SET);
                  
                  
                  
                  //if statment, if it is too big, divide it and send it in formate of the following
                  //File part:x:y: Data, where x and y is x/y as in 1/3 or 2/3 or 12/23 or so on
                  //loop until all all has been sent
                  
                  
                  
                  byteSize = fread(readBuf, sizeof(char), reqByteSize,reqFile);
                  
                  snprintf(tempSendBuf, 1024, "%d:%d:%s",startByte, reqByteSize, readBuf);
                     if (sendto(sockfd, tempSendBuf, strlen(tempSendBuf), 0, (struct sockaddr *) &cliAddr, len) == -1) {
                        perror("sendto()");
                        return EXIT_FAILURE;
                     }
                  fclose(reqFile);
               }
            }
            
            if(--nready <=0){
               break;
            }
         }
      }
   }
   
   close(listenfd);
   return 0;
}