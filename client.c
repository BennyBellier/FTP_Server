#include "csapp.h"
#include "ftp_protocole_client.h"
#include <unistd.h>

#define CLIENT_FOLDER "./client_folder"

void quit(int clientfd)
{
  printf("Your are disconnected\nGoodbye\n");

  Close(clientfd);
  exit(0);
}

void server_pwd(int clientfd, rio_t rio, char *server_id_string)
{
  char buf[MAXBUF];
  Rio_writen(clientfd, "pwd", MAXBUF);
  if (Rio_readnb(&rio, buf, MAXBUF) != 0)
    printf("\033[1;31m%s\033[0;37m:\033[1;36m%s\033[0;37m$ ", server_id_string, buf);
}

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXBUF];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    chdir(CLIENT_FOLDER);
    host = argv[1];
    port = atoi(argv[2]);

    clientfd = Open_clientfd(host, port);
    printf("client connected to FTP server\n");
    Rio_readinitb(&rio, clientfd);

    server_pwd(clientfd, rio, argv[1]);

    while (Fgets(buf, MAXBUF, stdin) != NULL)
    {
      Rio_writen(clientfd, buf, MAXBUF);
      if (strcmp(buf, "bye\n") == 0)
      {
        quit(clientfd);
      }
      else if (strstr(buf, "get") != NULL)
      {
        get_file(rio);
      }
      else
      {
        Rio_readnb(&rio, buf, MAXBUF);
        Fputs(buf, stdout);
      }

      server_pwd(clientfd, rio, argv[1]);
    }

  quit(clientfd);
}
