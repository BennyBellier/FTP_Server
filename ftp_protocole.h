#ifndef __FTP_PROTOCOLE_H__
#define __FTP_PROTOCOLE_H__

#include "csapp.h"
#include <sys/stat.h>
#include <string.h>

#define RWX_UGO (S_IRWXU | S_IRWXG | S_IRWXO)
#define MAX_NAME_LEN 256
#define MAX_BLOCK_SIZE (MAXBUF - (sizeof(long) + sizeof(ssize_t)))
#define MAX_MSG_LEN (MAXBUF - (sizeof(ftp_request) + sizeof(long)))

// Modifiable
#define FTP_PORT 2121
#define NB_PROC 5
#define NB_SLAVE 1
#define CLIENT_FOLDER "client_folder/"
#define SERV_FOLDER "server_folder/"

// structure décrivant le fichier
typedef struct ftp_file_descriptor
{
  long size;      // taille du fichier
  char name[512]; // nom du fichier
  mode_t perm;    // permission associé à celui-ci
  int error;      // code d'erreur 550 si le fichier n'existe pas sinon 250
} ftp_file_descriptor;

// Enumeration des requête
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

// structure contenant toutes les informations d'un serveur esclave
typedef struct slave_conn_info
{
  int fd;
  rio_t rio;
  char ip_string[INET_ADDRSTRLEN];
  char hostname[MAX_NAME_LEN];
  int deconnect;
} slave_conn_info;

#endif