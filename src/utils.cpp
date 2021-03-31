#include "utils.hpp"

bool Node::wait_request(int rank_src, int tag)
{
    MPI_Status status;

    MPI_Probe(rank_src, tag, MPI_COMM_WORLD, &status);

    switch(status.MPI_TAG) {
    case INVITATION:
    {
        // Attente d'invitation de la part du coordinateur
        this->recv<INVITATION>(COORDINATEUR);

        if(this->rank != BOOTSTRAP)
            // Demande d'insertion au bootstrap
            this->send<JOIN>(BOOTSTRAP);

        // Ack le coordinateur que l'insertion est ok pour qu'il puisse inviter un autre process
        this->ack<INVITATION>(COORDINATEUR);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case JOIN:
    {
        this->recv<JOIN>(status.MPI_SOURCE);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case INSERT:
    {
        Couple couple_donnee;
        // Reception de l'ordre d'inserer
        MPI_Recv(&couple_donnee, sizeof(Couple), MPI_BYTE, status.MPI_SOURCE, INSERT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if(this->contient(couple_donnee.position))
            this->data.push_back(couple_donnee);
        else
            this->send<NEWDATA>(this->voisins[0], couple_donnee);

        // Ack le coordinateur que l'insertion de donnee est ok
        this->ack<INSERT>(status.MPI_SOURCE);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case NEWDATA:
    {
        this->recv<NEWDATA>(status.MPI_SOURCE);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case GET_KEY:
    {
        MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, GET_KEY, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Envoie de la clef
        MPI_Send(&this->key, sizeof(Point), MPI_BYTE, status.MPI_SOURCE, GET_KEY+1, MPI_COMM_WORLD);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case GET_ZONE:
    {
        MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, GET_ZONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Envoie de la zone
        MPI_Send(&(this->chunk), sizeof(Zone), MPI_BYTE, status.MPI_SOURCE, GET_ZONE+1, MPI_COMM_WORLD);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case GET_DATA:
    {
        MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, GET_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Envoie des points

        std::size_t nb_donnee = this->data.size();
        MPI_Send(&nb_donnee, sizeof(std::size_t), MPI_BYTE, status.MPI_SOURCE, GET_DATA+1, MPI_COMM_WORLD);
        MPI_Send(this->data.data(), nb_donnee*sizeof(Couple), MPI_BYTE, status.MPI_SOURCE, GET_DATA+2, MPI_COMM_WORLD);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case GET_VALEUR:
    {
        Point pos;
        int valeur = 0;

        // Reception de l'ordre d'inserer
        MPI_Recv(&pos, sizeof(Point), MPI_BYTE, status.MPI_SOURCE, GET_VALEUR, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if(this->contient(pos))
            valeur = this->recherche_valeur(pos);
        else
            valeur = this->send<SEARCH_VALEUR>(this->voisins[0], pos);

        MPI_Send(&valeur, sizeof(int), MPI_BYTE, status.MPI_SOURCE, GET_VALEUR+1, MPI_COMM_WORLD);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case GET_STATUS:
    {
        MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, GET_STATUS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Envoie du status
        MPI_Send(&(this->enable), sizeof(bool), MPI_BYTE, status.MPI_SOURCE, GET_STATUS+1, MPI_COMM_WORLD);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case SET_DATA:
    {
        // Reception de la nouvelle zone
        MPI_Recv(&(this->chunk), sizeof(Zone), MPI_BYTE, status.MPI_SOURCE, SET_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Reception des nouvelles données
        std::vector<Couple> res;

        std::size_t nb_donnee = 0;
        MPI_Recv(&nb_donnee, sizeof(std::size_t), MPI_BYTE, status.MPI_SOURCE, SET_DATA+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        res.resize(nb_donnee);
        MPI_Recv(res.data(), nb_donnee*sizeof(Couple), MPI_BYTE, status.MPI_SOURCE, SET_DATA+2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Reception des voisins
        std::vector<int> new_voisin;

        MPI_Recv(&nb_donnee, sizeof(std::size_t), MPI_BYTE, status.MPI_SOURCE, SET_DATA+3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        new_voisin.resize(nb_donnee);
        MPI_Recv(new_voisin.data(), nb_donnee*sizeof(int), MPI_BYTE, status.MPI_SOURCE, SET_DATA+4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Ajout des couples de valeurs reçu
        for(unsigned int i(0); i < res.size(); i++)
            this->data.push_back(res[i]);

        // Ajout des nouveaux voisins
        for(unsigned int i(0); i < new_voisin.size(); i++)
        {
            this->voisins.push_back(new_voisin[i]);
            MPI_Send(&(this->rank), sizeof(int), MPI_BYTE, new_voisin[i], ADD_ME, MPI_COMM_WORLD);
        }
    }
    break;
    // ---------------------------------------------------------------------------------------
    case KILL:
    {
        MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, KILL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Auto-suppression

        // Selection du voisin parfait (coordonnées qui se concilie parfaitement sur un axe, i.e le voisin dont est issue le noeud courant)
        unsigned int i(0);
        Zone voisin;
        for(; i < this->voisins.size(); i++)
        {
            voisin = get_zone(this->voisins[i]);
            if(this->chunk.voisin_parfait(voisin))
                break;
        }

        // fusion des deux zones
        Zone fusion_zone = this->chunk.fusion(voisin);
        // Envoie de la fusion des deux zones
        MPI_Send(&fusion_zone, sizeof(Zone), MPI_BYTE, this->voisins[i], SET_DATA, MPI_COMM_WORLD);

        std::size_t nb = this->data.size();
        // Envoie des données au voisin choisi
        MPI_Send(&nb, sizeof(std::size_t), MPI_BYTE, this->voisins[i], SET_DATA+1, MPI_COMM_WORLD);
        MPI_Send(this->data.data(), this->data.size()*sizeof(Couple), MPI_BYTE, this->voisins[i], SET_DATA+2, MPI_COMM_WORLD);

        // Envoie des voisins
        nb = this->voisins.size();
        MPI_Send(&nb, sizeof(std::size_t), MPI_BYTE, this->voisins[i], SET_DATA+3, MPI_COMM_WORLD);
        MPI_Send(this->voisins.data(), this->voisins.size()*sizeof(int), MPI_BYTE, this->voisins[i], SET_DATA+4, MPI_COMM_WORLD);

        // Mise à jour de la liste de voisins des voisins
        for(unsigned int k(0); k < this->voisins.size(); k++)
            MPI_Send(NULL, 0, MPI_CHAR, this->voisins[k], RM_ME, MPI_COMM_WORLD);

        this->enable = false;

        MPI_Send(NULL, 0, MPI_BYTE, status.MPI_SOURCE, KILL+1, MPI_COMM_WORLD);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case SEARCH_VALEUR:
    {
        this->recv<SEARCH_VALEUR>(status.MPI_SOURCE);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case RM_ME:
    {
        MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, RM_ME, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Suppression du voisin expediteur de la requete
        std::cerr << "\n[" << this->rank << "] RM VOISIN: " << status.MPI_SOURCE << std::endl;
        for(unsigned int j(0); j < this->voisins.size(); j++)
            if(this->voisins[j] == status.MPI_SOURCE)
                this->voisins.erase(this->voisins.begin()+j);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case ADD_ME:
    {
        bool deja_voisin = false;
        int rank_src = this->voisins[0];

        MPI_Recv(&rank_src, sizeof(int), MPI_BYTE, status.MPI_SOURCE, ADD_ME, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cerr << "\n[" << this->rank << "] ADD VOISIN: " << status.MPI_SOURCE << std::endl;
        // Ajout dans les voisins le noeud expediteur
        for(unsigned int j(0); j < this->voisins.size(); j++)
            if(this->voisins[j] == rank_src)
            {
                deja_voisin = true;
                break;
            }

        if( ! deja_voisin)
            this->voisins.push_back(rank_src);
    }
    break;
    // ---------------------------------------------------------------------------------------
    case TERMINER:
    {
        return false;
    }
    break;
    default:
        std::cerr << "-- wait_request : no switch match MPI_TAG" << std::endl;
    }

    return true;
}
