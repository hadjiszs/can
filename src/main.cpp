#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

#include "utils.hpp"
#include "log.hpp"

//#define DEBUG

int main(int argc, char* argv[])
{
#ifndef DEBUG
    std::cerr.setstate(std::ios::failbit);
#endif

    int my_rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(argc >= 2)
        srand(atoi(argv[1])+my_rank);
    else
        srand(my_rank*time(NULL));

    bool continuer = true;
    Node node;
    node.rank = my_rank;

    if(my_rank != COORDINATEUR)
        while(continuer)
            continuer = node.wait_request();
    else
    {   // COORDINATEUR

        // -----
        // ETAPE 1 : Creation Overlay
        std::cout << "\n\n-- Debut de l'execution du programme\n" << std::endl;
        std::cout << "\nETAPE #1 Creation de l'overlay: ";
        std::cout.flush();

        for(int i(1); i < size; i++)
        {
            std::cout << "\rETAPE #1 Creation de l'overlay: invitation noeud #" << i;
            std::cout.flush();
            node.send<INVITATION>(i);

            log_svg("etape1_invit", node, i);
        }

        std::cout << "\rETAPE #1 Creation de l'overlay: terminé                   " << std::endl;

        log_txt("etape1_create_overlay", node, size-1);
        // -----
        // ETAPE 2 : Insertion des données

        std::cout << "\nETAPE #2 Insertion des données aléatoires: ";
        std::cout.flush();

        unsigned int nb_donnee = size*10;
        unsigned int nb_histo  = 5;

        Zone map(Point(MAX, MAX), Point(0,0));

        Point premiere_donnee[nb_histo];
        Point derniere_donnee[nb_histo];

        unsigned int k = 0;

        for(unsigned int i(0); i < nb_donnee; i++)
        {
            Couple donnee_courante = map.random_coord();
            std::cout << "\rETAPE #2 Insertion des données aléatoires: donnée #" << i << " :: " << donnee_courante;
            std::cout.flush();

            // Choix aleatoire du processus destinataire
            int alea_process = rand() % size;
            if(alea_process == 0)
                alea_process = 1;
            
            node.send<INSERT>(alea_process, donnee_courante);

            // Historique seulement les coordonnées des données, sans leurs valeurs
            if(i < nb_histo)
                premiere_donnee[i] = donnee_courante.position;

            if(i >= nb_donnee - nb_histo)
                derniere_donnee[k++] = donnee_courante.position;
        }

        std::cout << "\rETAPE #2 Insertion des données aléatoires: terminé                                              " << std::endl;
        std::cout.flush();

        log_svg("etape2_insert_donnee", node, size-1);
        log_txt("etape2_insert_donnee", node, size-1);

        // -----
        // ETAPE 3 : Recherche et lecture de données
        std::cout << "ETAPE #3 Recherche et lecture des données\n" << std::endl;
        std::cout << " A) " << nb_histo << " première(s) et dernière(s) valeur" << std::endl;
        std::cout << "    " << nb_histo << " première(s) donnée(s) :" << std::endl;

        for(unsigned int i(0); i < nb_histo; i++)
        {
            Couple courant;
            courant.position = premiere_donnee[i];

            // on veut la valeur du couple courant
            // Choix aleatoire du processus destinataire
            int alea_process = rand() % size;
            if(alea_process == 0)
                alea_process = 1;

            courant.valeur = node.get_val(alea_process, courant.position);

            std::cout << "\t" << courant << std::endl;
        }

        std::cout << "\n    " << nb_histo << " dernière(s) donnée(s) :" << std::endl;
        for(unsigned int i(0); i < nb_histo; i++)
        {
            Couple courant;
            courant.position = derniere_donnee[i];

            // on veut la valeur du couple courant
            // Choix aleatoire du processus destinataire
            int alea_process = rand() % size;
            if(alea_process == 0)
                alea_process = 1;

            courant.valeur = node.get_val(alea_process, courant.position);

            std::cout << "\t" << courant << std::endl;
        }

        std::cout << "\n B)  4 au hasard: " << std::endl;
        int alea_process = rand() % size;
        if(alea_process == 0)
            alea_process = 1;

        std::vector<Point> to_search;
        while(to_search.size() < 4)
        {
            std::vector<Couple> data = node.get_data(alea_process);
            // melange au hasard
            std::random_shuffle(data.begin(), data.end());
            if(data.size() != 0)
                to_search.push_back(data[0].position);
        }

        for(unsigned int i(0); i < to_search.size(); i++)
        {
            Couple courant;
            courant.position = to_search[i];

            // on veut la valeur du couple courant
            // Choix aleatoire du processus destinataire
            int alea_process = rand() % size;
            if(alea_process == 0)
                alea_process = 1;

            courant.valeur = node.get_val(alea_process, courant.position);

            std::cout << "\t" << courant << std::endl;
        }
        
        // -----
        // ETAPE 4 : Suppression d'un noeud
        alea_process = rand() % size;
        if(alea_process == 0)
            alea_process = 1;

        int alea2_process = rand() % size;
        if(alea2_process == 0 || alea2_process == alea_process)
            alea2_process = 1;

        std::cout << "\nETAPE #4 Suppression d'un noeud dans l'overlay: suppression du processus #" << alea_process << "\n" << std::endl;
        // insertion de la clef du processus courant en tant que donnée
        Couple couple = node.get_key(alea_process);

        node.send<INSERT>(alea2_process, couple);
        
        // Ordre de suppression à alea_process
        node.send<KILL>(alea_process);        
        
        couple.valeur = node.get_val(alea2_process, couple.position);
        std::cout << "\tclef du processus supprimé: " << couple.position << std::endl;
        std::cout << "\tvaleur associée retrouvée: " << couple.valeur << std::endl;
       
        log_txt("etape4_rm_noeud", node, size-1);
        log_svg("etape4_rm_noeud", node, size-1);
        std::cout << "\n\n-- Execution terminée" << std::endl;
        std::cout << "\n\nLes fichiers de logs se situent dans le dossier build/src/" << std::endl;
    }

    // terminaison
    for(unsigned int i(1); i < size; i++)
        node.send<TERMINER>(i);
    
    MPI_Finalize();
    
    return 0;
}
