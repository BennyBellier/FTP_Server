# FTP_Server
Ce programme est un projet dans le cadre de ma licence en informatique à l'université Grenoble Alpes. C'est un simple programme contenant un serveur et un client, implémentant les bases du FTP.
Ce programme utilise la bibliothèque **Rio**.


## Comment l'utiliser
Commencer par cloner le répéretoire:
```bash
$ git clone https://github.com/BennyBellier/FTP_Server.git
```
Puis compiler le code:
```bash
$ make
```

Certains paramètres sont modifiable, dans le fichier `ftp_protocole.h` vous pourrez modifier le port par defaut, le nombre de processus pour gérer les connexions client d'un serveur et les dossiers de travail du client et du serveur.
- Les dossiers d'éxecution par défaut sont : pour le serveur le dossier `server_folder` et pour le client le dossier `client_folder`.
- Le port par défaut est 2121
- Par défaut un serveur a 5 processus pour traiter des connexions clients

---
Enfin commencer par lancer le serveur et ensuite le client.

### Côté serveur
Pour le lancer:
```bash
$ ./server <port>
```
le port est optionnel, si il n'est pas préciser le server se lancera sur le port par défaut

### Côté client
Pour le lancer:
```bash
$ ./client <host> <port>
```
Si le serveur est lancé sur votre machine ou alors que la machine cliente est connecté sur le même réseaux wifi que le serveur, vous pouvez utiliser `localhost` comme adresse ip.
**Attention**, le port doit être le même que pour celui du serveur sinon le client n'arrivera pas à se connecter au serveur. Le port par défaut du serveur est le 2121.

---

Les commandes effectuables par le client sont :
```
$ ping
```
le serveur envoie alors comme réponse "pong".

```
$ get <fichier>
```
Si le fichier est présent alors celui-ci sera directement téléchargé sinon une erreur sera affiché sur le terminal
```
$ resume <fichier>
```
Si le téléchargement d'un fichier s'est inopinément interrompu, la commande `resume` va vous permettre de recommencer le téléchargement, là où il à était coupé.
```
$ bye
```
Permet de déconnecter et quitter le client. Le server se deconnectera du client et sera prêt pour récuperer une nouvelle connexion client.