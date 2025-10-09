/*************************************************************
* proto_tdd_v2 -  émetteur                                   *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
* Protocole « Stop-and-Wait » ARQ  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/


#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquet; /* paquet utilisé par le protocole */
    int prochain_paquet = 0;
    //int evt = 0; // 0: timeout  1: paquet arrivé
    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        paquet.num_seq = prochain_paquet;
        /* checksum calculation */

        uint8_t sum = generer_control(&paquet);
        paquet.somme_ctrl = sum;

        /* remise à la couche reseau */
        vers_reseau(&paquet);
        depart_temporisateur(50);
        int evt = attendre();
        while (evt != -1){ /*timeout*/
            vers_reseau(&paquet);
            depart_temporisateur(50);
            evt = attendre();
            //de_reseau(&paquet);
        } 
        /*Paquet d'ACK reçu*/
        de_reseau(&paquet);
        arret_temporisateur();
        prochain_paquet = inc(prochain_paquet,2);
        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}

