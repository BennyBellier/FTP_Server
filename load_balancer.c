#include "load_balancer.h"

void quit()
{
  // arrêt de tous les processus du même groupe, gestion des zombies
  kill(-getpid(), SIGINT);
}

void slave_process_generator(int listenfd, slave_conn_info slave, socklen_t slavelen, struct sockaddr_in slaveaddr, Queue slave_queue)
{
  // on accept la connexion
  slave.fd = Accept(listenfd, (SA *)&slaveaddr, &slavelen);

  // Récupération et affichage des info basic sur l'escalve
  Getnameinfo((SA *)&slaveaddr, slavelen, slave.hostname, MAX_NAME_LEN, 0, 0, 0);
  Inet_ntop(AF_INET, &slaveaddr.sin_addr, slave.ip_string, INET_ADDRSTRLEN);
  printf("Slave %s (%s) connected on PID : %d\n", slave.hostname, slave.ip_string, getpid());

  Rio_readinitb(&slave.rio, slave.fd);
  // Reponse comme quoi l'esclave est bien connecté au master
  ftp_com msg;
  msg.type = MASTER_CONNECTED;
  rio_writen(slave.fd, &msg, sizeof(ftp_com));

  // Récupération du port de l'esclave
  rio_readnb(&slave.rio, &msg, sizeof(ftp_com));

  // Ajout dans la queue, les informations de connexion à l'escalve
  slave_queue = inserer(slave_queue, slave.hostname, msg.value);
  int print = 0;

  // boucle pour bloquer le processus et débogguer
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
  // génération de processus pour communiquer avec les n esclaves
  for (int i = 0; i < NB_SLAVE; i++)
  {
    if (Fork() == 0)
    {
      // génération d'un processus pour un serveur escalves
      slave_process_generator(listenfd, slave, *slavelen, *slaveaddr, slave_queue);
    }
  }
}

void client_connection_manager(int listenfd, socklen_t *clientlen, struct sockaddr_in *clientaddr, Queue slave_queue)
{
  // Génération d'un seul processus pour rediriger les clients vers les serveur esclaves
  if (Fork() == 0)
  {
    int fd;
    ftp_com msg;
    int port;
    while (1)
    {
      // attente d'une connexion client
      while ((fd = Accept(listenfd, (SA *)clientaddr, clientlen)) == 0);

      // on extrait les informations de connexion au serveur esclave, pour le client
      slave_queue = extraire(slave_queue, msg.content, &port);

      // envoie des informations au client
      msg.value = port;
      msg.type = SLAVE_CONNECTION_INFO;
      rio_writen(fd, &msg, sizeof(ftp_com));

      // on réinsert à la fin de la queue les infromations du serveur
      slave_queue = inserer(slave_queue, msg.content, port);
    }
    exit(0);
  }
}

int main(int argc, char **argv)
{
  int listenfd;
  socklen_t slavelen;
  struct sockaddr_in slaveaddr;
  slave_conn_info slave;
  // Création d'une queue pour  stocker les escalves et gerer les connexion en Round-Robin
  Queue slave_queue = creer_file_vide();

  slavelen = (socklen_t)sizeof(slaveaddr);
  listenfd = Open_listenfd(FTP_PORT);

  // Gestion des esclaves
  slave_connection_generator(&slavelen, &slaveaddr, listenfd, slave, slave_queue);

  socklen_t clientlen;
  struct sockaddr_in clientaddr;

  // Gestionnaire de connexion client
  client_connection_manager(listenfd, &clientlen, &clientaddr, slave_queue);

  if (getgid() != 0)
  {
    // fermeture des de l'écoute pour le père
    Close(listenfd);

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
