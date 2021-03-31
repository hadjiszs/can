#ifndef _UTILS_
#define _UTILS_

#include <utility>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iomanip>
#include <mpi.h>

#define MAX       1000

#define COORDINATEUR 0
#define BOOTSTRAP    1

#define KILL        27
#define INVITATION  57
#define JOIN        67
#define INSERT      77
#define NEWDATA     87

#define GET_KEY    100
#define GET_ZONE   110
#define GET_DATA   120
#define GET_VALEUR 130
#define GET_STATUS 140
#define SEARCH_VALEUR 150

#define RM_ME      200
#define ADD_ME     210

#define SET_DATA   300

#define TERMINER   444

struct Point {
    double x;
    double y;

    double val;

    Point()
        : x(0), y(0)
    { }

    Point(double xx, double yy)
        : x(xx), y(yy)
    { val = x + y; }

    friend std::ostream& operator<<(std::ostream& flux, const Point & p)
    {
        flux << "{ " << std::setw(4) << p.x;
        flux << " ; ";
        flux << std::setw(4) << p.y;
        flux << " } ";

        return flux;
    }

    static double distance(Point a, Point b)
    {
        return sqrt( (b.x-a.x) * (b.x-a.x) +
                     (b.y-a.y) * (b.y-a.y) );
    }

    friend bool operator==(const Point & a, const Point & b)
    {
        return a.x == b.x && a.y == b.y;
    }
};

struct Couple {
    Point position;
    int valeur;

    Couple()
    {
        valeur = position.x + position.y;
    }

    Couple(Point p)
        : position(p)
    {
        valeur = (unsigned int)p.x + (unsigned int)p.y;
    }

    friend std::ostream& operator<<(std::ostream& flux, const Couple & c)
    {
        flux << c.position << "[" << c.valeur << "]";

        return flux;
    }
};

struct Zone {
    Point max;
    Point min;

    Zone(Point mmax = Point(), Point mmin = Point())
        : max(mmax), min(mmin)
    { }

    friend std::ostream& operator<<(std::ostream& flux, const Zone & z)
    {
        flux << "[ min: " << z.min << " | max: " << z.max << " ] " << std::endl;

        return flux;
    }

    friend bool operator==(const Zone & a, const Zone & b)
    {
        return a.max == b.max && a.min == b.min;
    }

    bool contient(Point a)
    {
        if(a.x >= max.x || a.y >= max.y)
            return false;

        if(a.x < min.x || a.y < min.y)
            return false;

        return true;
    }

    bool est_voisin(Zone zb)
    {
        Point a = Point(this->min.x, this->min.y);
        Point b = Point(zb.min.x, zb.min.y);

        double a_w = this->max.x - this->min.x;
        double b_w = zb.max.x - zb.min.x;

        double a_h = this->max.y - this->min.y;
        double b_h = zb.max.y - zb.min.y;

        if( (b.x > a.x + a_w) ||
            (b.x + b_w < a.x) ||
            (b.y > a.y + a_h) ||
            (b.y + b_h < a.y) )
            return false;
        else
            return true;

        return true;

    }

    // le first de la paire retourné contient le point p
    std::pair<Zone, Zone> diviser(Point R)
    {
        Zone partA = *this;
        Zone partB = *this;

        double largeur = fabs(max.x) - fabs(min.x);
        double hauteur = fabs(max.y) - fabs(min.y);

        if(largeur >= hauteur)
            partA.max.x = partB.min.x = partA.min.x + largeur/2.0f; // division en largeur
        else
            partA.max.y = partB.min.y = partA.min.y + hauteur/2.0f; // division en hauteur

        if(partA.contient(R))
            return std::make_pair(partA, partB);
        else
            return std::make_pair(partB, partA);
    }

    Point random_coord()
    {
        Point key;

        double largeur = fabs(max.x) - fabs(min.x);
        double hauteur = fabs(max.y) - fabs(min.y);

        int i_larg = floor(largeur);
        int i_haut = floor(hauteur);

        double d_larg = largeur - i_larg;
        double d_haut = hauteur - i_haut;

        if(i_larg == 0 || i_haut == 0)
            std::cout << "attention div par 0 : " << *this
                      << "\nlargeur: " << largeur
                      << "\nhauteur: " << hauteur <<
                std::endl;

        if(i_larg > 1)
            key.x = min.x + (rand() % (i_larg-1)) + d_larg;
        else
            key.x = min.x + d_larg;

        if(i_haut > 1)
            key.y = min.y + (rand() % (i_haut-1)) + d_haut;
        else
            key.y = min.y + d_haut;

        return key;
    }

