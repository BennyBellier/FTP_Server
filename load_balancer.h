#ifndef __LOAD_BALANCER_H__
#define __LOAD_BALANCER_H__

#include "ftp_protocole.h"
#include "queue.h"

typedef struct server_slave
{
  int fd;
  rio_t rio;

} server_slave;

#endif