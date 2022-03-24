#include "csapp.h"
#include "ftp_protocole_server.h"

#define MAX_NAME_LEN 256
#define NB_PROC 5
#define SERV_FOLDER "./"

int main(int argc, char **argv)
{
  int listenfd, connfd, pid;
  socklen_t clientlen;
  struct sockaddr_in clientaddr;
  char client_ip_string[INET_ADDRSTRLEN];
  char client_hostname[MAX_NAME_LEN];

  chdir(SERV_FOLDER);
  clientlen = (socklen_t)sizeof(clientaddr);
  listenfd = Open_listenfd(PORT);

  for (int i = 0; i < NB_PROC; i++)
  {
    if ((pid = Fork()) == 0)
    {
      while (1)
      {
        while ((connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) == 0) {}
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

        printf("Client %s (%s) connected in PID ; %d\n", client_hostname, client_ip_string, getpid());

        request_handler(connfd, client_ip_string);
        printf("Client %s (%s) disconnected from this server\n", client_hostname, client_ip_string);
        Close(connfd);
      }
    }
  }

  if (pid != 0) {
    Close(listenfd);

    printf("\033[1;32mServer ready to handle\033[0;37m\n");

    while (1)
    {
      char result;
      scanf("%s", &result);
      switch (result)
      {
      case 'q':
        killpg(getpgrp(), SIGKILL);
        break;

      default:
        printf("Unknow command\n");
        break;
      }
    }
  }
  exit(0);
}