#ifndef __SERVER_COM_H__
#define __SERVER_COM_H__

#include "server.h"

void server_send_block(client_conn_info conn_info, void *content, size_t n);

#endif