#ifndef __FTP_PROTOCOLE_CLIENT_H__
#define __FTP_PROTOCOLE_CLIENT_H__

#include "ftp_protocole.h"
#include <time.h>

/**
 * @brief Telecharge un fichier sur le serveur
 *
 */
void get_file(rio_t rio);

#endif