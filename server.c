// create an epoll server, and listen at port 8888, and it will get user input and return user's input in uppercase.
// client could connect to the server using the "nc hostname 8888"
// compile the code as: gcc server.c -o server.o
// run the code as: ./server.o
// note that: the code is only compiled on Linux which supports epoll since kernel 2.6.8
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#define MAXLINE 80
#define SERV_PORT 8888
#define OPEN_MAX 65525
int main(int argc, char *argv[]) {
  int i, j, maxi, listenfd, connfd, sockfd;
  int n;
  ssize_t nready, efd, res;
  char buf[MAXLINE], str[INET_ADDRSTRLEN];
  socklen_t clilen;
  int client[OPEN_MAX];
  struct sockaddr_in cliaddr, servaddr;
  struct epoll_event tep, ep[OPEN_MAX];

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);
  bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  listen(listenfd, 20);
  efd = epoll_create(OPEN_MAX);
  tep.events = EPOLLIN; 
  tep.data.fd = listenfd;

  res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);

  for (;;) {
    nready = epoll_wait(efd, ep, OPEN_MAX, -1);
    for (i = 0; i < nready; i++) {
      if (ep[i].data.fd == listenfd) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

        tep.events = EPOLLIN;
        tep.data.fd = connfd;
        res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
      } else {
        sockfd = ep[i].data.fd;
        n = read(sockfd, buf, MAXLINE);
        if (n == 0) {
          res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
          close(sockfd);
          printf("client[%d] closed connection\n", j);
        } else {
          for (j = 0; j < n; j++) {
            buf[j] = toupper(buf[j]);
          }
          write(sockfd, buf, n);
        }
      }
    }
  }
}
