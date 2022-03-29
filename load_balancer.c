#include "load_balancer.h"

void quit()
{
  kill(-getpid(), SIGINT);
}

int main(int argc, char **argv)
{
  Queue slave_queue;

  if (argc < 2)
  {
    printf("usage: %s <server_ip1> <server_port1> <server_ip1> <server_port2> ...\n", argv[0]);
    exit(0);
  }

  slave_queue = creer_file_vide();

  for (int i = 1; i < (argc - 1); i += 2)
  {
    int port = atoi(argv[i+1]);
    char *host = argv[i];
    int fd;
    rio_t rio;

    fd = Open_clientfd(host, port);
    printf("Load balancer connected to serverz %s on port %d\n", host, port);
    Rio_readinitb(&rio, fd);

    slave_queue = inserer(slave_queue, fd, rio);
  }

  // int listenfd;
  // struct sockaddr_in clientaddr;
  // socklen_t clientlen;

  // clientlen = (socklen_t)sizeof(clientaddr);
  // listenfd = Open_listenfd(FTP_PORT);

  if (getgid() != 0)
  {
    // Close(listenfd);

    printf("\033[1;33mLoad balancer ready\033[0;37m\n");

    while (1)
    {
      char result;
      scanf("%s", &result);
      switch (result)
      {
      case 'q':
        quit();
        break;

      default:
        printf("Unknow command\n");
        break;
      }
    }
  }
  exit(0);
}
