#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdarg.h>
#include "common.h"

// Déclaration des canaux globaux
Channel channels[MAX_CHANNELS];
int channelCount = 0;

// Déclaration des fonctions
void *handleClient(void *arg);
void sendMessageToChannel(Channel *channel, const char *message);
Channel *findOrCreateChannel(const char *name);
void sendChannelHistory(int clientSocket, Channel *channel);
void logMessage(const char *level, const char *format, ...);
void getCurrentTime(char *buffer, size_t size);

int main() {
    // Création de la socket du serveur
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Erreur lors de la création de la socket");
        logMessage("ERREUR", "Échec de la création de la socket.");
        return EXIT_FAILURE;
    }
    logMessage("INFO", "Socket du serveur créée avec succès.");

    // Configuration de l'adresse du serveur
    struct sockaddr_in serverAddress = {
        .sin_family = AF_INET,
        .sin_port = htons(DEFAULT_SERVER_PORT),
        .sin_addr.s_addr = inet_addr(DEFAULT_SERVER_ADDRESS)
    };

    // Liaison de la socket à l'adresse et au port
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Erreur lors du bind");
        logMessage("ERREUR", "Échec du bind sur le port %d.", DEFAULT_SERVER_PORT);
        close(serverSocket);
        return EXIT_FAILURE;
    }
    logMessage("INFO", "Socket liée à l'adresse %s et au port %d.", DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT);

    // Mise en écoute des connexions
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Erreur lors de l'écoute");
        logMessage("ERREUR", "Échec de l'écoute sur le port %d.", DEFAULT_SERVER_PORT);
        close(serverSocket);
        return EXIT_FAILURE;
    }
    logMessage("INFO", "Serveur en écoute sur l'adresse %s et le port %d.", DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT);

    while (1) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);

        // Acceptation d'une nouvelle connexion client
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);
        if (clientSocket < 0) {
            perror("Erreur lors de l'acceptation d'un client");
            logMessage("ERREUR", "Échec de l'acceptation de la connexion client.");
            continue;
        }

        // Création d'un thread pour gérer ce client
        pthread_t thread;
        if (pthread_create(&thread, NULL, handleClient, (void *)(intptr_t)clientSocket) != 0) {
            perror("Erreur lors de la création du thread");
            logMessage("ERREUR", "Échec de la création du thread pour le client.");
            close(clientSocket);
        }

        // Détachement du thread pour qu'il fonctionne indépendamment
        pthread_detach(thread);
    }

    close(serverSocket);
    return 0;
}

// Fonction pour gérer un client
void *handleClient(void *arg) {
    int clientSocket = (intptr_t)arg;
    char buffer[MAX_MSG_LENGTH];
    char username[MAX_NAME_LENGTH];
    char channelName[MAX_NAME_LENGTH];
    char timeBuffer[9];

    // Réception du nom d'utilisateur
    recv(clientSocket, username, sizeof(username), 0);
    username[strcspn(username, "\n")] = 0;
    logMessage("INFO", "Utilisateur connecté : %s", username);

    // Réception du nom du canal
    recv(clientSocket, channelName, sizeof(channelName), 0);
    channelName[strcspn(channelName, "\n")] = 0;

    // Recherche ou création du canal
    Channel *channel = findOrCreateChannel(channelName);

    // Log de la création du canal si nouveau
    if (channel->clientCount == 0) {
        logMessage("INFO", "Nouveau channel créé : %s", channelName);
    }

    channel->clients[channel->clientCount++] = malloc(sizeof(Client));
    strcpy(channel->clients[channel->clientCount - 1]->username, username);
    channel->clients[channel->clientCount - 1]->socket = clientSocket;

    getCurrentTime(timeBuffer, sizeof(timeBuffer));
    snprintf(buffer, sizeof(buffer), COLOR_MAGENTA "[%s] %s a rejoint le channel.\n" COLOR_RESET, timeBuffer, username);
    sendMessageToChannel(channel, buffer);
    logMessage("INFO", "%s a rejoint le channel %s", username, channelName);

    sendChannelHistory(clientSocket, channel);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            logMessage("INFO", "%s s'est déconnecté.", username);
            getCurrentTime(timeBuffer, sizeof(timeBuffer));
            snprintf(buffer, sizeof(buffer), COLOR_MAGENTA "[%s] %s a quitté le channel.\n" COLOR_RESET, timeBuffer, username);
            sendMessageToChannel(channel, buffer);
            logMessage("INFO", "%s a quitté le channel %s", username, channelName);
            break;
        }

        buffer[bytesReceived] = '\0';
        char formattedMessage[MAX_MSG_LENGTH];
        getCurrentTime(timeBuffer, sizeof(timeBuffer));
        snprintf(formattedMessage, sizeof(formattedMessage), COLOR_YELLOW "[%s] [%s] : %s" COLOR_RESET, timeBuffer, username, buffer);
        sendMessageToChannel(channel, formattedMessage);
        logMessage("LOG", "Message de %s dans le channel %s: %s", username, channelName, buffer);
    }

    close(clientSocket);
    return NULL;
}

// Fonction pour obtenir l'heure actuelle au format HH:MM:SS
void getCurrentTime(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%H:%M:%S", t);
}

// Fonction pour envoyer un message à tous les clients d'un canal
void sendMessageToChannel(Channel *channel, const char *message) {
    // Enregistrement du message dans l'historique
    if (channel->messageCount < MAX_HISTORY) {
        strncpy(channel->messages[channel->messageCount++], message, MAX_MSG_LENGTH);
    }

    // Envoi du message à tous les clients du canal
    for (int i = 0; i < channel->clientCount; ++i) {
        int clientSocket = channel->clients[i]->socket;
        send(clientSocket, message, strlen(message), 0);
    }
}

// Fonction pour trouver ou créer un canal
Channel *findOrCreateChannel(const char *name) {
    for (int i = 0; i < channelCount; ++i) {
        if (strcmp(channels[i].name, name) == 0) {
            return &channels[i];
        }
    }

    Channel *newChannel = &channels[channelCount++];
    strncpy(newChannel->name, name, MAX_NAME_LENGTH);
    newChannel->messageCount = 0;
    newChannel->clientCount = 0;
    return newChannel;
}

// Fonction pour envoyer l'historique d'un canal au client
void sendChannelHistory(int clientSocket, Channel *channel) {
    for (int i = 0; i < channel->messageCount; ++i) {
        char formattedMessage[MAX_MSG_LENGTH + 2]; // +2 pour ajouter "\n\0"
        snprintf(formattedMessage, sizeof(formattedMessage), "%s\n", channel->messages[i]);
        send(clientSocket, formattedMessage, strlen(formattedMessage), 0);
    }
}

// Fonction de log pour le serveur
void logMessage(const char *level, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);

    if (strcmp(level, "INFO") == 0) {
        printf(COLOR_CYAN "[INFO] %s" COLOR_RESET "\n", buffer);
    } else if (strcmp(level, "ERREUR") == 0) {
        printf(COLOR_RED "[ERREUR] %s" COLOR_RESET "\n", buffer);
    } else if (strcmp(level, "LOG") == 0) {
        printf(COLOR_YELLOW "[LOG] %s" COLOR_RESET "\n", buffer);
    } else {
        printf("[LOG] %s\n", buffer); // Par défaut, sans couleur
    }

    va_end(args);
}
