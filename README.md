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
---
Enfin commencer par lancer le serveur et ensuite le client.

### Côté serveur
Pour le lancer:
```bash
$ ./server
```
le port est optionnel, si il n'est pas préciser le server se lancera par défaut sur le port 88888

### Côté client
Pour le lancer:
```bash
$ ./client <ip>
```
Si le serveur est lancé sur votre machine ou alors que la machine cliente est connecté sur le même réseaux wifi que le serveur, vous pouvez utiliser `localhost` comme adresse ip.
**Attention**, le port doit être le même que pour celui du serveur sinon le client n'arrivera pas à se connecter au serveur.
