/* =================================================================== */
// Progrmame Serveur qui calcule le résultat d'un coup joué à partir
// des coordonnées reçues de la part d'un client "joueur".
// Version ITERATIVE : 1 seul client/joueur à la fois
/* =================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "tresor.h"

#define N 10
#define TAILLE_MESSAGE 6

/* =================================================================== */
/* FONCTION PRINCIPALE : SERVEUR ITERATIF                              */
/* =================================================================== */

void initTresor(int *vx, int *vy)
{
    *vx = rand() % 10 + 1;
    *vy = rand() % 10 + 1;
}

int main(int argc, char **argv)
{

    srand(time(NULL));

    /* Déclaration des variables */
    int x_tresor, y_tresor, column, row, result;
    char mess[TAILLE_MESSAGE] = "";

    /*Initialisation coords trésor*/
    initTresor(&x_tresor, &y_tresor);
    printf("Coords trésor : %d %d\n", x_tresor, y_tresor);

    int sid = socket(AF_INET, SOCK_STREAM, 0);

    if (sid == -1)
    {
        printf("Erreur creation socket\n");
        return EXIT_FAILURE;
    }

    /* Init socket pour futurs client */
    struct sockaddr_in socketAddress;
    memset(&socketAddress, 0, sizeof(struct sockaddr_in));
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(5555);
    socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sid, (struct sockaddr *)&socketAddress, sizeof(struct sockaddr_in)) == -1)
    {
        printf("Erreur bind\n");
        close(sid);
        return EXIT_FAILURE;
    }

    if (listen(sid, 4) == -1)
    {
        printf("Erreur ecoute\n");
        close(sid);
        return EXIT_FAILURE;
    }

    int connectionId;
    struct sockaddr_in clientSocket;
    socklen_t clientAddressLength = sizeof(struct sockaddr_in);

    /* Tant que le socket est valide et qu'on recoit des connections on les accepte, joue avec puis passe au client suivant */
    do
    {
        connectionId = accept(sid, (struct sockaddr *)&clientSocket, &clientAddressLength);
        if (connectionId < 0)
        {
            printf("Erreur acceptation socket");
            close(sid);
            exit(EXIT_FAILURE);
        }

        do
        {

            /* Reception coordonnees */
            recv(connectionId, mess, TAILLE_MESSAGE * sizeof(char), 0);

            /* Split en 2 int */
            sscanf(mess, "%d %d\n", &row, &column);

            /* Calcul de la distance */
            result = recherche_tresor(N, x_tresor, y_tresor, row, column);

            printf("lig %d, col %d, res %d\n", row, column, result);

            /* Serialisation réponse */
            sprintf(mess, "%d", result);

            /* Envoie la distance */
            send(connectionId, mess, TAILLE_MESSAGE * sizeof(char), 0);
        } while (result);

    } while (connectionId);
    close(connectionId);

    return 0;
}