#include "client_com.h"

void client_send_block(serv_conn_info serv_info, void *buf, size_t n)
{
  if (rio_writen(serv_info.fd, buf, n) < 0)
  {
    // si l'envoie c'est mal efffectué, c'est que le serveur s'est déconnecté donc on arrête le client
    printf("Connection lost with server\n");
    exit(0);
  }
}