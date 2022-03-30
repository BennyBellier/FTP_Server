#include "load_balancer.h"

void quit()
{
  kill(-getpid(), SIGINT);
}

void slave_process_generator(int listenfd, slave_conn_info slave, socklen_t slavelen, struct sockaddr_in slaveaddr, Queue slave_queue)
{
  slave.fd = Accept(listenfd, (SA*)&slaveaddr, &slavelen);
  Getnameinfo((SA*)&slaveaddr, slavelen, slave.hostname, MAX_NAME_LEN, 0, 0, 0);
  Inet_ntop(AF_INET, &slaveaddr.sin_addr, slave.ip_string, INET_ADDRSTRLEN);
  printf("Slave %s (%s) connected on PID : %d\n", slave.hostname, slave.ip_string, getpid());

  Rio_readinitb(&slave.rio, slave.fd);

  // Reponse comme quoi l'esclave est bien connecté au master
  ftp_com msg;
  msg.type = MASTER_CONNECTED;
  rio_writen(slave.fd, &msg, sizeof(ftp_com));

  // Récupération du port de l'esclave
  rio_readnb(&slave.rio, &msg, sizeof(ftp_com));

  slave_queue = inserer(slave_queue, slave.hostname, msg.value);
  int print = 0;
  while (1)
  {
    if (!print)
    {
      printf("%d\n", nb_elem(slave_queue));
      print = 1;
    }
  }
}

void slave_connection_generator(socklen_t *slavelen, struct sockaddr_in *slaveaddr, int listenfd, slave_conn_info slave, Queue slave_queue)
{
  for (int i = 0; i < NB_SLAVE; i++)
  {
    if (Fork() == 0)
    {
      slave_process_generator(listenfd, slave, *slavelen, *slaveaddr, slave_queue);
    }
  }
}

void client_connection_manager(int listenfd, socklen_t *clientlen, struct sockaddr_in *clientaddr, Queue slave_queue)
{
  if (Fork() == 0)
  {
    int fd;
    ftp_com msg;
    int port;

    fd = Accept(listenfd, (SA *)clientaddr, clientlen);

    slave_queue = extraire(slave_queue, msg.content, &port);

    msg.value = port;
    msg.type = SLAVE_CONNECTION_INFO;
    rio_writen(fd, &msg, sizeof(ftp_com));

    exit(0);
  }
}

int main(int argc, char **argv)
{
  int listenfd;
  socklen_t slavelen;
  struct sockaddr_in slaveaddr;
  slave_conn_info slave;
  Queue slave_queue = creer_file_vide();
  

  slavelen = (socklen_t)sizeof(slaveaddr);
  listenfd = Open_listenfd(FTP_PORT);

  // Gestion des esclaves
  slave_connection_generator(&slavelen, &slaveaddr, listenfd, slave, slave_queue);

  socklen_t clientlen;
  struct sockaddr_in clientaddr;

  // Gestionnaire de connexion client
  client_connection_manager(listenfd, &clientlen, &clientaddr, slave_queue);

  Close(listenfd);

  // Load balancer father
  if (getgid() != 0)
  {
    printf("\033[1;33mLoad balancer ready on port %d\033[0;37m\n", FTP_PORT);

    while (1)
    {
      if (!est_vide_file(slave_queue))
      {
        printf("Queue as %d slave\n", nb_elem(slave_queue));
      }
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
