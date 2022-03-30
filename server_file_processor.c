#include "server_file_processor.h"

void recherche_fichier(char *filename)
{
  int fds[2];
  pipe(fds);

  if (Fork() == 0)
  {
    close(fds[0]);
    dup2(fds[1], 1);
    close(fds[1]);
    // execution de la commande find pour trouver le fichier dans les sous-repertoires du serveur
    execlp("find", "find", "./server_folder", "-name", filename, NULL);
    exit(0);
  }
  else
  {
    close(fds[1]);
    int status;
    char buf[512];
    while (waitpid(-1, &status, WNOHANG) != -1);

    read(fds[0], buf, sizeof(buf));
    buf[strcspn(buf, "\n")] = 0;
    strcpy(filename, buf);
  }

}

int send_file_desc(client_conn_info conn_info, char *filename, long *file_size)
{
  struct stat st;
  int fd = 0;
  ftp_file_descriptor f_desc;


  fd = open(filename, O_RDONLY);

  // recherche du fichier dans les sous-dossiers
  if (fd == -1)
  {
    recherche_fichier(filename);
    fd = open(filename, O_RDONLY);
  }

  if (fd > 3)
  {
    strcpy(f_desc.name, filename);
    fstat(fd, &st);
    f_desc.size = st.st_size;
    *file_size = st.st_size;
    f_desc.perm = st.st_mode & RWX_UGO;
    f_desc.error = 250;
  }
  else
  {
    f_desc.error = 550;
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
    printf("%s> Tried to access a non-existent file '%s'\n", conn_info.ip_string, filename);
    return;
  }
  nb_block = (fz / BLOCK_SIZE) + 1;

  memset(&block, 0, sizeof(ftp_file_transfert));

  for (long i = 0; i < nb_block; i++)
  {
    block.bl_size = Read(fd, block.buf, BLOCK_SIZE);
    block.block_num = i;
    if (conn_info.deconnect)
    {
      printf("break\n");
      break;
    }
    server_send_block(conn_info, &block, sizeof(ftp_file_transfert));
  }
  Close(fd);
}

void resume_request(client_conn_info conn_info, char *filename)
{
  int fd, nb_block;
  ftp_file_transfert block;
  struct stat st;
  ftp_com start_block;
  ftp_com file_state;

  fd = open(filename, O_RDONLY);
  // recherche du fichier dans les sous-dossiers
  if (fd == -1)
  {
    recherche_fichier(filename);
    fd = open(filename, O_RDONLY);
  }

  // Si le fichier n'existe pas
  if (fd < 3)
  {
    file_state.type = UNKNOWN_FILE;
    server_send_block(conn_info, &file_state, sizeof(ftp_com));
    return;
  }
  file_state.type = FILE_EXIST;
  server_send_block(conn_info, &file_state, sizeof(ftp_com));

  fstat(fd, &st);
  nb_block = (st.st_size / BLOCK_SIZE) + 1;

  if (rio_readnb(&conn_info.rio, &start_block, sizeof(ftp_com)) != sizeof(ftp_com))
  {
    conn_info.deconnect = 1;
    return;
  }
  if (start_block.type == START_BLOCK)
  {
    lseek(fd, (start_block.value * sizeof(BLOCK_SIZE)), SEEK_SET);
  }
  else
  {
    printf("Error client doesn't send START_BLOCK\n");
  }

  for (int i = start_block.value; i < nb_block; i++)
  {
    block.bl_size = Read(fd, block.buf, BLOCK_SIZE);
    block.block_num = i;
    if (conn_info.deconnect)
    {
      printf("break\n");
      break;
    }
    server_send_block(conn_info, &block, sizeof(ftp_file_transfert));
  }
  Close(fd);
}
