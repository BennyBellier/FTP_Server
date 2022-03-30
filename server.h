#ifndef __SERVER_H__
#define __SERVER_H__

#include <unistd.h>
#include <math.h>
#include "ftp_protocole.h"

typedef struct client_conn_info
{
  int fd;
  rio_t rio;
  char ip_string[INET_ADDRSTRLEN];
  char hostname[MAX_NAME_LEN];
  int deconnect;
} client_conn_info;

typedef struct master_conn
{
  int fd;
  rio_t rio;
  int disconnect;
} master_conn;

#endif