#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define MAX_DATA_LENGTH 80
#define PORT 8974
#define SA struct sockaddr
void func(int sockfd) {
  char buff[MAX_DATA_LENGTH];
  int n;
  for (;;) {
    bzero(buff, sizeof(buff));
    printf("\nEnter the string : ");
    n = 0;
    while ((buff[n++] = getchar()) != '\n')
      ;
    write(sockfd, buff, sizeof(buff));
    bzero(buff, sizeof(buff));
    read(sockfd, buff, sizeof(buff));
    printf("From Server : %s", buff);
    if ((strncmp(buff, "exit", 4)) == 0) {
      printf("Client Exit...\n");
      break;
    }
  }
}

int main() {
  int sockfd, connfd;
  struct sockaddr_in servaddr, cli;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sockfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  } else
    printf("Socket successfully created..\n");
  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(PORT);

  if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
    printf("connection with the server failed...\n");
    exit(0);
  } else
    printf("connected to the server..\n");

  func(sockfd);

  close(sockfd);
}
