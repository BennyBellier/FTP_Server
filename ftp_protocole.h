#ifndef __FTP_PROTOCOLE_H__
#define __FTP_PROTOCOLE_H__

#include "csapp.h"
#include <sys/stat.h>
#include <string.h>

#define RWX_UGO (S_IRWXU | S_IRWXG | S_IRWXO)
#define FTP_PORT 88888

typedef struct ftp_file_descriptor
{
  long size;
  char name[512];
  mode_t perm;
  int error;
} ftp_file_descriptor;

typedef enum ftp_request
{
  GET,
  RESUME,
  PWD,
  QUIT,
  END_OF_FILE,
  START_BLOCK,
  PING,
  MSG,
  FILE_EXIST,
  UNKNOWN_FILE,
} ftp_request;

typedef struct ftp_file_transfert
{
  long block_num;
  ssize_t bl_size;
  char buf[MAXBUF - (sizeof(long) + sizeof(ssize_t))];
} ftp_file_transfert;

typedef struct ftp_com
{
  ftp_request type;
  long value; // used as length of content tab, or as integer value
  char content[MAXBUF - (sizeof(ftp_request) + sizeof(long))];
} ftp_com;

#endif