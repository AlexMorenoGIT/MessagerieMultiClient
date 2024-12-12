#ifndef COMMON_H
#define COMMON_H

// Maximum parameters
#define MAX_CHANNELS 10        // Nombre maximum de canaux que le serveur peut gérer
#define MAX_CLIENTS 50         // Nombre maximum de clients par canal
#define MAX_MSG_LENGTH 256     // Taille maximale d'un message
#define MAX_NAME_LENGTH 32     // Taille maximale pour un nom d'utilisateur ou de canal
#define MAX_HISTORY 100        // Nombre maximum de messages à conserver dans l'historique d'un canal

// Colors for terminal output
#define COLOR_RESET   "\033[0m"   // Réinitialisation de la couleur
#define COLOR_RED     "\033[31m"  // Rouge
#define COLOR_GREEN   "\033[32m"  // Vert
#define COLOR_YELLOW  "\033[33m"  // Jaune
#define COLOR_BLUE    "\033[34m"  // Bleu
#define COLOR_MAGENTA "\033[35m"  // Magenta
#define COLOR_CYAN    "\033[36m"  // Cyan
#define COLOR_WHITE   "\033[37m"  // Blanc

// Structure to store client information
typedef struct {
    char username[MAX_NAME_LENGTH];   // Nom d'utilisateur
    int socket;                       // Socket du client
    char currentChannel[MAX_NAME_LENGTH]; // Nom du canal auquel le client est connecté
} Client;

// Structure to store channel information
typedef struct {
    char name[MAX_NAME_LENGTH];                  // Nom du canal
    char messages[MAX_HISTORY][MAX_MSG_LENGTH]; // Historique des messages
    int messageCount;                           // Nombre de messages dans l'historique
    Client *clients[MAX_CLIENTS];               // Liste des clients connectés
    int clientCount;                            // Nombre de clients connectés
} Channel;

#endif
