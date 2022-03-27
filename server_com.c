#include "server_com.h"

void server_send_block(client_conn_info conn_info, void *content, size_t n)
{
  if (rio_writen(conn_info.fd, content, n) < 0)
  {
    printf("Connection lost with client %s (%s)\n", conn_info.hostname, conn_info.ip_string);
    conn_info.deconnect = 1;
  }
}
