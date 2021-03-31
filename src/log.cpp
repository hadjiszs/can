#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>

#include "log.hpp"

void log_svg(const char* filename, Node node, int i)
{
    // ----
    // Log
    std::ostringstream oss;
    oss << filename << "_" << i << ".svg";

    FILE *f = fopen(oss.str().c_str(), "w+");
    if(f == NULL)
        std::cerr << "Erreur ouverture fichier: " << oss.str() << std::endl;

    unsigned int mult = 4;
    fprintf(f, "<?xml version=\"1.0\" encoding=\"utf-8\"?> \n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"%d\" height=\"%d\"> \n", MAX * mult, MAX * mult);

    for(int j(1); j <= i; j++)
    {
        if(Node::is_rm(j))
            continue;

        Zone zone = node.get_zone(j);
        unsigned int width  = zone.max.x - zone.min.x;
        unsigned int height = zone.max.y - zone.min.y;

        int milieu_x, milieu_y;
        milieu_x = zone.min.x + (width/2);
        milieu_y = MAX-zone.max.y + (height/2);

        fprintf(f, " <rect width=\"%u\" height=\"%u\" x=\"%u\" y=\"%u\" style=\"fill:rgb(150,100,100);stroke-width:5;stroke:rgb(255,255,255)\"  /> \n",
                width * mult, height * mult,
                (unsigned int)zone.min.x * mult,
                (unsigned int)(MAX-zone.max.y) * mult);
    }

    for(int j(1); j <= i; j++)
    {
        if(Node::is_rm(j))
            continue;

        std::vector<Couple> data = node.get_data(j);

        for(unsigned int k(0); k < data.size(); k++)
            fprintf(f, "<circle cx=\"%u\" cy=\"%u\" r=\"10\" fill=\"#76C36A\" /> \n",
                    (unsigned int)data[k].position.x * mult ,
                    (unsigned int)(MAX-data[k].position.y) * mult);
    }

    for(int j(1); j <= i; j++)
    {
        if(Node::is_rm(j))
            continue;

        Point key = node.get_key(j);
        Zone zone = node.get_zone(j);

        unsigned int width  = zone.max.x - zone.min.x;
        unsigned int height = zone.max.y - zone.min.y;

        int milieu_x, milieu_y;
        milieu_x = zone.min.x + (width/2);
        milieu_y = MAX-zone.max.y + (height/2);

        // width et height au centre
        fprintf(f, "<text style=\" font-size: %d; font-weight: bold;\" x=\"%d\" y=\"%d\" fill=\"#F0F0F0\"  font-weight=\"900\"  font-family=\"monospace\" text-anchor=\"middle\">", mult*12, milieu_x * mult , milieu_y * mult);
        fprintf(f, "w:%d h:%d", width, height);
        fprintf(f, "</text>\n");

        bool print_max_min = false;
        if(print_max_min)
        {
            // point max
            fprintf(f, "<text style=\" font-size: %d; font-weight: bold;\" x=\"%d\" y=\"%d\" fill=\"#F0F0F0\"  font-weight=\"900\"  font-family=\"monospace\" text-anchor=\"middle\">", mult*12,
                    milieu_x * mult,
                    milieu_y * mult + 15*mult);
            fprintf(f, "max:{%.1lf, %.1lf}", zone.max.x, zone.max.y);
            fprintf(f, "</text>\n");

            // point min
            fprintf(f, "<text style=\" font-size: %d; font-weight: bold;\" x=\"%d\" y=\"%d\" fill=\"#F0F0F0\"  font-weight=\"900\"  font-family=\"monospace\" text-anchor=\"middle\">", mult*12,
                    milieu_x * mult,
                    milieu_y * mult + 30*mult);
            fprintf(f, "min:{%.1lf, %.1lf}", zone.min.x, zone.min.y);
            fprintf(f, "</text>\n");
        }

        // clef
        fprintf(f, "<text style=\" font-size: %d; font-weight: bold;\" x=\"%d\" y=\"%d\" fill=\"#000000\"  font-weight=\"900\"  font-family=\"monospace\" text-anchor=\"middle\">", mult*12, (unsigned int)key.x * mult , (unsigned int)(MAX-key.y) * mult);
        fprintf(f, "%d", j);
        fprintf(f, "</text>\n");

        fprintf(f, "<text style=\" font-size: %d; font-weight: bold;\" x=\"%d\" y=\"%d\" fill=\"#000000\" font-weight=\"900\"  font-family=\"monospace\" text-anchor=\"middle\">", mult*12, (unsigned int)key.x * mult , (unsigned int)((MAX-key.y) * mult)+15*mult);
        fprintf(f, "{%.1lf, %.1lf}", key.x, key.y);
        fprintf(f, "</text>\n");
    }

    fprintf(f, "</svg> \n");
    fclose(f);
}

void log_txt(const char* filename, Node node, int i)
{
    // ---
    // Log
    std::ostringstream oss;
    oss << filename << "_" << i << ".txt";

    std::ofstream ofs;
    ofs.open(oss.str().c_str(), std::ofstream::out);

    if(ofs.bad())
        std::cerr << "Erreur ouverture fichier: " << oss.str() << std::endl;

    ofs << std::endl;
    ofs.fill('#');
    ofs << std::setw(85) << "#";
    ofs.fill(' ');
    ofs << std::endl;

    ofs << std::left;
    ofs << std::setw(18) << "PROCESSUS";
    ofs << std::setw(40) << "CLEF";
    ofs <<  "ZONE";

    ofs << std::endl;
    ofs.fill('#');
    ofs << std::setw(85) << "#";
    ofs.fill(' ');
    ofs << std::endl;
    ofs << std::right << std::endl;

    for(int j(1); j <= i; j++)
    {
        if(Node::is_rm(j))
            continue;

        Point key = node.get_key(j);
        Zone zone = node.get_zone(j);
        std::vector<Couple> data = node.get_data(j);

        ofs << "[" << std::setw(2) << j << "]";
        ofs << std::setw(10) << key << std::setw(15) << zone << std::endl;

        // Affichage des données
        if(data.size() == 0)
            ofs << "Pas encore de donnée" << std::endl;
        else
        {
            ofs << "DONNEE:" << std::endl;
            for(unsigned int k(0); k < data.size(); k++)
                ofs << data[k] << std::endl;
        }

        ofs << std::endl;
        ofs.fill('#');
        ofs << std::setw(85) << "#";
        ofs.fill(' ');
        ofs << std::endl;
    }

    ofs.close();
}
