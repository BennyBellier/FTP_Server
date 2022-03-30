#include "queue.h"

Queue creer_file_vide()
{
  return NULL;
}

Queue inserer(Queue f, char *host, int port)
{
  Queue new, current, previous;
  new = (Queue)calloc(1, sizeof(struct Queue_elem));
  strcpy(new->hostname, host);
  new->port = port;
  new->next =NULL;

  if ((f == NULL))
  {
    new->next = f;
    f = new;
  }
  else
  {
    previous = f;
    current = f->next;
    while (current != NULL)
    {
      previous = current;
      current = current->next;
    }
    previous->next = new;

  }
  return f;
}

Queue extraire(Queue f, char *host, int *port)
{
  Queue current;
  if (f != NULL)
  {
    current = f;
    strcpy(host, current->hostname);
    *port = current->port;
    f = current->next;
    free(current);
  }
  return f;
}

int nb_elem(Queue f)
{
  int nb = 0;
  Queue current;

  if (f != NULL)
  {
    current = f;
    while (current != NULL)
    {
      ++nb;
      current = current->next;
    }

  }

  return nb;
}

    int est_vide_file(Queue f)
{
  return f == NULL;
}

void detruire_file(Queue f)
{
  char host[MAX_NAME_LEN];
  int port;
  while (!est_vide_file(f))
    f = extraire(f, host, &port);
}
