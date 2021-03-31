// Spécialisation pour la requete JOIN
template<>
inline
int Node::send<INVITATION>(int rank_dest, Couple couple_donnee)
{
    // Envoie de l'invitation
    MPI_Send(NULL, 0, MPI_CHAR, rank_dest, INVITATION, MPI_COMM_WORLD);

    // Attente de la fin d'insertion
    MPI_Recv(NULL, 0, MPI_CHAR, rank_dest, INVITATION+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return 0;
}

template<>
inline
int Node::send<KILL>(int rank_dest, Couple couple_donnee)
{
    // Envoie de l'invitation
    MPI_Send(NULL, 0, MPI_CHAR, rank_dest, KILL, MPI_COMM_WORLD);

    // Attente de la fin d'insertion
    MPI_Recv(NULL, 0, MPI_CHAR, rank_dest, KILL+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return 0;
}

template<>
inline
int Node::send<JOIN>(int rank_dest, Couple couple_donnee)
{
    MPI_Status status;

    // ----
    // Envoie de la demande
    std::cerr << "[" << this->rank << "] : Envoie de demande" << std::endl;

    // Envoie de la clef de destination
    MPI_Send(&this->key,  sizeof(Point), MPI_BYTE, rank_dest, JOIN, MPI_COMM_WORLD);

    // ----
    // Attente de réponse


    MPI_Probe(rank_dest, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if(status.MPI_TAG == JOIN+3)
    {
        // Le noeud destinateur ne peut pas m'accueillir, il m'a donc routé
        int target = 1;
        MPI_Recv(&target, sizeof(int), MPI_BYTE, rank_dest, JOIN+3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return this->send<JOIN>(target);
    }

    MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, JOIN+2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // ----
    // Reception des resultats de placement
    std::cerr << "[" << this->rank << "] : Attente de reception de resultats" << std::endl;

    // Reception de l'éventuelle nouvelle clef
    MPI_Recv(&this->key, sizeof(Point), MPI_BYTE, rank_dest, JOIN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Reception de la nouvelle zone
    MPI_Recv(&this->chunk, sizeof(Zone), MPI_BYTE, rank_dest, JOIN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Reception des voisins
    std::size_t nb_voisin;
    MPI_Recv(&nb_voisin, sizeof(std::size_t), MPI_BYTE, rank_dest, JOIN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    this->voisins.resize(nb_voisin);
    MPI_Recv(this->voisins.data(), nb_voisin*sizeof(int), MPI_BYTE, rank_dest, JOIN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::cerr << "-- Noeud I " << *this << std::endl;

    return 0;
}


// Spécialisation pour la requete JOIN
template<>
inline
int Node::send<INSERT>(int rank_dest, Couple couple_donnee)
{
    // Envoie de l'ordre d'insertion de donnée
    MPI_Send(&couple_donnee, sizeof(Couple), MPI_BYTE, rank_dest, INSERT, MPI_COMM_WORLD);

    std::cerr << couple_donnee << std::endl;
    // Attente de la fin d'insertion
    MPI_Recv(NULL, 0, MPI_CHAR, rank_dest, INSERT+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return 0;
}

template<>
inline
int Node::send<NEWDATA>(int rank_dest, Couple couple_donnee)
{
    MPI_Status status;

    // ----
    // Envoie de la demande
    std::cerr << "[" << this->rank << "] : Envoie de demande" << std::endl;

    // Envoie de la clef de destination
    MPI_Send(&couple_donnee, sizeof(Couple), MPI_BYTE, rank_dest, NEWDATA, MPI_COMM_WORLD);

    // ----
    // Attente de réponse
    MPI_Probe(rank_dest, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if(status.MPI_TAG == NEWDATA+3)
    {
        // Le noeud destinateur ne peut pas accueillir la donnée, il m'a donc routé
        int target = 1;
        MPI_Recv(&target, sizeof(int), MPI_BYTE, rank_dest, NEWDATA+3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return this->send<NEWDATA>(target, couple_donnee);
    }

    MPI_Recv(NULL, 0, MPI_CHAR, status.MPI_SOURCE, NEWDATA+2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return 0;
}

template<>
inline
int Node::send<SEARCH_VALEUR>(int rank_dest, Couple pos)
{
    MPI_Status status;
    int valeur = 0;
    // ----
    // Envoie de la demande
    std::cerr << "[" << this->rank << "] : Envoie de demande" << std::endl;

    // Envoie de la clef de destination
    MPI_Send(&pos.position, sizeof(Point), MPI_BYTE, rank_dest, SEARCH_VALEUR, MPI_COMM_WORLD);

    // ----
    // Attente de réponse
    MPI_Probe(rank_dest, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if(status.MPI_TAG == SEARCH_VALEUR+3)
    {
        // Le noeud destinateur ne peut pas accueillir la donnée, il m'a donc routé
        int target = 1;
        MPI_Recv(&target, sizeof(int), MPI_BYTE, rank_dest, SEARCH_VALEUR+3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return this->send<SEARCH_VALEUR>(target, pos.position);
    }

    MPI_Recv(&valeur, sizeof(int), MPI_BYTE, status.MPI_SOURCE, SEARCH_VALEUR+2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return valeur;
}

template<>
inline
bool Node::recv<SEARCH_VALEUR>(int rank_src)
{
    MPI_Status status;

    // Reception des coord du noeud requeteur
    Point pos;
    MPI_Recv(&pos, sizeof(Point), MPI_BYTE, rank_src, SEARCH_VALEUR, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::cerr << "Coordonnées de la valeur à chercher : " << pos << std::endl;

    // Si le noeud courant contient la coord_invite, c'est ok
    if(this->contient(pos))
    {
        int valeur = this->recherche_valeur(pos);

        MPI_Send(&valeur, sizeof(int), MPI_BYTE, rank_src, SEARCH_VALEUR+2, MPI_COMM_WORLD);
        return true;
    }

    // ----
    // Routage
    // Le noeud courant le contient pas la destination voulue par le noeud requeteur
    // -> On le délegue à un des noeuds voisins le plus proche

    std::cerr << "-- Routage" << std::endl;

    // Recherche du voisin le plus proche de la data
    std::vector< std::pair<int, double> > rank_to_dist;

    // Node à router au voisin le plus proche de la data si voisin présent
    for(unsigned int i(0); i < this->voisins.size(); i++)
    {
        if(this->voisins[i] == rank_src)
            continue;

        Zone zone_voisin = this->get_zone(this->voisins[i]);

        unsigned int width  = zone_voisin.max.x - zone_voisin.min.x;
        unsigned int height = zone_voisin.max.y - zone_voisin.min.y;

        Point milieu;
        milieu.x = zone_voisin.min.x + (width/2);
        milieu.y = zone_voisin.max.y - (height/2);

        rank_to_dist.push_back(std::make_pair(this->voisins[i], Point::distance(milieu, pos) ));
    }

    struct tmp {
        static bool comp(std::pair<int, double> a, std::pair<int, double> b)
        { return a.second < b.second; }
    };

    std::sort(rank_to_dist.begin(), rank_to_dist.end(), tmp::comp);

    int tab[100];
    int limite = 50;
    unsigned int indice_courant = 0;
    for(int i(0); i < 100; i++)
    {
        if(i >= limite)
        {
            limite += limite/2; // on ajoute la moitié du reste
            indice_courant++;

            if(indice_courant >= rank_to_dist.size())
                indice_courant = 0;
        }

        tab[i] = rank_to_dist[indice_courant].first;
    }
    int choisi = tab[(int)(rand() % 100)];

    // On delegue au voisin choisi
    std::cerr << "\n[" << this->rank << "] On delegue à: " << choisi << std::endl;
    MPI_Send(&choisi, sizeof(int), MPI_BYTE, rank_src, SEARCH_VALEUR+3, MPI_COMM_WORLD);

    return true;
}

template<>
inline
bool Node::recv<NEWDATA>(int rank_src)
{
    MPI_Status status;

    // Reception des coord du noeud requeteur
    Couple couple_donnee;
    MPI_Recv(&couple_donnee, sizeof(Couple), MPI_BYTE, rank_src, NEWDATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::cerr << "Couple à inserer : " << couple_donnee << std::endl;

    // Si le noeud courant contient la coord_invite, c'est ok
    if(this->contient(couple_donnee.position))
    {
        this->data.push_back(couple_donnee);

        MPI_Send(NULL, 0, MPI_CHAR, rank_src, NEWDATA+2, MPI_COMM_WORLD);
        return true;
    }

    // ----
    // Routage
    // Le noeud courant le contient pas la destination voulue par le noeud requeteur
    // -> On le délegue à un des noeuds voisins le plus proche

    std::cerr << "-- Routage" << std::endl;

    // Recherche du voisin le plus proche de la data
    std::vector< std::pair<int, double> > rank_to_dist;

    // Node à router au voisin le plus proche de la data si voisin présent
    for(unsigned int i(0); i < this->voisins.size(); i++)
    {
        if(this->voisins[i] == rank_src)
            continue;

        Zone zone_voisin = this->get_zone(this->voisins[i]);

        unsigned int width  = zone_voisin.max.x - zone_voisin.min.x;
        unsigned int height = zone_voisin.max.y - zone_voisin.min.y;

        Point milieu;
        milieu.x = zone_voisin.min.x + (width/2);
        milieu.y = zone_voisin.max.y - (height/2);

        rank_to_dist.push_back(std::make_pair(this->voisins[i], Point::distance(milieu, couple_donnee.position) ));
    }

    struct tmp {
        static bool comp(std::pair<int, double> a, std::pair<int, double> b)
        { return a.second < b.second; }
    };

    std::sort(rank_to_dist.begin(), rank_to_dist.end(), tmp::comp);

    int tab[100];
    int limite = 50;
    unsigned int indice_courant = 0;
    for(int i(0); i < 100; i++)
    {
        if(i >= limite)
        {
            limite += limite/2; // on ajoute la moitié du reste
            indice_courant++;

            if(indice_courant >= rank_to_dist.size())
                indice_courant = 0;
        }

        tab[i] = rank_to_dist[indice_courant].first;
    }
    int choisi = tab[(int)(rand() % 100)];

    // On delegue au voisin choisi
    std::cerr << "\n[" << this->rank << "] On delegue à: " << choisi << std::endl;
    MPI_Send(&choisi, sizeof(int), MPI_BYTE, rank_src, NEWDATA+3, MPI_COMM_WORLD);

    return true;
}

template<>
inline
bool Node::recv<JOIN>(int rank_src)
{
    if(rank_src == COORDINATEUR)
        return false;

    MPI_Status status;

    // Reception des coord du noeud requeteur
    Node guest_node;
    guest_node.rank = rank_src;
    MPI_Recv(&(guest_node.key), sizeof(Point), MPI_BYTE, rank_src, JOIN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    Point coord_invite = guest_node.key;

    std::cerr << "Point invité : " << coord_invite << std::endl;

    // Si le noeud courant contient la coord_invite, on divise
    if(this->contient(coord_invite))
    {
        // ----
        // Division du chunk du noeud courant

        std::pair<Zone, Zone> divise = this->diviser();

        this->chunk = divise.first;
        guest_node.chunk  = divise.second;
        guest_node.key    = coord_invite;

        std::cerr << "node R: " << *this << std::endl;
        if(! guest_node.contient(coord_invite))
            guest_node.compute_alea_coord();

        // ----
        // Mise à jour des voisins du noeud courant et du noeud requeteur
        std::vector<int> tab_remove; // voisin supprimé du noeud courant

        for(unsigned int i(0); i < this->voisins.size(); i++)
        {
            Zone chunk_voisin_courant = this->get_zone(voisins[i]);

            if(chunk_voisin_courant.est_voisin(guest_node.chunk))
            {
                guest_node.voisins.push_back(this->voisins[i]);
                std::cerr << "[" << guest_node.rank << "] : Ajout nouveau voisin du RESP: " << this->voisins[i] << std::endl;
                MPI_Send(&guest_node.rank, sizeof(int), MPI_BYTE, this->voisins[i], ADD_ME, MPI_COMM_WORLD);
            }
            else {
                std::cerr << "[" << guest_node.rank << "] : Je refuse d'ajouter le voisin: " << this->voisins[i]
                          << "\nzone:" << guest_node.rank << " :: " << guest_node.chunk
                          << "\nvoisin:" << voisins[i] << " :: " << chunk_voisin_courant << std::endl;

            }
        }

        for(unsigned int i(0); i < this->voisins.size(); i++)
        {
            Zone chunk_voisin_courant = this->get_zone(voisins[i]);

            if( ! chunk_voisin_courant.est_voisin(this->chunk))
            {
                std::cerr << "\n## [" << this->rank << "] Plus voisin avec " << voisins[i]
                          << "\nzone:" << this->rank << " :: " << this->chunk
                          << "\nvoisin:" << voisins[i] << " :: " << chunk_voisin_courant << std::endl;

                tab_remove.push_back(this->voisins[i]);
                MPI_Send(NULL, 0, MPI_CHAR, this->voisins[i], RM_ME, MPI_COMM_WORLD);
            }
        }

        // Suppression des voisins
        for(unsigned int i(0); i < tab_remove.size(); i++)
            for(unsigned int j(0); j < this->voisins.size(); j++)
                if(this->voisins[j] == tab_remove[i])
                    this->voisins.erase(this->voisins.begin()+j);


        // Ajout mutuel
        this->voisins.push_back(guest_node.rank);
        guest_node.voisins.push_back(this->rank);

        // Renseigne les voisins sur le nouvel agencement de leur voisinage

        // ----
        // Envoie des resultats au noeud requeteur

        MPI_Send(NULL, 0, MPI_CHAR, rank_src, JOIN+2, MPI_COMM_WORLD);

        // Envoie de l'éventuelle nouvelle clef
        MPI_Send(&(guest_node.key),   sizeof(Point), MPI_BYTE, rank_src, JOIN, MPI_COMM_WORLD);

        // Envoie de la zone calculée
        MPI_Send(&(guest_node.chunk), sizeof(Zone), MPI_BYTE, rank_src, JOIN, MPI_COMM_WORLD);

        // Envoie des voisins
        std::size_t nb_voisin = guest_node.voisins.size();
        MPI_Send(&nb_voisin, sizeof(std::size_t), MPI_BYTE, rank_src, JOIN, MPI_COMM_WORLD);
        MPI_Send(guest_node.voisins.data(), nb_voisin*sizeof(int), MPI_BYTE, rank_src, JOIN, MPI_COMM_WORLD);
        //sleep(1);
        std::cerr << "-- Noeud R : " << *this << std::endl;
        return true;
    }

    // ----
    // Routage
    // Le noeud courant le contient pas la destination voulue par le noeud requeteur
    // -> On le délegue à un des noeuds voisins le plus proche

    std::cerr << "-- Routage" << std::endl;

    // Recherche du voisin le plus proche de la coord_invite
    std::vector< std::pair<int, double> > rank_to_dist;

    // Node à router au voisin le plus proche de la coord_invite si voisin présent
    for(unsigned int i(0); i < this->voisins.size(); i++)
    {
        Zone zone_voisin = this->get_zone(this->voisins[i]);

        unsigned int width  = zone_voisin.max.x - zone_voisin.min.x;
        unsigned int height = zone_voisin.max.y - zone_voisin.min.y;

        Point milieu;
        milieu.x = zone_voisin.min.x + (width/2);
        milieu.y = zone_voisin.max.y - (height/2);

        rank_to_dist.push_back(std::make_pair(this->voisins[i], Point::distance(milieu, coord_invite) ));
    }

    struct tmp {
        static bool comp(std::pair<int, double> a, std::pair<int, double> b)
        { return a.second < b.second; }
    };

    std::sort(rank_to_dist.begin(), rank_to_dist.end(), tmp::comp);

    int tab[100];
    int limite = 50;
    unsigned int indice_courant = 0;

    for(int i(0); i < 100; i++)
    {
        if(i >= limite)
        {
            limite += limite/2; // on ajoute la moitié du reste
            indice_courant++;

            if(indice_courant >= rank_to_dist.size())
                indice_courant = 0;
        }

        tab[i] = rank_to_dist[indice_courant].first;
    }

    int choisi = tab[(int)(rand() % 100)];

    // On delegue au voisin choisi
    std::cerr << "\n[" << this->rank << "] On delegue à: " << choisi << std::endl;
    MPI_Send(&choisi, sizeof(int), MPI_BYTE, rank_src, JOIN+3, MPI_COMM_WORLD);

    return true;
}

template<int TAG>
inline
int
Node::send(int rank_dest, Couple couple_donnee)
{
    MPI_Send(NULL, 0, MPI_CHAR, rank_dest, TAG, MPI_COMM_WORLD);

    return 0;
}

template<int TAG>
inline
bool
Node::recv(int rank_src)
{
    MPI_Recv(NULL, 0, MPI_CHAR, rank_src, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return true;
}

template<int TAG>
inline
bool
Node::ack(int rank_dest)
{
    return send<TAG+1>(rank_dest);
}
