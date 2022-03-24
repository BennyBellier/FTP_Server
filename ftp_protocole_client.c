#include "ftp_protocole_client.h"

void file_name_check(char *fname)
{
  char *p = strtok(fname, "/");
  while (p != NULL)
  {
    strcpy(fname, p);
    p = strtok(NULL, "/");
  }
}

void get_file(rio_t rio)
{
  ftp_file_protocole f_desc;
  ftp_status status;
  struct stat st;
  time_t start, end;
  ftp_file_transfert block;
  int nb_block;
  int nb_block_cur = 0;

  Rio_readnb(&rio, &f_desc, sizeof(ftp_file_protocole));
  if (f_desc.error == 550)
  {
    Fputs("FTP 550: File not found\n", stdout);
    return;
  }
  file_name_check(f_desc.name);
  printf("Start downloading\n");
  int fd = Open(f_desc.name, O_WRONLY | O_CREAT, f_desc.perm);
  nb_block = (f_desc.size / MAXBUF) + 1;

  start = time(NULL);
  if (nb_block > 0)
  {
    do
    {
      Rio_readnb(&rio, &block, sizeof(ftp_file_transfert));
      Write(fd, block.buf, block.bl_size);
      if (nb_block_cur == block.block_num)
        ++nb_block_cur;
      else
      {
        char buf[MAXBUF];
        Fputs("FTP 426: transfer aborted\n", stdout);
        Rio_readnb(&rio, buf, MAXBUF);
        return;
      }
    } while (block.block_num + 1 < nb_block);
  }
  end = time(NULL);
  Close(fd);
  stat(f_desc.name, &st);

  Rio_readnb(&rio, &status, sizeof(status));
  if (status.code == 226 && f_desc.size == st.st_size)
  {
    unsigned long delta = (unsigned long)difftime(end, start);
    double speed = st.st_size;
    if (delta > 0)
      speed = speed / delta;

    printf("Donwload successfull\n");

    if (st.st_size > 1000000000)
    {
      printf("%.2ld Gbytes", st.st_size / 1000000000);
    }
    else if (st.st_size > 10000000)
    {
      printf("%.2ld Mbytes", st.st_size / 1000000);
    }
    else if (st.st_size > 1000)
    {
      printf("%.2ld Kbytes", st.st_size / 1000);
    }
    else
      printf("%ld bytes", st.st_size);


    printf(" received in %ld seconds", delta);

    if (speed > 1000000000)
    {
      printf(" (%.2f Gbytes/s).\n", speed / 1000000000);
    }
    else if (speed > 10000000)
    {
      printf(" (%.2f Mbytes/s).\n", speed / 1000000);
    }
    else if (speed > 1000)
    {
      printf(" (%.2f Kbytes/s).\n", speed / 1000);
    }
    else
      printf(" (%.2f bytes/s).\n", speed);
  }
  else if(status.code == 226 && f_desc.size != st.st_size)
  {
    printf("Download failed\n%ld%% of the file been download!\n", st.st_size / f_desc.size);
  }
}