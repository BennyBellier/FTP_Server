#include "server_com.h"

void server_send_block(client_conn_info conn_info, void *content, size_t n)
{
  if (rio_writen(conn_info.fd, content, n) != (ssize_t) n)
  {
    // si l'envoie c'est mal efffectué, c'est que le client s'est déconnecté donc on reset le processus pour un nouveau client
    printf("Connection lost with client %s (%s)\n", conn_info.hostname, conn_info.ip_string);
    conn_info.deconnect = 1;
  }
}