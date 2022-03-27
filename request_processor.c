#include "request_processor.h"

void pwd(client_conn_info conn_info)
{
  char cwd[MAXBUF];
  if (getcwd(cwd, MAXBUF) != NULL)
  {
    server_send_block(conn_info, cwd, MAXBUF);
  }
}

void request_processor(client_conn_info conn_info)
{
  size_t n;
  char buf[MAXLINE];

  Rio_readinitb(&conn_info.rio, conn_info.fd);
  while ((n = Rio_readnb(&conn_info.rio, buf, MAXBUF)) != 0 && !conn_info.deconnect)
  {
    if (strcmp(buf, "pwd") == 0)
    {
      pwd(conn_info);
    }
    else
    {
      printf("%s> %s\n", conn_info.ip_string, buf);
      if (strcmp(buf, "ping") == 0)
      {
        server_send_block(conn_info, "pong\n", MAXBUF);
      }
      else if (strcmp(buf, "quit") == 0)
      {
        conn_info.deconnect = 1;
      }
      else if (strstr(buf, "get") == buf)
      {
        get_request(conn_info, buf + 4);
      }
      else if (strstr(buf, "resume") == buf)
      {
        resume_request(conn_info, buf + 7);
      }
      else
      {
        server_send_block(conn_info, "Unknown command!\n", MAXBUF);
      }
    }
  }
}