#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <unistd.h>
#include "ftp_protocole.h"


#define CLIENT_FOLDER "client_folder/"

typedef struct serv_conn_info
{
  int fd;
  rio_t rio;
  int disconnected;
} serv_conn_info;

#endif