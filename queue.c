#include "queue.h"

Queue creer_file_vide()
{
  return NULL;
}

Queue inserer(Queue f, int fd, rio_t rio)
{
  Queue new, current, previous;
  new = (Queue)calloc(1, sizeof(struct Queue_elem));
  new->fd = fd;
  new->rio = rio;
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

Queue extraire(Queue f, int *fd, rio_t *rio)
{
  Queue current;
  if (f != NULL)
  {
    current = f;
    *fd = f->fd;
    *rio = f->rio;
    f = current->next;
    free(current);
  }
  return f;
}

int est_vide_file(Queue f)
{
  return f == NULL;
}

void detruire_file(Queue f)
{
  int fd;
  rio_t rio;

  while (!est_vide_file(f))
    f = extraire(f, &fd, & rio);

}
