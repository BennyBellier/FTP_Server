#ifndef __SERVER_FILE_PROCESSOR_H__
#define __SERVER_FILE_PROCESSOR_H__

#include "server.h"
#include "server_com.h"

void get_request(client_conn_info conn_info, char *filename);

void resume_request(client_conn_info conn_info, char *filename);

#endif