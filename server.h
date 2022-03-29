#ifndef __SERVER_H__
#define __SERVER_H__

#include <unistd.h>
#include <math.h>
#include "ftp_protocole.h"

#define MAX_NAME_LEN 256
#define NB_PROC 1
#define SERV_FOLDER "./"

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
  socklen_t socklen;
  struct sockaddr_in sockaddr;
  char ip_string[INET_ADDRSTRLEN];
  char hostname[MAX_NAME_LEN];
  int disconnect;
} master_conn;

#endif