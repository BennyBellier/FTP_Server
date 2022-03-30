#include "client.h"
#include "client_file_processor.h"

// Variable global pour pouvoir stoper la connexion correctement si le client effectue un Ctrl + C
volatile serv_conn_info *serv_info_handler;

// Envoie de la requête de deconnexion au serveur et fermeture du client
void quit(serv_conn_info serv_info)
{
  ftp_com quit;
  quit.type = QUIT;
  client_send_block(serv_info, &quit, sizeof(ftp_com));

  Close(serv_info.fd);
  printf("Your are disconnected\nGoodbye\n");
  exit(0);
}

// Demande au serveur de lui renvoyer son dossier courant
void server_pwd(serv_conn_info serv_info, char *server_id_string)
{
  ftp_com pwd;
  pwd.type = PWD;
  client_send_block(serv_info, &pwd, sizeof(ftp_com));

  if (rio_readnb(&serv_info.rio, &pwd, sizeof(ftp_com)) == sizeof(ftp_com) && pwd.type == PWD)
    printf("\033[1;31m%s\033[0;37m:\033[1;36m%s\033[0;37m$ ", server_id_string, pwd.content);
}

// gestionnaire des requêtes client
void client_request(serv_conn_info serv_info, char *host)
{
  ftp_com msg;
  memset(&msg, 0, sizeof(ftp_com));
  while (!serv_info.disconnected)
  {
    if (Fgets(msg.content, sizeof(msg.content), stdin) == NULL) // Récupération de l'entrée de l'utilisateur
    {
      printf("\n");
      break;
    }
    // Suppression du retour chariot '\n' à la fin de la saisie de l'tilisateur
    msg.content[strcspn(msg.content, "\n")] = 0;
    if (strlen(msg.content) == 0)
    {
      // reset affichage
      server_pwd(serv_info, host);
    }
    else
    {
      // Détections et executions des entrées de l'utilisateur
      if (strcmp(msg.content, "bye") == 0)
      {
        quit(serv_info);
      }
      else if (strstr(msg.content, "get") == msg.content)
      {
        // utilisation de 'msg.content + 4' pour garder seulement le nom de fichier
        get_file(serv_info, msg.content + 4);
      }
      else if (strstr(msg.content, "resume") == msg.content)
      {
        // utilisation de 'msg.content + 7' pour garder seulement le nom de fichier
        resume_get_file(serv_info, msg.content + 7);
      }
      else
      {
        if (strcmp(msg.content, "ping") == 0)
        {
          memset(&msg, 0, sizeof(ftp_com));
          msg.type = PING;
          client_send_block(serv_info, &msg, sizeof(ftp_com));
        }
        else
        {
          // l'utilisateur envoie une commande inconnue côté client
          msg.type = MSG;
          msg.value = strlen(msg.content);
          client_send_block(serv_info, &msg, sizeof(ftp_com));
        }

        memset(&msg, 0, sizeof(ftp_com));
        rio_readnb(&serv_info.rio, &msg, sizeof(ftp_com));
        Fputs(msg.content, stdout);
      }
      server_pwd(serv_info, host);
    }
    memset(&msg, 0, sizeof(ftp_com));
  }
}

// Handler pour Ctrl + C
void sig_int_handler(int sig)
{
  printf("\n");
  quit(*serv_info_handler);
}

int main(int argc, char **argv)
{
  int port;
  char *host;
  serv_conn_info serv_info;
  serv_info_handler = &serv_info;

  Signal(SIGINT, sig_int_handler);

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

  chdir(CLIENT_FOLDER); // changement du dossier de travail pour le client
  host = argv[1];

  serv_info.fd = Open_clientfd(host, port);
  serv_info.disconnected = 0;
  printf("client connected to FTP server\n");
  Rio_readinitb(&serv_info.rio, serv_info.fd);

  server_pwd(serv_info, host);
  client_request(serv_info, host);

  quit(serv_info);
}