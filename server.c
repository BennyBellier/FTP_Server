#include "request_processor.h"

void connection_processor_generator(int listenfd, client_conn_info conn_info, socklen_t clientlen, struct sockaddr_in clientaddr)
{
  while (1)
  {
    while ((conn_info.fd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) == 0)
    {
    }
    conn_info.deconnect = 0;
    Getnameinfo((SA *)&clientaddr, clientlen, conn_info.hostname, MAX_NAME_LEN, 0, 0, 0);

    Inet_ntop(AF_INET, &clientaddr.sin_addr, conn_info.ip_string, INET_ADDRSTRLEN);

    printf("Client %s (%s) connected in PID ; %d\n", conn_info.hostname, conn_info.ip_string, getpid());

    request_processor(conn_info);
    printf("Client %s (%s) disconnected from this server\n", conn_info.hostname, conn_info.ip_string);
    Close(conn_info.fd);
  }
}

void quit()
{
  kill(-getpid(), SIGINT);
}

int main(int argc, char **argv)
{
  int listenfd;
  socklen_t clientlen;
  struct sockaddr_in clientaddr;
  client_conn_info conn_info;

  chdir(SERV_FOLDER);
  clientlen = (socklen_t)sizeof(clientaddr);
  listenfd = Open_listenfd(FTP_PORT);

  for (int i = 0; i < NB_PROC; i++)
  {
    if ((Fork()) == 0)
    {
      // génération des traitans de connexions
      connection_processor_generator(listenfd, conn_info, clientlen, clientaddr);
    }
  }

  if (getgid() != 0) {
    Close(listenfd);

    printf("\033[1;32mServer ready\033[0;37m\n");

    while (1)
    {
      char result;
      scanf("%s", &result);
      switch (result)
      {
      case 'q':
        quit();
        break;

      default:
        printf("Unknow command\n");
        break;
      }
    }
  }
  exit(0);
}
