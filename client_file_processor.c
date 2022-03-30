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

void get_file(serv_conn_info serv_info, char *filename)
{
  ftp_file_descriptor f_desc;
  time_t start, end;
  ftp_file_transfert block;
  int nb_block;
  long nb_block_cur = 0, dl_size = 0;
  char file_part_name[517];

  ftp_com msg;
  strcpy(msg.content, filename);
  msg.value = strlen(msg.content);
  msg.type = GET;
  client_send_block(serv_info, &msg, sizeof(ftp_com));

  rio_readnb(&serv_info.rio, &f_desc, sizeof(ftp_file_descriptor));
  if (f_desc.error == 550)
  {
    printf("FTP 550: File '%s' not found\n", filename);
    return;
  }
  file_name_check(f_desc.name);
  strcpy(file_part_name, f_desc.name);
  strcat(file_part_name, ".part");

  int file = Open(f_desc.name, O_WRONLY | O_CREAT, f_desc.perm);
  int file_part = Open(file_part_name, O_RDWR | O_CREAT, DEF_MODE);

  Write(file_part, &f_desc, sizeof(ftp_file_descriptor));

  nb_block = (f_desc.size / MAXBUF) + 1;

  start = time(NULL);
  if (nb_block > 0)
  {
    do
    {
      rio_readnb(&serv_info.rio, &block, sizeof(ftp_file_transfert));
      if (nb_block_cur == block.block_num)
      {
        Write(file_part, &block, sizeof(ftp_file_transfert));
        ++nb_block_cur;
        dl_size += block.bl_size;
      }
      else
      {
        Fputs("FTP \033[1;35m426\033[0;37m: transfer aborted.\n\033[1;31mServer disconnected\033[0;37m\n", stdout);
        exit(0);
      }
    } while (block.block_num + 1 < nb_block);
  }
  end = time(NULL);

  if (dl_size == f_desc.size)
  {
    end_get_file(file_part, file);
  }

  Close(file);
  Close(file_part);

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
  strcpy(msg.content, filename);
  msg.value = strlen(msg.content);
  msg.type = RESUME;
  client_send_block(serv_info, &msg, sizeof(ftp_com));

  Rio_readnb(&serv_info.rio, &msg, sizeof(ftp_com));
  if (msg.type == UNKNOWN_FILE)
  {
    printf("File doesn't exist\n");
    return;
  }


  strcpy(part_name, filename);
  strcat(part_name, ".part");
  file_name_check(part_name);

  if ((part = Open(part_name, O_RDWR, DEF_MODE)) <= 3)
  {
    printf("Error: no file transfer named '%s' was interrupted\nPlease use: get %s to download it\n", filename, filename);
    return;
  }

  Read(part, &f_desc, sizeof(ftp_file_descriptor));
  file = Open(f_desc.name, O_WRONLY | O_CREAT, f_desc.perm);
  int nb_block = (f_desc.size / MAXBUF) + 1;

  while (read(part, &block, sizeof(ftp_file_transfert)) > 0)
  {
    ++current_block_number;
    dl_size += block.bl_size;
  }


  if (current_block_number == nb_block + 1)
  {
    printf("Error while resume please restart download\n");
    return;
  }
  lseek(part, -(sizeof(ftp_file_transfert)), SEEK_CUR);
  dl_size -= block.bl_size;

  ftp_com start_block;
  start_block.type = START_BLOCK;
  start_block.value = current_block_number;

  client_send_block(serv_info, &start_block, sizeof(ftp_com));

  do
  {
    rio_readnb(&serv_info.rio, &block, sizeof(ftp_file_transfert));
    if (current_block_number == block.block_num)
    {
      Write(part, &block, sizeof(ftp_file_transfert));
      ++current_block_number;
      dl_size += block.bl_size;
    }
    else
    {
      Fputs("FTP \033[1;35m426\033[0;37m: transfer aborted.\n\033[1;31mServer disconnected\033[0;37m\n", stdout);
      exit(0);
    }
  } while (block.block_num + 1 < nb_block);

  if (dl_size == f_desc.size)
  {
    end_get_file(part, file);
    printf("Download successfull\n");
  }
  else
  {
    printf("Error during resume\nPlease retry with get %s\n", f_desc.name);
    remove(part_name);
    remove(f_desc.name);
  }

  Close(file);
  Close(part);



}