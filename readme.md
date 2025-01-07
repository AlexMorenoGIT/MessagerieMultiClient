# README: Messagerie Multi Clients

## Description
Ce projet implémente un serveur de chat multicanal permettant aux utilisateurs de rejoindre ou de créer des canaux, d'envoyer des messages et de recevoir un historique des conversations. 

## Fonctionnalités principales
- **Multi-canal** : Les utilisateurs peuvent créer ou rejoindre des canaux spécifiques.
- **Historique des messages** : Lorsqu'un utilisateur rejoint un canal, il reçoit les messages précédents.
- **Logs serveurs** : Journaux d'informations sur les connexions, déconnexions et erreurs.

## Structure des fichiers
```
|-- common.h           # Définition des constantes et des structures
|-- server.c           # Code source du serveur
|-- client.c           # Code source du client
```

## Compilation

```bash
gcc -o serveur serveur.c -lpthread
gcc -o client client.c -lpthread
```

Cela génère deux exécutables :
- `serveur`
- `client`

## Exécution

### Lancer le serveur
Exécutez le serveur en ligne de commande :

```bash
./serveur
```

Le serveur :
- Tente d’écouter sur le port par défaut défini dans `common.h`.

### Lancer le client
Exécutez le client en ligne de commande :

```bash
./client
```

Le client :
- Se connecte au serveur
- Saisie du nom d'utilisateur et du canal à rejoindre

## Personnalisation

### Modifier le port par défaut
Le port par défaut est défini dans `common.h`. Pour le modifier, changez la valeur de la constante `DEFAULT_SERVER_PORT` :

```c
#define DEFAULT_SERVER_PORT 8000
```

### Limites et paramètres
Vous pouvez ajuster les limites suivantes dans `common.h` :
- **Nombre maximum de clients** : `MAX_CLIENTS`
- **Nombre maximum de canaux** : `MAX_CHANNELS`
- **Longueur maximale des messages** : `MAX_MSG_LENGTH`
- **Historique des messages** : `MAX_HISTORY`

## Ce que j'aurais aimé faire en plus
- Une interface graphique avec GTK
- Partage de fichiers entre utilisateurs
- Des commandes telles que /quit, /create etc...

## Auteur
Ce projet a été développé par [Alex Moreno].
