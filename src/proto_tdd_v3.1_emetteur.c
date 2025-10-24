/*************************************************************
* proto_tdd_v3 -  émetteur                                   *
* TRANSFERT DE DONNEES  v3                                   *
*                                                            *
* Protocole Go-Back-N ARQ  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"
/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */


int main(int argc, char* argv[]){
    int window_size = 4; // Prends la valeur 4 par défaut
    if (argc > 2) {
        printf("Usage: %s <window>\n", argv[0]);
        return 1;
    }
    if (argc == 2) { // Si un argument est passé, on le prend comme taille de la fenêtre
        window_size = atoi(argv[1]);
        if (window_size < 1 && window_size < ((SEQ_NUM_SIZE-1)/2)) {
            printf("Erreur Taille Fenetre\n");
            return 1;
        }
    }

    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t tab_p[SEQ_NUM_SIZE]; 
    paquet_t pack;
    int borne_inf = 0;
    int curseur = 0;
    int evt;
    

    //int evt = 0; // 0: timeout  1: paquet arrivé
    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 || borne_inf != curseur) {
        /* verifier le numero de sequence */
        if (dans_fenetre(borne_inf, curseur, window_size)){
        // Construct the packet
            for (int i=0; i<taille_msg; i++) 
                tab_p[curseur].info[i] = message[i];

            tab_p[curseur].num_seq = curseur;
            tab_p[curseur].lg_info = taille_msg;
            tab_p[curseur].type = DATA;
            tab_p[curseur].somme_ctrl = generer_controle(&tab_p[curseur]);

            printf("[DEBUG] Sending packet seq=%d, taille=%d\n",
                   tab_p[curseur].num_seq, tab_p[curseur].lg_info);
            vers_reseau(&tab_p[curseur]);

            // Start timer if this is the first in the window
            if (borne_inf == curseur)
                depart_temporisateur(100);

            // Move window forward
            curseur = inc(curseur, SEQ_NUM_SIZE);

            // Read next message
            de_application(message, &taille_msg);
        
        /*If the sequence is not in the window*/
        }else{ 
            while(borne_inf != curseur){ 
                if (( evt = attendre()) == PAQUET_RECU){
                    de_reseau(&pack);
                    if (verifier_controle(&pack) && dans_fenetre(borne_inf, pack.num_seq, window_size))
                        borne_inf = inc(pack.num_seq, SEQ_NUM_SIZE);
                    if (borne_inf == curseur) // Arrêt du temporisateur si c'est le dernier paquet de la fenêtre
                        arret_temporisateur();
                }
                else {
                    int i = borne_inf;
                    while (i != curseur) {
                        vers_reseau(&tab_p[i]);
                        i = inc(i, SEQ_NUM_SIZE);
                    }
                    depart_temporisateur(100);
                }

        }
    }
    }// while close
    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
    
    
}

