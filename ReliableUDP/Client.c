#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>

#define SIZE 1024

//************************************** THREAD ARGUMENT ************************************************//

struct thread_info{    
   pthread_t   thread_id;        
   struct sockaddr_in servAddr;
   FILE        *outFile;
   char*       fileName;
   char*       FileData;
   int         thread_num;       /* Application-defined thread # */
   int         sockfd;
   int         len;
   int         numberConnection;
   int         reqByteSize;
   int         startByte;
   int         flag;
};

//************************************** THREAD ************************************************//
static void * thread_start(void *arg){
   
   //++++++++++++++++++++++++++ THREAD VARIABLES ++++++++++++++++++++++++++++++++++++++++
   //General Variables
   struct thread_info *threadInfo = arg;     //Pharse the input for threads
   char tBuf[SIZE];
   char *endReq = "END";                     //Buffer for End connection command
   int n;
   int *token1, *token2;                     
   
   //Variable Start Bytes and Size for data request
   int s_Byte = threadInfo->startByte;       //Start Bytes
   int fileSize = threadInfo->reqByteSize;   //Total amount requesting
   int r_Byte = 1000;                        // divide, or max udp transfer request byte size
   
   //TimeOut Variables
   int tTimeOut = 10;
   int tTries = 0;
   
   //Init flag for successful transfer and allocate memory for data requested
   threadInfo->FileData = (char *)malloc(fileSize*sizeof(char));
   threadInfo->flag = 0;
   
   //*********************************** MAIN THREAD LOOP ***********************************//   
   while(1){
   
      //Determine the amount of bytes for request
      if(fileSize<=0){     //keep requesting 1000Bytes of data
         break;
      }else if(fileSize <1000){  //request the rest of the left over Bytes
         r_Byte = fileSize;
      }
      
      //Reseting Buffer
      memset(tBuf, 0, sizeof(tBuf));
      snprintf(tBuf, 100, "File Data:%s:%d:%d", threadInfo->fileName, s_Byte, r_Byte);

      //Request for data
      if (sendto(threadInfo->sockfd, tBuf, strlen(tBuf), 0, (struct sockaddr *) &threadInfo->servAddr, threadInfo->len) == -1) {
         fprintf(stderr,"ERROR: Unable send file name\n");
         threadInfo->flag = 1;
         break;
      }
      
      //Retrieve data from Server
      if((n = recvfrom(threadInfo->sockfd, tBuf, SIZE, 0, (struct sockaddr *) &threadInfo->servAddr, &threadInfo->len)) != -1) {
         //reset timeout tries
         tTries = 0;
         tBuf[n]='\0';
         
         //Get the bytes received indicated by Server
         token1 = strtok(tBuf, ":");
         token2 = strtok(NULL, ":");
         
         //If wrong Packet, skip it, else store data
         if((atoi(token1) != s_Byte) || (atoi(token2) != r_Byte)){
            continue;
         }else{
            token2 = strtok(NULL, ":");
         }

         //Put the small chucks into Request Bytes
         strcat(threadInfo->FileData, token2);
         
         //Setup next start byte and request bytes
         s_Byte += r_Byte;
         fileSize -= r_Byte;
         
      }else{
      
         //Times Out,
         if(++tTries>tTimeOut){     //if more than 10 tries
            threadInfo->flag = 1;      //exit and flag something went wrong
            break;
         }
      }
      
   }

   //+++++++++++++++++++++++++++++ END TRANSFER & CLEAN UP/WRITE DATA+++++++++++++++++++++++++++++++++
 
   //Notify Server for Ending transfer
   if (sendto(threadInfo->sockfd, endReq, strlen(endReq), 0, (struct sockaddr *) &threadInfo->servAddr, threadInfo->len) == -1) {
      perror("sendto()");
      return EXIT_FAILURE;
   }   

   //If nothing goes wrong, write the data into the file   
   if(threadInfo->flag == 0){
      fseek(threadInfo->outFile,threadInfo->startByte, SEEK_SET);
      fwrite(threadInfo->FileData, sizeof(char), threadInfo->reqByteSize ,threadInfo->outFile);
   }
   
   //Clean Up
   free(threadInfo->FileData);
   close(threadInfo->sockfd);
   pthread_exit((void*) arg);
}