    bool voisin_parfait(Zone z, int* cas = NULL)
    {
        bool res = false;

        //     3
        //   A---B
        // 4 |   | 2
        //   D---C
        //     1

        Point A(min.x, max.y);        Point Az(z.min.x, z.max.y);
        Point B(max.x, max.y);        Point Bz(z.max.x, z.max.y);
        Point C(max.x, min.y);        Point Cz(z.max.x, z.min.y);
        Point D(min.x, min.y);        Point Dz(z.min.x, z.min.y);

        if(Az == D && Bz == C)
        {
            if(cas != NULL)
                *cas = 1;
            res = true;
        }

        if(Az == B && Dz == C)
        {
            if(cas != NULL)
                *cas = 2;
            res = true;
        }

        if(A == Dz && B == Cz)
        {
            if(cas != NULL)
                *cas = 3;
            res = true;
        }

        if(A == Bz && D == Cz)
        {
            if(cas != NULL)
                *cas = 4;
            res = true;
        }

        return res;
    }

    Zone fusion(Zone z)
    {
        Zone res;
        int cas = 0;
        this->voisin_parfait(z, &cas);

        Point A(min.x, max.y);        Point Az(z.min.x, z.max.y);
        Point B(max.x, max.y);        Point Bz(z.max.x, z.max.y);
        Point C(max.x, min.y);        Point Cz(z.max.x, z.min.y);
        Point D(min.x, min.y);        Point Dz(z.min.x, z.min.y);

        if(cas == 1)
        {
            res.min = Dz;
            res.max = B;
        }
        else if(cas == 2)
        {
            res.min = D;
            res.max = Bz;
        }
        else if(cas == 3)
        {
            res.min = D;
            res.max = Bz;
        }
        else if(cas == 4)
        {
            res.min = Dz;
            res.max = B;
        }

        return res;
    }

};

struct Node {
    int rank;
    Point  key;
    Zone   chunk;

    std::vector<int> voisins;
    std::vector<Couple> data;

    bool enable;

    static bool is_rm(int rank_dest)
    {
        bool is_enable;
        MPI_Send(NULL, 0, MPI_CHAR, rank_dest, GET_STATUS, MPI_COMM_WORLD);

        MPI_Recv(&is_enable, sizeof(bool), MPI_BYTE, rank_dest, GET_STATUS+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return !is_enable;
    }

    Node()
    {
        enable = true;
        chunk.min = Point(0, 0);
        chunk.max = Point(MAX, MAX);

        compute_alea_coord();
    }

    // le first de la paire est la zone contenant la clef du node courant
    std::pair<Zone, Zone> diviser()
    { return this->chunk.diviser(this->key); }

    void compute_alea_coord()
    { key = chunk.random_coord(); }

    bool contient(Point a)
    { return chunk.contient(a); }

    friend std::ostream& operator<<(std::ostream& flux, const Node & n)
    {
        flux << "\n\tcle  :: " << n.key
             << "\n\trank :: " << n.rank << "\n"
             << n.chunk << "\t"
             << (n.voisins.size() == 0 ? "[PAS DE VOISINS]" : "voisins :: ");

        for(unsigned int i(0); i < n.voisins.size(); i++)
            flux << " " << n.voisins[i];

        flux << std::endl;

        return flux;
    }

    template<int TAG> int send(int rank_dest, Couple data = Point());
    template<int TAG> bool recv(int rank_src);
    template<int TAG> bool ack(int rank_dest);

    Point get_key(int rank_dest)
    {
        Point res;
        MPI_Send(NULL, 0, MPI_CHAR, rank_dest, GET_KEY, MPI_COMM_WORLD);

        MPI_Recv(&res, sizeof(Point), MPI_BYTE, rank_dest, GET_KEY+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return res;
    }

    Zone get_zone(int rank_dest)
    {
        Zone res;
        MPI_Send(NULL, 0, MPI_CHAR, rank_dest, GET_ZONE, MPI_COMM_WORLD);

        MPI_Recv(&res, sizeof(Zone), MPI_BYTE, rank_dest, GET_ZONE+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return res;
    }

    std::vector<Couple> get_data(int rank_dest)
    {
        std::vector<Couple> res;

        // Demande du nombre de donnee
        MPI_Send(NULL, 0, MPI_CHAR, rank_dest, GET_DATA, MPI_COMM_WORLD);

        // Recepetion de ce nombre
        std::size_t nb_donnee = 5;
        MPI_Recv(&nb_donnee, sizeof(std::size_t), MPI_BYTE, rank_dest, GET_DATA+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        res.resize(nb_donnee);
        MPI_Recv(res.data(), nb_donnee*sizeof(Couple), MPI_BYTE, rank_dest, GET_DATA+2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return res;
    }

    int get_val(int rank_dest, Point pos)
    {
        int res = 0;

        // Envoie de l'ordre de recuperation de valeur
        MPI_Send(&pos, sizeof(Point), MPI_BYTE, rank_dest, GET_VALEUR, MPI_COMM_WORLD);

        // Attente de la fin d'insertion
        MPI_Recv(&res, sizeof(int), MPI_BYTE, rank_dest, GET_VALEUR+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        return res;
    }

    int recherche_valeur(Point pos)
    {
        for(unsigned int i(0); i < this->data.size(); i++)
            if(pos == this->data[i].position)
                return this->data[i].valeur;

        return -1;
    }

    bool wait_request(int rank_src = MPI_ANY_SOURCE, int tag = MPI_ANY_TAG);
};

#include "utils.tcc"

#endif
