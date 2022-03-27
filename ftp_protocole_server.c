#include "ftp_protocole_server.h"

int send_file_description(int connfd, char *buf, long *file_size)
{
  struct stat st;
  int fd = 0;
  ftp_file_protocole file_description;

  buf[strcspn(buf, "\n")] = 0;
  fd = open(buf, O_RDONLY);

  if (fd > 0)
  {
    strcpy(file_description.name, buf);
    fstat(fd, &st);
    file_description.size = st.st_size;
    *file_size = st.st_size;
    file_description.perm = st.st_mode;
    file_description.error = 250;
  }
  else
  {
    file_description.error = 550;
    printf("file doesn't exist\n");
  }

  Rio_writen(connfd, &file_description, sizeof(ftp_file_protocole));
  return fd;
}

void get_request(int connfd, char *filename)
{
  int fd;
  char buf[MAXBUF];
  ftp_status status;
  long fz;

  fd = send_file_description(connfd, filename, &fz);

  // Envoie du fichiers entiers
  if (fd < 3)
  {
    printf("Bad file descriptor %d\n", fd);
    return;
  }
  Read(fd, buf, fz);
  Rio_writen(connfd, buf, fz);
  Close(fd);

  // Envoie du code status de fin
  status.code = 226;
  Rio_writen(connfd, &status, MAXBUF);
}

void sendMessage(int connfd, char *message)
{
  Rio_writen(connfd, message, MAXBUF);
}

void pwd(int connfd)
{
  char cwd[MAXBUF];
  if (getcwd(cwd, MAXBUF) != NULL)
    Rio_writen(connfd, cwd, MAXBUF);
}

void request_handler(int connfd, char *ip)
{
  size_t n;
  char buf[MAXLINE];
  rio_t rio;
  int client_quit = 0;

  Rio_readinitb(&rio, connfd);
  while ((n = Rio_readnb(&rio, buf, MAXBUF)) != 0 && !client_quit)
  {
    if (strcmp(buf, "pwd") == 0)
    {
      pwd(connfd);
    }
    else
    {
      printf("%s> %s", ip, buf);
      if (strcmp(buf, "ping\n") == 0)
      {
        sendMessage(connfd, "pong\n");
      }
      else if (strcmp(buf, "quit\n") == 0)
      {
        client_quit = 1;
      }
      else if (strstr(buf, "get") == buf)
      {
        get_request(connfd, buf+4);
      }
      else
      {
        sendMessage(connfd, "Commande inconnue!\n");
      }
    }
  }
}