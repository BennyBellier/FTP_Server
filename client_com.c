#include "client_com.h"

void client_send_block(serv_conn_info serv_info, void *buf, size_t n)
{
  if (rio_writen(serv_info.fd, buf, n) < 0)
  {
    printf("Connection lost with server\n");
    exit(0);
  }
}