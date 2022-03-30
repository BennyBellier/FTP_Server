#include "client_file_processor.h"

// Affichage des informations de téléchargement (taille du fichiers, temps, vitesse de téléchargement)
void display_download_info(long f_size, long dl_size, time_t start, time_t end)
{
  if (f_size == dl_size)
  {
    unsigned long delta = (unsigned long)difftime(end, start);
    double speed = dl_size;
    if (delta > 0)
      speed = speed / delta;

    printf("Download successfull\n");

    if (dl_size > 1000000000)
    {
      printf("%.2ld Gbytes", dl_size / 1000000000);
    }
    else if (dl_size > 10000000)
    {
      printf("%.2ld Mbytes", dl_size / 1000000);
    }
    else if (dl_size > 1000)
    {
      printf("%.2ld Kbytes", dl_size / 1000);
    }
    else
      printf("%ld bytes", dl_size);

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
  else
  {
    printf("Download failed\n%ld%% of the file been download!\n", (dl_size * 100) / f_size);
  }
}

// Récupératio seulement du nom du fichiers (sans le chemins de dossier(s))
void file_name_check(char *fname)
{
  char *p = strtok(fname, "/");
  while (p != NULL)
  {
    strcpy(fname, p);
    p = strtok(NULL, "/");
  }
}

// Écriture du fichier final
void end_get_file(int part, int file)
{
  ftp_file_transfert block;
  ftp_file_descriptor f_desc;
  struct stat st;
  // Reset de la tête de lecture au début du fichier
  lseek(part, 0, SEEK_SET);

  // lecture du descripteur de fichier
  read(part, &f_desc, sizeof(ftp_file_descriptor));

  // Boucle de lecture des blocks téléchargé dans le fichier '.part' avec leurs informations complémentaires
  while (read(part, &block, sizeof(ftp_file_transfert)) > 0)
  {
    // Écriture dans le fichier final
    Write(file, block.buf, block.bl_size);
  }

  // Vérification fichier téléchargé conforme
  fstat(file, &st);
  if (f_desc.size == st.st_size)
  {
    char part_name[512];
    strcpy(part_name, f_desc.name);
    strcat(part_name, ".part");
    remove(part_name);
  }
  else
  {
    char part_name[512];
    printf("Erreur fichiers corrompu\nRééssayer en utilisant la commande 'get'\n");
    strcpy(part_name, f_desc.name);
    strcat(part_name, ".part");
    remove(f_desc.name);
    remove(part_name);
  }
}

long file_receive_multiblock(serv_conn_info serv_info, ftp_file_descriptor f_desc, int file_part, long nb_block, long nb_block_cur)
{
  ftp_file_transfert block;
  long dl_size = 0;

  if (nb_block > 0)
  {
    do
    {
      // lecture desblocks envoyé par le serveur
      rio_readnb(&serv_info.rio, &block, sizeof(ftp_file_transfert));
      // Vérification qu'il n'y a pas eu de saut de block
      if (nb_block_cur == block.block_num)
      {
        // si block correct alors écriture du block dans le fichier .part
        Write(file_part, &block, sizeof(ftp_file_transfert));
        ++nb_block_cur;
        // Récupération de la taille des block du fichier téléchargé
        dl_size += block.bl_size;
      }
      else
      {
        // Sinon affichage d'une erreur
        Fputs("FTP \033[1;35m426\033[0;37m: transfer aborted.\n\033[1;31mServer disconnected\033[0;37m\n", stdout);
        exit(0);
      }
    } while (block.block_num + 1 < nb_block);
  }
  return dl_size;
}

void get_file(serv_conn_info serv_info, char *filename)
{
  ftp_file_descriptor f_desc;
  time_t start, end;
  char file_part_name[517];

  ftp_com msg;
  // Envoie du nom du fichier au serveur en tant que requête GET
  strcpy(msg.content, filename);
  msg.value = strlen(msg.content);
  msg.type = GET;
  client_send_block(serv_info, &msg, sizeof(ftp_com));

  // Récupération de la description du fichier
  rio_readnb(&serv_info.rio, &f_desc, sizeof(ftp_file_descriptor));
  if (f_desc.error == 550)
  {
    // Erreur si le fichier n'existe pas
    printf("FTP 550: File '%s' not found\n", filename);
    return;
  }
  file_name_check(f_desc.name);
  strcpy(file_part_name, f_desc.name);
  strcat(file_part_name, ".part");

  // Création du fichier à télécharger avec les permissions associé
  int file = Open(f_desc.name, O_WRONLY | O_CREAT, f_desc.perm);
  // Création d'un fichier '.part', pour restauration du téléchargement
  int file_part = Open(file_part_name, O_RDWR | O_CREAT, DEF_MODE);

  // On écrit au début du .part la description du fichier auxquel il correspond
  Write(file_part, &f_desc, sizeof(ftp_file_descriptor));

  // Calcul du nombre de block à téléchargé
  long nb_block = (f_desc.size / MAX_BLOCK_SIZE) + 1;

  // Démarrage du téléchargement
  start = time(NULL);
  long dl_size = file_receive_multiblock(serv_info, f_desc, file_part, nb_block, 0);
  end = time(NULL);

  // Vérification si la taille téléchargé correspond à la taille du fichier
  if (dl_size == f_desc.size)
  {
    // Lecture du fichier .part et écriture du fichier final
    end_get_file(file_part, file);
  }

  // fermeture des fichiers
  Close(file);
  Close(file_part);

  // affichage des informations lié au téléchargement
  display_download_info(f_desc.size, dl_size, start, end);
}

void resume_get_file(serv_conn_info serv_info, char *filename)
{
  ftp_file_descriptor f_desc;
  ftp_file_transfert block;
  char part_name[517];
  int part, file;
  long current_block_number = -1;
  long dl_size = 0;

  ftp_com msg;
  // Envoie du nom du fichier au serveur en tant que requête RESUME
  strcpy(msg.content, filename);
  msg.value = strlen(msg.content);
  msg.type = RESUME;
  client_send_block(serv_info, &msg, sizeof(ftp_com));

  // Lecture de la réponse serveur
  Rio_readnb(&serv_info.rio, &msg, sizeof(ftp_com));
  if (msg.type == UNKNOWN_FILE)
  {
    // Affichage erreur si fichier introuvable
    printf("File doesn't exist\n");
    return;
  }

  strcpy(part_name, filename);
  strcat(part_name, ".part");
  file_name_check(part_name);

  // vérification si un transfert avait déjà était commencé avec ce fichier
  if ((part = Open(part_name, O_RDWR, DEF_MODE)) <= 3)
  {
    // Aucun téléchargement érroné avec ce fichier, donc affiachage erreur
    printf("Error: no file transfer named '%s' was interrupted\nPlease use: get %s to download it\n", filename, filename);
    return;
  }

  // lecture du descripteur de fichier contenue dans le '.part'
  Read(part, &f_desc, sizeof(ftp_file_descriptor));
  // ouverture du fichier à téléchargé
  file = Open(f_desc.name, O_WRONLY | O_CREAT, f_desc.perm);
  // calcul du nombre de block à téléchargé
  long nb_block = (f_desc.size / MAX_BLOCK_SIZE) + 1;

  // Lecture des blocks déjà téléchargé
  while (read(part, &block, sizeof(ftp_file_transfert)) > 0)
  {
    ++current_block_number;
    dl_size += block.bl_size;
  }

  // Si tout le fichier est dans le fichier '.part' il y a eu une erreur
  if (current_block_number == nb_block + 1)
  {
    printf("Error while resume please restart download\n");
    return;
  }

  // On recule la tête de lecture du fichier '.part' pour retélécharger le dernier block
  // dans le cas où celui-ci aurait était corrompu quand le téléchargement c'est coupé
  lseek(part, -(sizeof(ftp_file_transfert)), SEEK_CUR);
  // on enlève la taille du dernier block de la taille global téléchargé
  dl_size -= block.bl_size;

  ftp_com start_block;
  // Envoie du numéro du block d'où recommencer le téléchargement
  start_block.type = START_BLOCK;
  start_block.value = current_block_number;
  client_send_block(serv_info, &start_block, sizeof(ftp_com));

  // récupération du fichier
  dl_size += file_receive_multiblock(serv_info, f_desc, part, nb_block, current_block_number);

  // Vérification si la taille téléchargé correspond à la taille du fichier
  if (dl_size == f_desc.size)
  {
    // Lecture du fichier .part et écriture du fichier final
    end_get_file(part, file);
    printf("Download successfull\n");
  }
  else
  {
    // Affichage d'une erreur sinon
    printf("Error during resume\nPlease retry with get %s\n", f_desc.name);
    remove(part_name);
    remove(f_desc.name);
  }

  // fermeture des fichiers
  Close(file);
  Close(part);
}