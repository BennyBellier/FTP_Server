#include "request_processor.h"

void connection_processor_generator(int listenfd, client_conn_info conn_info, socklen_t clientlen, struct sockaddr_in clientaddr)
{
  while (1)
  {
    while ((conn_info.fd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) < 3);
    conn_info.deconnect = 0;
    Getnameinfo((SA *)&clientaddr, clientlen, conn_info.hostname, MAX_NAME_LEN, 0, 0, 0);

    Inet_ntop(AF_INET, &clientaddr.sin_addr, conn_info.ip_string, INET_ADDRSTRLEN);

    printf("Client %s (%s) connected in PID : %d\n", conn_info.hostname, conn_info.ip_string, getpid());

    request_processor(conn_info);
    printf("Client %s (%s) disconnected from this server\n", conn_info.hostname, conn_info.ip_string);
    Close(conn_info.fd);
  }
}

void client_connection_manager(socklen_t *clientlen, struct sockaddr_in *clientaddr, int *listenfd, int port, client_conn_info conn_info)
{
  *clientlen = (socklen_t)sizeof(clientaddr);
  *listenfd = Open_listenfd(port);

  for (int i = 0; i < NB_PROC; i++)
  {
    if ((Fork()) == 0)
    {
      // génération des traitans de connexions
      connection_processor_generator(*listenfd, conn_info, *clientlen, *clientaddr);
    }
  }
}

void load_balancer_connection(char *load_balancer_host, master_conn *master)
{
  master->fd = Open_clientfd(load_balancer_host, FTP_PORT);
  master->disconnect = 0;
  Rio_readinitb(&master->rio, master->fd);
}

void quit()
{
  kill(-getpid(), SIGINT);
}

int main(int argc, char **argv)
{
  int listenfd;
  master_conn master;
  socklen_t clientlen;
  struct sockaddr_in clientaddr;
  client_conn_info conn_info;

  if (argc < 3)
  {
    printf("usage: %s <load balancer host> <port>\n", argv[0]);
    exit(0);
  }

  chdir(SERV_FOLDER);

  load_balancer_connection(argv[1], &master);


  ftp_com msg;
  if (rio_readnb(&master.rio, &msg, sizeof(ftp_com)) && msg.type == MASTER_CONNECTED)
  {
    msg.type = PORT;
    msg.value = atoi(argv[2]);
    rio_writen(master.fd, &msg, sizeof(ftp_com));

    client_connection_manager(&clientlen, &clientaddr, &listenfd, atoi(argv[2]), conn_info);

    Close(listenfd);

    printf("\033[1;32mServer ready on port: %d\033[0;37m\n", atoi(argv[2]));

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
  kill(-getpid(), SIGINT);
}
