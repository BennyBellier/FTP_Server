#include "server_file_processor.h"

int send_file_desc(client_conn_info conn_info, char *buf, long *file_size)
{
  struct stat st;
  int fd = 0;
  ftp_file_descriptor f_desc;

  fd = open(buf, O_RDONLY);

  if (fd > 0)
  {
    strcpy(f_desc.name, buf);
    fstat(fd, &st);
    f_desc.size = st.st_size;
    *file_size = st.st_size;
    f_desc.perm = st.st_mode & RWX_UGO;
    f_desc.error = 250;
  }
  else
  {
    f_desc.error = 550;
    printf("file doesn't exist\n");
  }

  server_send_block(conn_info, &f_desc, sizeof(ftp_file_descriptor));
  return fd;
}

void get_request(client_conn_info conn_info, char *filename)
{
  int fd;
  int nb_block;
  ftp_file_transfert block;
  long fz;

  fd = send_file_desc(conn_info, filename, &fz);

  // Envoie du fichiers entiers
  if (fd < 3)
  {
    printf("Bad file descriptor %d\n", fd);
    return;
  }
  nb_block = (fz / MAXBUF) + 1;

  for (int i = 0; i < nb_block; i++)
  {
    block.bl_size = Read(fd, block.buf, MAXBUF);
    block.block_num = i;
    server_send_block(conn_info, &block, sizeof(ftp_file_transfert));
  }
  Close(fd);
}

void resume_request(client_conn_info conn_info, char *filename)
{
  int fd, nb_block;
  ftp_file_transfert block;
  struct stat st;
  long block_safe = 0;

  printf("Start resume of file '%s'\n", filename);

  fd = open(filename, O_RDONLY);
  if (fd < 3)
  {
    printf("File doesn't exist!\n");
    return;
  }
  fstat(fd, &st);
  nb_block = (st.st_size / MAXBUF) + 1;

  Rio_readnb(&conn_info.rio, &block_safe, sizeof(long));
  printf("number: %ld\n", block_safe);
  lseek(fd, (block_safe * MAXBUF), SEEK_SET);

  for (int i = block_safe; i < nb_block; i++)
  {
    block.bl_size = Read(fd, block.buf, MAXBUF);
    block.block_num = i;
    server_send_block(conn_info, &block, sizeof(ftp_file_transfert));
  }
  printf("End of sending file\n");
  Close(fd);
}
