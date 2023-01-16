/* =================================================================== */
// Progrmame Client à destination d'un joueur qui doit deviner la case
// du trésor. Après chaque coup le résultat retourné par le serveur est
// affiché. Le coup consite en une abcsisse et une ordonnée (x, y).
/* =================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define N 10
#define TAILLE_MESSAGE 6

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define MAGENTA "\033[35m"

/* ====================================================================== */
/*                  Affichage du jeu en mode texte brut                   */
/* ====================================================================== */
void afficher_jeu(int jeu[N][N], int res, int points, int coups)
{

    printf("\n************ TROUVEZ LE TRESOR ! ************\n");
    printf("    ");
    for (int i = 0; i < 10; i++)
        printf("  %d ", i + 1);
    printf("\n    -----------------------------------------\n");
    for (int i = 0; i < 10; i++)
    {
        printf("%2d  ", i + 1);
        for (int j = 0; j < 10; j++)
        {
            printf("|");
            switch (jeu[i][j])
            {
            case -1:
                printf(" 0 ");
                break;
            case 0:
                printf(GREEN " T " RESET);
                break;
            case 1:
                printf(YELLOW " %d " RESET, jeu[i][j]);
                break;
            case 2:
                printf(RED " %d " RESET, jeu[i][j]);
                break;
            case 3:
                printf(MAGENTA " %d " RESET, jeu[i][j]);
                break;
            }
        }
        printf("|\n");
    }
    printf("    -----------------------------------------\n");
    printf("Pts dernier coup %d | Pts total %d | Nb coups %d\n", res, points, coups);
}

/* ====================================================================== */
/*                    Fonction principale                                 */
/* ====================================================================== */
int main(int argc, char **argv)
{

    if (argc != 3)
    {
        printf("Usage : %s [IP] [PORT]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Init variables */
    int jeu[N][N];
    int lig, col;
    int res = -1, points = 0, coups = 0;
    char mess[TAILLE_MESSAGE] = "";

    /* Init jeu */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            jeu[i][j] = -1;

    /* Creation socket TCP */
    int socketId = socket(AF_INET, SOCK_STREAM, 0);

    if (socketId == -1)
    {
        printf("Erreur creation socket\n");
        return EXIT_FAILURE;
    }

    /* Init socket serveur dsitant*/
    struct sockaddr_in distantServerSocket;

    distantServerSocket.sin_family = AF_INET;
    distantServerSocket.sin_port = htons(atoi(argv[2]));

    /* Verif adresse IP */
    if (inet_pton(AF_INET, argv[1], &(distantServerSocket.sin_addr)) != 1)
    {
        printf("Erreur conversion de l'adresse IP\n");
        return EXIT_FAILURE;
    }

    /* Connection au process distant */
    if (connect(socketId, (struct sockaddr *)&distantServerSocket, sizeof(struct sockaddr_in)) == -1)
    {
        printf("Erreur connexion\n");
        close(socketId);
        return EXIT_FAILURE;
    }

    /* Tant que le trésors n'est pas trouvé on redemande les coords puis envoie un packet serialisé */
    do
    {
        afficher_jeu(jeu, res, points, coups);
        printf("\nEntrer le numéro de ligne : ");
        scanf("%d", &lig);
        printf("Entrer le numéro de colonne : ");
        scanf("%d", &col);

        /* Construction requete  */
        sprintf(mess, "%d %d\n", lig, col);

        /* Expedition de la requete */
        send(socketId, mess, TAILLE_MESSAGE * sizeof(char), 0);

        memset(mess, 0, TAILLE_MESSAGE * sizeof(char));

        /* Reception résultat */
        recv(socketId, mess, TAILLE_MESSAGE * sizeof(char), 0);

        /* Deserialisation en int */
        sscanf(mess, "%d", &res);

        /* display */
        if (lig >= 1 && lig <= N && col >= 1 && col <= N)
            jeu[lig - 1][col - 1] = res;
        points += res;
        coups++;
    } while (res);

    /* Fermeture socket */
    close(socketId);

    afficher_jeu(jeu, res, points, coups);
    printf("\nBRAVO : trésor trouvé en %d essai(s) avec %d point(s)"
           " au total !\n\n",
           coups, points);
    return 0;
}
