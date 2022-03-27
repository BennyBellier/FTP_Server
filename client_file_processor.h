#ifndef __CLIENT_FILE_PROCESSOR_H__
#define __CLIENT_FILE_PROCESSOR_H__

#include <time.h>
#include "client.h"
#include "client_com.h"

void get_file(rio_t rio);

void resume_get_file(serv_conn_info serv_info, char *filename);

#endif