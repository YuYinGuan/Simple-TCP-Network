#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

void error(char* msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char* argv[])
{
  pid_t pid;
  struct sockaddr_in addr_in, cli_addr, serv_addr;
  struct hostent* host;
  int sockfd, newsockfd;

  if (argc < 2)
    error("./proxy <port_no>");

  printf("\n*****WELCOME TO PROXY SERVER*****\n");
  printf("\nCopyright (c) 2014  GODLY T.ALIAS\n\n");

  bzero((char*)&serv_addr, sizeof(serv_addr));
  bzero((char*)&cli_addr, sizeof(cli_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[1]));
  serv_addr.sin_addr.s_addr = INADDR_ANY;


  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0)
    error("Problem in initializing socket");

  if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    error("Error on binding");


  listen(sockfd, 50);
  int clilen = sizeof(cli_addr);



accepting:

  newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

  if (newsockfd < 0)
    error("Problem in accepting connection");

  pid = fork();
  if (pid == 0)
  {
    struct sockaddr_in host_addr;
    int flag = 0, newsockfd1, n, port = 0, i, sockfd1;
    char buffer[510], t1[300], t2[300], t3[10];
    char* temp = NULL;
    bzero((char*)buffer, 500);
    recv(newsockfd, buffer, 500, 0);

    printf("I got:\n %s",buffer);
    
    close(sockfd1);
    close(newsockfd);
    close(sockfd);
    _exit(0);
  }
  return 0;
}