//************************************** MAIN ************************************************//
int main(int argc, char *argv[]) {

   //Checking for Valid user arguments
   if(argc !=3){
      fprintf(stderr, "Usage: %s <file name> <number of connections>\n", argv[0]);
      return EXIT_FAILURE;
   }
   
   //++++++++++++++++++++++++++++++++++++++++++ VARIABLES +++++++++++++++++++++++++++++++++++++++++++++++++++
   
   //General Variables
   char buf[SIZE];               //General Buffer
   int n, i, t;                  //general variables
   char *token;                  //for tokenizing
   int allCon = atoi(argv[2]);   //# of chunks of data
   int numCon = 0;               //Number of connections made successfully
   void *status;                 //Stores return of Pthreads
   int wholeFile;                //Byte Size of the entire file
   int connectFlag[allCon+1];    //Used to check which chunk is already received 
   
   //Variables for Servers
   struct sockaddr_in servAddr;
   int serNum;                         //server Number used in forloop
   int len = sizeof(struct sockaddr_in);
   
   //Variable for Pthread
   struct thread_info *pThreads;       //pointer to group threads_info structure
   
   //Variables for checking if Chunks > # servers
   int counter = 0; //used to check how many lines in Server List
   
   //Variables for reading from server-info.text and IO
   FILE *serList;
   FILE *outFile;
   char line[250];
   char *s_ip;
   int  s_port;
   
   //Variables for Timeouts
   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = 100000; //0.1 second
   int timeOut = 10;    //amount of timeouts to be consider disconnected
   int timeTries;       //amount of timeouts

   
   //++++++++++++++++++++++++++++++++ INITIALIZATION ++++++++++++++++++++++++++++++++++
   
   //allocate memory to multiple thread_info structure
   pThreads = calloc(allCon, sizeof(struct thread_info));
   
   //check if memory allocated correctly
   if (pThreads== NULL){
      fprintf(stderr, "Error : Could not memory for thread_info Structure\n");
      return EXIT_FAILURE;          //exit failed is no memory can be allocated for threads
   }
   
   //checking if server list file is able to be opened
   if((serList = fopen("server-info.text", "r"))== NULL){
      fprintf(stderr,"ERROR: Unable to open read file.\n");
      return EXIT_FAILURE;       //exit fail if can't open server list
   }
   
   //create and check if file can be written into
   if((outFile = fopen("output.txt", "w"))== NULL){                                          /////////argv[1]
      fprintf(stderr,"ERROR: Unable to open output file.\n");
      return EXIT_FAILURE;    //exit fail if can't write to file
   }
   
   //name chunks from 0 to defined chunks
   for(i = 0; i<allCon; i++){
      connectFlag[i] = i;
   }
   connectFlag[allCon] = -2;        //put -2 as symbol of the end of untransfered chunks
   
   //Count number of lines in server-info.text, means number of servers in this files
   while (!feof(serList)){
      counter += (fgetc(serList) == '\n');
   }
   counter +=1; //add one because the last line shouldn't have line space

   
   //**************************************** MAIN LOOP***********************************//
   do{
      //+++++++++++++++++++++++++++ INITIALIZATION OF VARIABLES (EVERYLOOP) +++++++++++++++++++++++++++
      
      //checking if there are more chunks than servers
      if(allCon > counter){
         numCon = counter;
      }else{
         numCon = allCon;
      }
      
      //set the file pointer back to beginning
      fseek(serList,0,SEEK_SET);
      
      //for all connections
      for(serNum = 0; serNum< numCon; serNum++){
         timeTries = 0;
         
         //Get one line of Server Info
         if(fgets(line, sizeof(line), serList)!=NULL){
            s_ip = strtok(line, " \n\r");             //retrieve IP of Server 
            s_port = atoi(strtok(NULL, " \n\r"));     //retrieve Port of Server
         }
      
         //create a socket for each server and store it in structure, check if socket can be created
         if((pThreads[serNum].sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))== -1){
            fprintf(stderr, "Error : Create socket #%d \n", serNum);
            return EXIT_FAILURE;                                                    //program failed if socket can not be created
         }
         
         //Set a time out for recvfrom
         if (setsockopt(pThreads[serNum].sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            perror("setsockopt");
         }
         
         //initialize servAddr with ip and port from file
         memset((char *) &servAddr, 0, sizeof(struct sockaddr_in));
         servAddr.sin_family = AF_INET;
         servAddr.sin_port= htons(s_port);
         servAddr.sin_addr.s_addr = inet_addr(s_ip);
    

   //++++++++++++++++++++++++++ CONNECT WITH SERVERS AND MAKING THREADS FOR THEM +++++++++++++++++++++   
CONNECT:
         
         //Reset buffer
         memset(buf, 0, sizeof(buf));
         snprintf(buf, sizeof(buf), "File Name:%s", argv[1]);
         
         
         //connect function using UDP, Init with request for file size
         if (sendto(pThreads[serNum].sockfd, buf, strlen(buf), 0, (struct sockaddr *) &servAddr, len) == -1) {
            fprintf(stderr, "Error: Sendto socket #%s \n", s_ip);
            --serNum;                                                      //skip this server if can not sent to it
            --numCon;
            continue;
         }
         
         //reseting buffer
         memset(buf, 0, sizeof(buf));
        
         //receiving ACK/File_size back, if timeout, Jump to connect for sending request for connect again, up to 10 times
         if((n = recvfrom(pThreads[serNum].sockfd, buf, SIZE, 0, (struct sockaddr *) &servAddr, &len)) != -1) { //servAddr should have new port in it
            servAddr.sin_port= htons(ntohs(servAddr.sin_port));
            servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(servAddr.sin_addr));
            token = strtok(buf, ":");
            
            //Server said there is no such file
            if(!strcmp(token, "No Such File")){
               --serNum;                                                      //skip this server if can not sent to it
               --numCon;
               continue;
            }
            
            //Server said there is such file, and here is the size of it
            if(!strcmp(token, "File Size")){
            
               token = strtok(NULL, ":");
               wholeFile = atoi(token);         //File Size
               
               //Add information to for threads to communicate to that server
               pThreads[serNum].servAddr = servAddr;           //Address with new port
               pThreads[serNum].outFile = outFile;             //File fd for writing the data
               pThreads[serNum].fileName = argv[1];            //file of retrieval 
               pThreads[serNum].thread_num = serNum;           //its server Number I assigned
               pThreads[serNum].len = len;
               pThreads[serNum].numberConnection = atoi(argv[2]);
               pThreads[serNum].reqByteSize = ceil((double)(wholeFile/allCon));    //Size of the chunk retrieving 
               pThreads[serNum].startByte = connectFlag[serNum] * pThreads[serNum].reqByteSize;        
            }
         }else{
            //If Timed Out, tries 10 more times and consider it unreachable since have not heard from it at all
            if(timeTries<timeOut){
               timeTries++;
               goto CONNECT;
            }else{
               --serNum;                                                      //skip this server if can not sent to it
               --numCon;
               continue;
            }
         }
         
         //create a thread to handle each server
         pthread_create(&pThreads[serNum].thread_id, NULL, thread_start, &pThreads[serNum]);
      }
         
      
   //+++++++++++++++++++++++++++++++++FLAG HANDLING++++++++++++++++++++++++++++++++++++++++++++++      
      //wait for all thread to finish, flag the ones that are finished -1
      for(serNum = 0; serNum< numCon; serNum++){
         pthread_join(pThreads[serNum].thread_id, &status);    //join all threads created up til this point
         if(pThreads[serNum].flag == 0){    //check if the thread finish and wrote to file with no problem                   
            connectFlag[serNum] = -1;     //set flag to be done
         }
      }
      
      //orgainize all the ones that still need processing on the left
      for(i = 0; i< allCon; i++){
         while(connectFlag[i] == -1){
            t = i;
            while(connectFlag[t] != -2){
               connectFlag[t] = connectFlag[t+1];
               t++;
            }
         }
      }
      
      //update all connection of chunks that is left to connect
      for(i = 0; i< allCon; i++){
         if(connectFlag[i] == -2){
            allCon = i;
         }
      }
   
   }while(allCon!=0);
   
//+++++++++++++++++++++++++++++++++++++++++CLOSE RESOURCES++++++++++++++++++++++++++++++++++++++++++++   
   
   fclose(outFile);
   fclose(serList);
   free(pThreads);
   pthread_exit(NULL);
   
   return 0;   
}