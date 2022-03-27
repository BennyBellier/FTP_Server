#include "ftp_protocole_client.h"

void get_file(rio_t rio)
{
  char buf[MAXBUF];
  char *content;
  ftp_file_protocole f_desc;
  ftp_status status;
  struct stat st;
  time_t start, end;

  Rio_readnb(&rio, &f_desc, sizeof(ftp_file_protocole));
  if (f_desc.error == 550)
  {
    Fputs("FTP 550: File not found\n", stdout);
    return;
  }
  content = calloc(f_desc.size, sizeof(char));

  start = time(NULL);
  if (f_desc.size != Rio_readnb(&rio, content, f_desc.size))
  {
    Fputs("FTP 426: transfer aborted\n", stdout);
    Rio_readnb(&rio, buf, MAXBUF);
    return;
  }
  end = time(NULL);


  FILE *f= Fopen(f_desc.name, "wb");
  Fwrite(content, sizeof(char), f_desc.size, f);
  Fclose(f);

  stat(f_desc.name, &st);
  unsigned long delta = (unsigned long) difftime(end, start);
  double speed = f_desc.size;
  if (delta > 0)
    speed = speed / delta;
  speed = speed / 1000;

  Rio_readnb(&rio, &status, sizeof(status));
  if (status.code == 226)
  {
    printf("Download successfull\n%ld bytes received in %ld seconds (%f Kbytes/s).\n", f_desc.size, delta, speed);
  }
}