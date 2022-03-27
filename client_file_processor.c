#include "client_file_processor.h"

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

void file_name_check(char *fname)
{
  char *p = strtok(fname, "/");
  while (p != NULL)
  {
    strcpy(fname, p);
    p = strtok(NULL, "/");
  }
}

void end_get_file(int part, int file)
{
  ftp_file_transfert block;
  ftp_file_descriptor f_desc;
  struct stat st;
  printf("end get request\n");
  lseek(part, 0, SEEK_SET);

  read(part, &f_desc, sizeof(ftp_file_descriptor));

  while (read(part, &block, sizeof(ftp_file_transfert)) > 0)
  {
    Write(file, block.buf, block.bl_size);
  }

  fstat(file, &st);

  if (f_desc.size == st.st_size)
  {
    char part_name[512];
    strcpy(part_name, f_desc.name);
    strcat(part_name, ".part");
    remove(part_name);
  }
}

void get_file(rio_t rio)
{
  ftp_file_descriptor f_desc;
  time_t start, end;
  ftp_file_transfert block;
  int nb_block;
  int nb_block_cur = 0;
  char file_part_name[517];
  long dl_size = 0;

  Rio_readnb(&rio, &f_desc, sizeof(ftp_file_descriptor));
  if (f_desc.error == 550)
  {
    Fputs("FTP 550: File not found\n", stdout);
    return;
  }
  file_name_check(f_desc.name);
  strcpy(file_part_name, f_desc.name);
  strcat(file_part_name, ".part");
  printf("Start downloading\n");

  int file = Open(f_desc.name, O_WRONLY | O_CREAT, f_desc.perm);
  int file_part = Open(file_part_name, O_RDWR | O_CREAT, DEF_MODE);

  Write(file_part, &f_desc, sizeof(ftp_file_descriptor));

  nb_block = (f_desc.size / MAXBUF) + 1;

  start = time(NULL);
  if (nb_block > 0)
  {
    do
    {
      Rio_readnb(&rio, &block, sizeof(ftp_file_transfert));
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

  printf("start resume\n");

  strcpy(part_name, filename);
  strcat(part_name, ".part");
  file_name_check(part_name);

  if ((part = Open(part_name, O_RDWR, DEF_MODE)) <= 3)
  {
    printf("Error: no file transfer named '%s' was interrupted\nPlease use: get %s to download it\n", filename, filename);
    return;
  }

  Read(part, &f_desc, sizeof(ftp_file_descriptor));
  printf("%ld, %s, %d\n", f_desc.size, f_desc.name, f_desc.error);
  file = Open(f_desc.name, O_WRONLY | O_CREAT, f_desc.perm);
  int nb_block = (f_desc.size / MAXBUF) + 1;
  printf("nb block: %d\n", nb_block);

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

  client_send_block(serv_info.fd, &current_block_number, sizeof(long));
  printf("sended block number %ld\n", current_block_number);

  do
  {
    Rio_readnb(&serv_info.rio, &block, sizeof(ftp_file_transfert));
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

  printf("dl siz: %ld\nf_desc: %ld\n", dl_size, f_desc.size);

  if (dl_size == f_desc.size)
  {
    end_get_file(part, file);
    printf("Download successfull\n");
  }
  else
  {
    printf("Error during resume\nPlease retry with get command\n");
  }

  Close(file);
  Close(part);



}