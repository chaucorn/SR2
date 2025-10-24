

/*************************************************************
* proto_tdd_v0 -  récepteur                                  *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
*  Protocole Go-Back-N ARQ *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define NUMEROTATION 16 
/* =============================== */
/* Programme principal - récepteur */

int main(int argc, char* argv[])
{
    int window = 1;
    int borne_inf = 0;

    unsigned char message[MAX_INFO];
    paquet_t paquet, pack;
    pack.lg_info = 0;
    pack.type = NACK;
    int fin = 0;
    
    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while (!fin) {
        de_reseau(&paquet);
        if (verifier_control(&paquet) && dans_fenetre(borne_inf, paquet.num_seq, window)){
            pack.num_seq = paquet.num_seq;
            pack.type = ACK;
            pack.somme_ctrl = generer_control(&pack);
            vers_reseau(&pack);
            borne_inf = inc(borne_inf, SEQ_NUM_SIZE);
            for (int i=0; i<paquet.lg_info; i++) {
                message[i] = paquet.info[i];
            }
            fin = vers_application(message, paquet.lg_info);
        }
        else {
            if (pack.type != NACK){ 
                vers_reseau(&pack); 
            }
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
