#include "request_processor.h"

// Génération d'une pool de processus pour gérer les connexions clients
void connection_processor_generator(int listenfd, client_conn_info conn_info, socklen_t clientlen, struct sockaddr_in clientaddr)
{
  while (1)
  {
    // Boucle d'attente d'une connection entrante
    while ((conn_info.fd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) == 0);
    conn_info.deconnect = 0;
    Getnameinfo((SA *)&clientaddr, clientlen, conn_info.hostname, MAX_NAME_LEN, 0, 0, 0);
    Inet_ntop(AF_INET, &clientaddr.sin_addr, conn_info.ip_string, INET_ADDRSTRLEN);

    // on affiche un message côté serveur avec les informations du client connecté
    printf("Client %s (%s) connected in PID ; %d\n", conn_info.hostname, conn_info.ip_string, getpid());

    // Gestion des commandes que peut faire le client
    request_processor(conn_info);

    // On affiche un message sur le terminal du server lorsque le client est déconnecté
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
  // arrêt de tous les processus du même groupe, gestion des zombies
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
    printf("usage: %s <host> <port>\n", argv[0]);
    exit(0);
  }

  chdir(SERV_FOLDER);
  // Connexion au repartiteur de charge
  load_balancer_connection(argv[1], &master);

  ftp_com msg;
  // Si connecté au repartiteur de charge
  if (rio_readnb(&master.rio, &msg, sizeof(ftp_com)) && msg.type == MASTER_CONNECTED)
  {
    // envoie du port du serveur
    msg.type = PORT;
    msg.value = atoi(argv[2]);
    rio_writen(master.fd, &msg, sizeof(ftp_com));

    // génération des processus pour traitement des connexions clients
    client_connection_manager(&clientlen, &clientaddr, &listenfd, atoi(argv[2]), conn_info);

    // fermeture des de l'écoute pour le père
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
