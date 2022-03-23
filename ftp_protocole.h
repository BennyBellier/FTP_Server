#ifndef __FTP_PROTOCOLE_H__
#define __FTP_PROTOCOLE_H__

#include "csapp.h"
#include <sys/stat.h>
#include <string.h>

#define PORT 88888
#define MAX

typedef struct ftp_file_protocole
{
  long int size;
  char name[512];
  mode_t perm;
  int error;
} ftp_file_protocole;

typedef enum ftp_request
{
  GET,
  NO_REQUEST
} ftp_request;

typedef struct ftp_status
{
  int code;
} ftp_status;


void file_request(int connfd);

#endif