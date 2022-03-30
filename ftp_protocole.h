#ifndef __FTP_PROTOCOLE_H__
#define __FTP_PROTOCOLE_H__

#include "csapp.h"
#include <sys/stat.h>
#include <string.h>

#define RWX_UGO (S_IRWXU | S_IRWXG | S_IRWXO)
#define FTP_PORT 2121
#define NB_SLAVE 1
#define NB_PROC 5
#define MAX_NAME_LEN 256
#define BLOCK_SIZE (MAXBUF - (sizeof(long) + sizeof(ssize_t)))
#define MSG_SIZE (MAXBUF - (sizeof(ftp_request) + sizeof(long)))
#define SERV_FOLDER "./"

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
  SHUTDOWN,
  MASTER_CONNECTED,
  PORT,
  SLAVE_CONNECTION_INFO,
} ftp_request;

typedef struct ftp_file_transfert
{
  long block_num;
  ssize_t bl_size;
  char buf[BLOCK_SIZE];
} ftp_file_transfert;

typedef struct ftp_com
{
  ftp_request type;
  long value; // used as length of content tab, or as integer value
  char content[MSG_SIZE];
} ftp_com;

typedef struct slave_conn_info
{
  int fd;
  rio_t rio;
  char ip_string[INET_ADDRSTRLEN];
  char hostname[MAX_NAME_LEN];
  int deconnect;
} slave_conn_info;

#endif