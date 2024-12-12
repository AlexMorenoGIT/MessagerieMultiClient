#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "common.h"

// Prototypes des fonctions
void *receiveMessages(void *arg);
void displayBanner(const char *channelName);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage : %s <adresse_serveur> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Création de la socket du client
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Erreur lors de la création de la socket");
        return EXIT_FAILURE;
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in serverAddress = {
        .sin_family = AF_INET,
        .sin_port = htons(atoi(argv[2]))
    };
    inet_pton(AF_INET, argv[1], &serverAddress.sin_addr);

    // Connexion au serveur
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Erreur lors de la connexion au serveur");
        return EXIT_FAILURE;
    }

    char username[MAX_NAME_LENGTH];
    char channelName[MAX_NAME_LENGTH];
    char message[MAX_MSG_LENGTH];

    // Saisie du nom d'utilisateur
    printf("Entrez votre nom d'utilisateur : ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0'; // Retire le saut de ligne
    send(clientSocket, username, strlen(username), 0);

    // Saisie du nom du canal
    printf("Entrez le nom du canal : ");
    fgets(channelName, sizeof(channelName), stdin);
    channelName[strcspn(channelName, "\n")] = '\0'; // Retire le saut de ligne
    send(clientSocket, channelName, strlen(channelName), 0);

    // Affichage de la bannière d'accueil
    displayBanner(channelName);

    // Création d'un thread pour recevoir les messages du serveur
    pthread_t thread;
    pthread_create(&thread, NULL, receiveMessages, (void *)(intptr_t)clientSocket);

    // Boucle principale pour envoyer des messages au serveur
    while (1) {
        printf(COLOR_BLUE "Vous : " COLOR_RESET);
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0'; // Retire le saut de ligne
        send(clientSocket, message, strlen(message), 0);
    }

    close(clientSocket);
    return 0;
}

// Fonction pour recevoir les messages du serveur
void *receiveMessages(void *arg) {
    int clientSocket = (intptr_t)arg;
    char buffer[MAX_MSG_LENGTH];

    while (1) {
        memset(buffer, 0, sizeof(buffer)); // Nettoyage du tampon
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        // Si la connexion est interrompue
        if (bytesReceived <= 0) {
            printf(COLOR_RED "Déconnecté du serveur.\n" COLOR_RESET);
            close(clientSocket);
            pthread_exit(NULL);
        }

        buffer[bytesReceived] = '\0'; // Ajout du caractère nul
        printf(COLOR_GREEN "%s\n" COLOR_RESET, buffer);
    }
    return NULL;
}

// Fonction pour afficher une bannière avec le nom du canal
void displayBanner(const char *channelName) {
    system("clear"); // Efface l'écran
    printf(COLOR_CYAN "=========================================\n" COLOR_RESET);
    printf(COLOR_YELLOW "     Bienvenue dans le canal %s\n" COLOR_RESET, channelName);
    printf(COLOR_CYAN "=========================================\n\n" COLOR_RESET);
}