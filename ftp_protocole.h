#ifndef __FTP_PROTOCOLE_H__
#define __FTP_PROTOCOLE_H__

#include "csapp.h"
#include <sys/stat.h>
#include <string.h>

#define RWX_UGO (S_IRWXU | S_IRWXG | S_IRWXO)
#define FTP_PORT 88888

typedef struct ftp_file_descriptor
{
  long int size;
  char name[512];
  mode_t perm;
  int error;
} ftp_file_descriptor;

typedef enum ftp_request
{
  GET,
  RESUME,
  NO_REQUEST
} ftp_request;

typedef struct ftp_file_transfert
{
  int block_num;
  int bl_size;
  char buf[MAXBUF];
} ftp_file_transfert;

#endif