#ifndef __CLIENT_COM_H__
#define __CLIENT_COM_H__

#include "client.h"

void client_send_block(serv_conn_info serv_info, void *buf, size_t n);

void client_receive_block(serv_conn_info serv_info, void *buf, size_t n);
#endif