#include "queue.h"

// Création d'une nouvelle file vide
Queue creer_file_vide()
{
  return NULL;
}

// insertion d'une nouvelle élemnt dans la file
Queue inserer(Queue f, char *host, int port)
{
  Queue new, current, previous;
  new = (Queue)calloc(1, sizeof(struct Queue_elem));
  strcpy(new->hostname, host);
  new->port = port;
  new->next = NULL;

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

// extraction d'un élément de la file
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

// Retourne un booléen Vrai si la file est vide
int est_vide_file(Queue f)
{
  return f == NULL;
}

// suppression complête de la file
void detruire_file(Queue f)
{
  char host[MAX_NAME_LEN];
  int port;
  while (!est_vide_file(f))
    f = extraire(f, host, &port);
}
