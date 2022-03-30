#include "client.h"
#include "client_file_processor.h"

volatile serv_conn_info *serv_info_handler;

void quit(serv_conn_info serv_info)
{
  ftp_com quit;
  quit.type = QUIT;
  client_send_block(serv_info, &quit, sizeof(ftp_com));

  Close(serv_info.fd);
  printf("Your are disconnected\nGoodbye\n");
  exit(0);
}

void server_pwd(serv_conn_info serv_info, char *server_id_string)
{
  ftp_com pwd;
  pwd.type = PWD;
  client_send_block(serv_info, &pwd, sizeof(ftp_com));

  if (rio_readnb(&serv_info.rio, &pwd, sizeof(ftp_com)) == sizeof(ftp_com) && pwd.type == PWD)
    printf("\033[1;31m%s\033[0;37m:\033[1;36m%s\033[0;37m$ ", server_id_string, pwd.content);
}

void client_request(serv_conn_info serv_info, char *host)
{
  ftp_com msg;
  memset(&msg, 0, sizeof(ftp_com));
  while (!serv_info.disconnected)
  {
    if (Fgets(msg.content, sizeof(msg.content), stdin) == NULL)
    {
      printf("\n");
      break;
    }
    msg.content[strcspn(msg.content, "\n")] = 0;
    if (strlen(msg.content) == 0)
    {
      server_pwd(serv_info, host);
    }
    else
    {
      if (strcmp(msg.content, "bye") == 0)
      {
        quit(serv_info);
      }
      else if (strstr(msg.content, "get") == msg.content)
      {
        get_file(serv_info, msg.content + 4);
      }
      else if (strstr(msg.content, "resume") == msg.content)
      {
        resume_get_file(serv_info, msg.content + 7);
      }
      else
      {
        if (strcmp(msg.content, "ping") == 0)
        {
          memset(&msg, 0, sizeof(ftp_com));
          msg.type = PING;
          client_send_block(serv_info, &msg, sizeof(ftp_com));
        }
        else
        {
          msg.type = MSG;
          msg.value = strlen(msg.content);
          client_send_block(serv_info, &msg, sizeof(ftp_com));
        }

        memset(&msg, 0, sizeof(ftp_com));
        rio_readnb(&serv_info.rio, &msg, sizeof(ftp_com));
        Fputs(msg.content, stdout);
      }
      server_pwd(serv_info, host);
    }
    memset(&msg, 0, sizeof(ftp_com));
  }
}

void sig_int_handler(int sig)
{
  printf("\n");
  quit(*serv_info_handler);
}

int main(int argc, char **argv)
{
  int port;
  char *host;
  serv_conn_info serv_info;
  serv_info_handler = &serv_info;

  Signal(SIGINT, sig_int_handler);

  if (argc < 2)
  {
    fprintf(stderr, "usage: %s <host>\n", argv[0]);
    exit(0);
  }
  else if (argc == 2)
  {
    port = FTP_PORT;
  }
  else if (argc == 3)
  {
    port = atoi(argv[2]);
  }

  chdir(CLIENT_FOLDER);
  host = argv[1];

  serv_info.fd = Open_clientfd(host, port);
  serv_info.disconnected = 0;
  Rio_readinitb(&serv_info.rio, serv_info.fd);

  ftp_com msg;

  rio_readnb(&serv_info.rio, &msg, sizeof(ftp_com));
  printf("%s:%ld\n", msg.content, msg.value);

  // server_pwd(serv_info, host);
  // client_request(serv_info, host);

  quit(serv_info);
}