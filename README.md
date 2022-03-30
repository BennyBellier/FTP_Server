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

---
# Répartiteur de charge
Le répartiteur de charge n'est pas entièrement fonctionnel. Les serveur esclaves peuvent s'y connecter, et les client aussi, mais les clients ne reçoivent pas les informations de connexion au serveur escalve.

## Fonctionnement du répartiteur
Mon but était que dans un premier temps on lance le répartiteur de charge. Ensuite on lance les serveurs esclaves, chaque serveur esclave allaient se connecter au répartiteur et lui communiquer leur informations de connexion. Ensuite, une fois que les serveur esclaves, avait communiqué leur informations, les processus de gestion de connexion client était générer. En même temps un processus était générer dans le répartiteur pour rediriger les conexions des clients.

Je comptais utilisé une file, pour gérer la politque de Round-Robin.

### Problèmes
Le problème est que lorsque que le serveur est censé renvoyant les informations de connexion au client, celui-ci reçoit des valeurs nul.