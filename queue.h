#ifndef __FILE_H__
#define __FILE_H__

#include "ftp_protocole.h"

struct Queue_elem;

typedef struct Queue_elem *Queue;

struct Queue_elem
{
  char hostname[MAX_NAME_LEN];
  int port;
  long nb_fils_libre;
  Queue next;
};

Queue creer_file_vide();

Queue inserer(Queue f, char *host, int port);

Queue extraire(Queue f, char *host, int *port);

int nb_elem(Queue f);

int est_vide_file(Queue f);

void detruire_file(Queue f);

#endif