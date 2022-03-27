#ifndef __CLIENT_COM_H__
#define __CLIENT_COM_H__

#include "client.h"

void client_send_block(int clientfd, void *buf, size_t n);

#endif