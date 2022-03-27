#include "client_com.h"

void client_send_block(int clientfd, void *buf, size_t n)
{
  if (rio_writen(clientfd, buf, n) < 0)
  {
    printf("Connection lost with server\n");
  }
}

void client_receive_block(int clientfd, void *buf, size_t n);