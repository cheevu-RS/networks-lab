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
char *strrev(char *str) {
  char *p1, *p2;

  if (!str || !*str)
    return str;
  for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
    *p1 ^= *p2;
    *p2 ^= *p1;
    *p1 ^= *p2;
  }
  return str;
}

void func(int sockfd) {
  char buff[MAX_DATA_LENGTH];
  int i;
  for (;;) {
    bzero(buff, MAX_DATA_LENGTH);

    read(sockfd, buff, sizeof(buff));
    printf("From client: %s", buff);

    // printf("you better work\%s\n",strrev(buff));

    write(sockfd, strrev(buff), sizeof(strrev(buff)));

    if (strncmp("exit", buff, 4) == 0) {
      printf("Server Exit...\n");
      break;
    }
  }
}

int main() {
  int sockfd, connfd, len;
  struct sockaddr_in myserver, cli;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    printf("socket couldnt be created! byee\n");
    exit(0);
  }
  bzero(&myserver, sizeof(myserver));

  myserver.sin_family = AF_INET;
  myserver.sin_addr.s_addr = htonl(INADDR_ANY);
  myserver.sin_port = htons(PORT);

  if ((bind(sockfd, (SA *)&myserver, sizeof(myserver))) != 0) {
    printf("socket binding to port failed. check netstat\n");
    exit(0);
  }

  if ((listen(sockfd, 5)) != 0) {
    printf("Listen failed...\n");
    exit(0);
  } else
    printf("Server listening..\n");
  len = sizeof(cli);

  connfd = accept(sockfd, (SA *)&cli, &len);
  if (connfd < 0) {
    printf("server acccept failed...\n");
    exit(0);
  } else
    printf("server acccept the client...\n");

  func(connfd);

  close(sockfd);
}
