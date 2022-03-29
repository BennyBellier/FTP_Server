#ifndef __FILE_H__
#define __FILE_H__

#include "ftp_protocole.h"

struct Queue_elem;

typedef struct Queue_elem *Queue;

struct Queue_elem
{
  int fd;
  rio_t rio;
  long nb_fils_libre;
  Queue next;
};

Queue creer_file_vide();

Queue inserer(Queue f, int fd, rio_t rio);

Queue extraire(Queue f, int *fd, rio_t *rio);

int est_vide_file(Queue f);

void detruire_file(Queue f);


#endif