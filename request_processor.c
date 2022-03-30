#include "request_processor.h"

// fonction renvoyant le pwd du serveur au client
void pwd(client_conn_info conn_info)
{
  ftp_com cwd;
  if (getcwd(cwd.content, sizeof(ftp_com)) != NULL)
  {
    cwd.value = strlen(cwd.content);
    cwd.type = PWD;
    server_send_block(conn_info, &cwd, sizeof(ftp_com));
  }
}

// fonction renvoyant 'pong' au client
void pong(client_conn_info conn_info)
{
  ftp_com pong;
  strcpy(pong.content, "pong\n");
  pong.value = strlen(pong.content);
  pong.type = PING;
  server_send_block(conn_info, &pong, sizeof(ftp_com));
}

// gestionnaire des requêtes client
void request_processor(client_conn_info conn_info)
{
  ftp_com msg;

  Rio_readinitb(&conn_info.rio, conn_info.fd);
  while (!conn_info.deconnect)
  {
    // récupération de la requête
    rio_readnb(&conn_info.rio, &msg, sizeof(ftp_com));
    switch (msg.type)
    {
    // requête pour télécharger un fichier
    case GET:
      printf("%s> get '%s'\n", conn_info.ip_string, msg.content);
      get_request(conn_info, msg.content);
      break;

    // Requête pour redemarrer un téléchargement interrompue
    case RESUME:
      printf("%s> resume '%s'\n", conn_info.ip_string, msg.content);
      resume_request(conn_info, msg.content);
      break;

    // Demande du PWD du serveur
    case PWD:
      pwd(conn_info);
      break;

    // Déconnexion du client
    case QUIT:
      printf("%s> quit\n", conn_info.ip_string);
      conn_info.deconnect = 1;
      break;

    // Requête ping, le serveur répond automatiquement pong
    case PING:
      printf("%s> ping\n", conn_info.ip_string);
      pong(conn_info);
      break;

    // Requête inconnue
    default:
      memset(&msg, 0, sizeof(ftp_com));
      msg.type = MSG;
      strcpy(msg.content, "Unknown command!\n");
      server_send_block(conn_info, &msg, sizeof(ftp_com));
      break;
    }
  }
}