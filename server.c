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

void quit()
{
  // arrêt de tous les processus du même groupe, gestion des zombies
  kill(-getpid(), SIGINT);
}

int main(int argc, char **argv)
{
  int listenfd, port;
  socklen_t clientlen;
  struct sockaddr_in clientaddr;
  client_conn_info conn_info;

  // Initialisation du port du serveur a FTP_PORT (2121) si l'utilisateur n'a pas précisé de port
  if (argc < 2)
  {
    fprintf(stderr, "usage: %s <host>\n", argv[0]);
    exit(0);
  }
  else if (argc == 2)
  {
    port = FTP_PORT;
  }
  else if (argc == 3)
  {
    port = atoi(argv[2]);
  }

  chdir(SERV_FOLDER);
  clientlen = (socklen_t)sizeof(clientaddr);
  listenfd = Open_listenfd(port);

  for (int i = 0; i < NB_PROC; i++)
  {
    if ((Fork()) == 0)
    {
      // génération des traitans de connexions client
      connection_processor_generator(listenfd, conn_info, clientlen, clientaddr);
    }
  }

  // Père
  if (getgid() != 0) {
    // Le père ne gère pas les connexion avec les client on peut donc fermé l'écoute des connexion
    Close(listenfd);

    printf("\033[1;32mServer ready on port %d\033[0;37m\n", FTP_PORT);

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
