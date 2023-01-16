/* =================================================================== */
// Progrmame Serveur qui calcule le résultat d'un coup joué à partir
// des coordonnées reçues de la part d'un client "joueur".
// Version CONCURRENTE : N clients/joueurs à la fois
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

void initTresor(int *vx, int *vy)
{
    *vx = rand() % 10 + 1;
    *vy = rand() % 10 + 1;
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    /*Déclaration des variables*/
    int y_tresor, x_tresor, row, column, result;
    char message[TAILLE_MESSAGE] = "";

    /*Init coords trésor*/
    initTresor(&x_tresor, &y_tresor);
    printf("Coords trésor : %d %d\n", x_tresor, y_tresor);

    int sid = socket(AF_INET, SOCK_STREAM, 0);

    if (sid == -1)
    {
        printf("Erreur creation socket\n");
        return EXIT_FAILURE;
    }

    /*Init socket pour futurs clients*/
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

    if (listen(sid, 15) == -1)
    {
        printf("Erreur listen\n");
        close(sid);
        return EXIT_FAILURE;
    }

    int newSocket;
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(struct sockaddr_in);

    /*On boucle dans l'attente de nouveaux clients, s'il y en a, on fork pour qu'ils puissent
    jouer tout en attendant encore des nouveaux clients*/
    do
    {
        newSocket = accept(sid, (struct sockaddr *)&clientAddress, &clientAddressLength);
        switch (fork())
        {
        case -1:
            perror("Erreur creation processus fils");
            exit(1);

        case 0:
            if (newSocket < 0)
            {
                perror("Erreur acceptation socket\n");
                close(sid);
                exit(EXIT_FAILURE);
            }

            do
            {

                /* Reception coordonnees */
                recv(newSocket, message, TAILLE_MESSAGE * sizeof(char), 0);

                /* Split en 2 int */
                sscanf(message, "%d %d\n", &row, &column);

                /* Calcul de la distance */
                result = recherche_tresor(N, x_tresor, y_tresor, row, column);

                printf("lig %d, col %d, res %d\n", row, column, result);

                /* Serialisation réponse */
                sprintf(message, "%d", result);

                /* Envoie la distance */
                send(newSocket, message, TAILLE_MESSAGE * sizeof(char), 0);
            } while (result);

            break;

        default:
            break;
        }
    } while (newSocket);
    close(newSocket);

    return 0;
}