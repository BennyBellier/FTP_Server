#include "server_file_processor.h"

// recherche le fichier dans la sous-arborescence du dossier courant
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
    // attente de l'exécution du fils
    while (waitpid(-1, &status, WNOHANG) != -1);

    // récupération de la sortie
    read(fds[0], buf, sizeof(buf));
    // on récupère la première occurence de la sortie
    buf[strcspn(buf, "\n")] = 0;
    strcpy(filename, buf);
  }
}

// envoie au client de la description du fichier
int send_file_desc(client_conn_info conn_info, char *filename, long *file_size)
{
  struct stat st;
  int fd = 0;
  ftp_file_descriptor f_desc;

  // Ouverture du fichier
  fd = open(filename, O_RDONLY);

  // recherche du fichier dans les sous-dossiers
  if (fd == -1)
  {
    recherche_fichier(filename);
    fd = open(filename, O_RDONLY);
  }

  if (fd > 0)
  {
    // si le fichier à été ouvert, on remplie la structure avec les informations correspondante
    strcpy(f_desc.name, filename);
    fstat(fd, &st);
    f_desc.size = st.st_size;
    *file_size = st.st_size;
    f_desc.perm = st.st_mode & RWX_UGO;
    f_desc.error = 250;
  }
  else
  {
    // le fichier n'existe pas retour d'un code erreur
    f_desc.error = 550;
  }

  // envoie de la structure au client
  server_send_block(conn_info, &f_desc, sizeof(ftp_file_descriptor));
  return fd;
}

void file_sender_multiblock(client_conn_info conn_info, int fd, long fz, long start_bl_num)
{
  ftp_file_transfert block;
  // calcul du nombre de block pour envoyer tout le fichier
  int nb_block = (fz / MAX_BLOCK_SIZE) + 1;

  for (long i = start_bl_num; i < nb_block; i++)
  {
    // lecture d'une block
    block.bl_size = Read(fd, block.buf, MAX_BLOCK_SIZE);
    block.block_num = (long) i;

    if (conn_info.deconnect)
    {
      // Client deconnecté
      printf("break\n");
      break;
    }

    // envoie du block au client
    server_send_block(conn_info, &block, sizeof(ftp_file_transfert));
  }
}

void get_request(client_conn_info conn_info, char *filename)
{
  int fd;
  long fz;

  // envoie du descripteur de fichier au client
  fd = send_file_desc(conn_info, filename, &fz);

  // fichier non existant, affichage message erreur terminal server
  if (fd < 3)
  {
    printf("%s> Tried to access a non-existent file '%s'\n", conn_info.ip_string, filename);
    return;
  }

  // envoie du fichier
  file_sender_multiblock(conn_info, fd, fz, 0);

  // fermeture du fichier envoyé
  Close(fd);
}

void resume_request(client_conn_info conn_info, char *filename)
{
  int fd;
  struct stat st;
  ftp_com start_block;
  ftp_com file_state;

  fd = open(filename, O_RDONLY); // ouverture du fichier si il existe

  // recherche du fichier dans les sous-dossiers
  if (fd == -1)
  {
    recherche_fichier(filename);
    fd = open(filename, O_RDONLY);
  }

  // fichier non existant, affichage message erreur terminal server
  if (fd < 3)
  {
    file_state.type = UNKNOWN_FILE;
    server_send_block(conn_info, &file_state, sizeof(ftp_com));
    return;
  }

  // Envoie d'un message pour signifier au client que le serveur à trouvé le fichier
  file_state.type = FILE_EXIST;
  server_send_block(conn_info, &file_state, sizeof(ftp_com));

  // informations lié au fichier
  fstat(fd, &st);

  // récupération de l'emplacement de reprise du téléchargement
  if (rio_readnb(&conn_info.rio, &start_block, sizeof(ftp_com)) != sizeof(ftp_com))
  {
    conn_info.deconnect = 1;
    return;
  }
  if (start_block.type == START_BLOCK)
  {
    // on déplace la tête de lecture du fichier à l'endroit ou l'on doit recommencer à envoyer
    lseek(fd, (start_block.value * MAX_BLOCK_SIZE), SEEK_SET);
  }
  else
  {
    printf("Error client doesn't send START_BLOCK\n");
  }

  file_sender_multiblock(conn_info, fd, st.st_size, start_block.value);

  // fermeture du fichier envoyé
  Close(fd);
}
