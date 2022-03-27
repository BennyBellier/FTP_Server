#include "client.h"
#include "client_file_processor.h"

void quit(int clientfd)
{
  printf("Your are disconnected\nGoodbye\n");

  Close(clientfd);
  exit(0);
}

void server_pwd(serv_conn_info serv_info, char *server_id_string)
{
  char buf[MAXBUF];
  client_send_block(serv_info.fd, "pwd", MAXBUF);
  if (Rio_readnb(&serv_info.rio, buf, MAXBUF) != 0)
    printf("\033[1;31m%s\033[0;37m:\033[1;36m%s\033[0;37m$ ", server_id_string, buf);
}

void client_request(serv_conn_info serv_info, char *host)
{
  char buf[MAXBUF];
  while (Fgets(buf, MAXBUF, stdin) != NULL)
  {
    buf[strcspn(buf, "\n")] = 0;
    if (strlen(buf) == 0)
    {
      server_pwd(serv_info, host);
    }
    else
    {
      Rio_writen(serv_info.fd, buf, MAXBUF);
      if (strcmp(buf, "bye") == 0)
      {
        quit(serv_info.fd);
      }
      else if (strstr(buf, "get") == buf)
      {
        get_file(serv_info.rio);
      }
      else if (strstr(buf, "resume") == buf)
      {
        resume_get_file(serv_info, buf+7);
      }
      else
      {
        Rio_readnb(&serv_info.rio, buf, MAXBUF);
        Fputs(buf, stdout);
      }
      server_pwd(serv_info, host);
    }
  }
}

int main(int argc, char **argv)
{
    int port;
    char *host;
    serv_conn_info serv_info;

    if (argc < 2)
    {
      fprintf(stderr, "usage: %s <host>\n", argv[0]);
      exit(0);
    }
    else if (argc == 2)
    {
      port = FTP_PORT;
    }
    else if (argc == 3) {
        port = atoi(argv[2]);
    }

    chdir(CLIENT_FOLDER);
    host = argv[1];

    serv_info.fd = Open_clientfd(host, port);
    printf("client connected to FTP server\n");
    Rio_readinitb(&serv_info.rio, serv_info.fd);

    server_pwd(serv_info, host);
    client_request(serv_info, host);

  quit(serv_info.fd);
}