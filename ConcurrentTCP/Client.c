#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  //Socket data types
#include <sys/socket.h> //socket(), connect(), send(), and recv()
#include <netinet/in.h> //IP Socket data types
#include <arpa/inet.h>  //for sockaddr_in and inet_addr()
#include <pthread.h>
#include <unistd.h>

struct thread_info{    /* Used as argument to thread_start() */
   pthread_t   thread_id;        /* ID returned by pthread_create() */
   int        thread_num;       /* Application-defined thread # */
   int         sockfd;
   char *      fileName;
};

static void * thread_start(void *arg){
   struct thread_info *threadInfo = arg;
   char *idFlag1 = "FileName";
   write(threadInfo->sockfd, idFlag1, strlen(idFlag1));  
   close(threadInfo->sockfd);
   pthread_exit((void*) arg);
}

int main(int argc, char* argv[]){

   
   //Variables
   int numCon = atoi(argv[2]);         //Number of connections
   int serNum;                         //server Number used in forloop
   struct thread_info *pThreads;       //pointer to group threads_info structure
   
   //variables for reading files
   FILE *serList;
   char line[250];
   char *token;
   int  Port;
   char *IP;
   
   
   //allocate memory to multiple thread_info structure
   pThreads = calloc(numCon, sizeof(struct thread_info));
   
   //check if memory allocated correctly
   if (pThreads== NULL){
      fprintf(stderr, "Error : Could not memory for thread_info Structure\n");
      return EXIT_FAILURE;
   }
   
   //checking if server list file is able to be opened
   if((serList = fopen("server-info.text", "r"))== NULL){
      fprintf(stderr,"ERROR: Unable to open read file.\n");
      return EXIT_FAILURE;
   }
   
   //server Information
   struct sockaddr_in servAddr;
   servAddr.sin_family = AF_INET;

   //Create Threads for connection
   for(serNum = 0; serNum< numCon; serNum++){
   
      //getting one line in the list of servers
      if(fgets(line, sizeof(line), serList)!=NULL){
      
         //retrieve IP of Server
         token = strtok(line, " \n\r");   
         IP = token;

         //retrieve Port of Server
         token = strtok(NULL, " \n\r");
         Port = atoi(token);
      }  
      
      //Update Server Information
      servAddr.sin_port = htons(Port);
      servAddr.sin_addr.s_addr = inet_addr(IP);
      
      //create a socket for each server and store it in structure
      if((pThreads[serNum].sockfd = socket(AF_INET, SOCK_STREAM,0))< 0){
         fprintf(stderr, "Error : Could not create socket #%d \n", serNum);
         return EXIT_FAILURE;
      }
      
      //connect to servers
      printf("Port %d, Ip %s\n", Port, IP);
      if(connect(pThreads[serNum].sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr))< 0){
         fprintf(stderr, "Error : Connect to socket #%d Failed \n", serNum);
         return EXIT_FAILURE;
      }
      
      //Add information to for threads to communicate to that server
      pThreads[serNum].thread_num = serNum;           //its server Number
      pThreads[serNum].fileName = argv[1];            //file of retrieval 
      
      //create a thread to handle this
      pthread_create(&pThreads[serNum].thread_id, NULL, thread_start, &pThreads[serNum]);
   }
   
   //wait for all thread to finish
   for(serNum = 0; serNum< numCon; serNum++){
      pthread_join(pThreads[serNum].thread_id, NULL);
   }
   
   
   free(pThreads);
   pthread_exit(NULL);
   return 0;
}
