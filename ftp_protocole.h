#ifndef __FTP_PROTOCOLE_H__
#define __FTP_PROTOCOLE_H__

#include "csapp.h"
#include <sys/stat.h>
#include <string.h>

#define RWX_UGO (S_IRWXU | S_IRWXG | S_IRWXO)
#define FTP_PORT 2121
#define MAX_NAME_LEN 256
#define NB_PROC 5
#define SERV_FOLDER "./"
#define MAX_BLOCK_SIZE (MAXBUF - (sizeof(long) + sizeof(ssize_t)))
#define MAX_MSG_LEN (MAXBUF - (sizeof(ftp_request) + sizeof(long)))

// structure décrivant le fichier
typedef struct ftp_file_descriptor
{
  long size;      // taille du fichier
  char name[512]; // nom du fichier
  mode_t perm;    // permission associé à celui-ci
  int error;      // code d'erreur 550 si le fichier n'existe pas sinon 250
} ftp_file_descriptor;

// Enumeration des requête (non utilisé dans cette partie)
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

// structure pour tranfert de fichier lourd
typedef struct ftp_file_transfert
{
  long block_num;
  ssize_t bl_size;
  char buf[MAX_BLOCK_SIZE];
} ftp_file_transfert;

// structure pour transfert de fichier simple
typedef struct ftp_com
{
  ftp_request type;
  long value; // utilisé pour transferé la taille du message ou une valeur
  char content[MAX_MSG_LEN];
} ftp_com;

#endif