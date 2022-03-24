#ifndef __FTP_PROTOCOLE_SERVER_H__
#define __FTP_PROTOCOLE_SERVER_H__

#include "ftp_protocole.h"
#include <unistd.h>
#include <math.h>

void request_handler(int connfd, char *ip);


#endif