#include "csapp.h"

void echo(int connfd, char *ip)
{
		size_t n;
		char buf[MAXLINE];
		rio_t rio;

		Rio_readinitb(&rio, connfd);
		while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
				// printf("server received %u bytes\n", (unsigned int)n);
				if (strcmp(buf, "ping\n") == 0)
					Rio_writen(connfd, "pong\n", 5);
				else
				{
					printf("%s> %s\n", ip, buf);
					strcpy(buf, "Requête traité\n");
					Rio_writen(connfd, buf, strlen(buf));
				}
		}
}

