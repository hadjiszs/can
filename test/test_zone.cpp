#include "catch.hpp"
#include "utils.hpp"

#include <iostream>

TEST_CASE("Test des zones", "[zone]")
{
    SECTION("Test si une zone contient un point")
    {
        Zone zone;

        zone.max = Point(10, 4);
        zone.min = Point(8,3);

        REQUIRE(zone.contient(Point(8, 3.5)));
    }

    SECTION("Test si une zone se divise bien")
    {
        Zone zone;

        zone.max = Point(10, 10);
        zone.min = Point(0, 0);

        Point R(4, 3);
        Point I(4, 8);

        std::pair<Zone, Zone> pair_zone = zone.diviser(R); // first zone contient R

        REQUIRE(pair_zone.first  == Zone(Point(5 , 10), Point(0, 0)));
        REQUIRE(pair_zone.second == Zone(Point(10, 10), Point(5, 0)));
        REQUIRE(pair_zone.second == Zone(Point(10, 10), Point(5, 0)));
        REQUIRE(pair_zone.second.contient(I) == false);

        I = pair_zone.second.random_coord();

        REQUIRE(pair_zone.second.contient(I) == true);
    }

    SECTION("Test si une zone est voisin parfait d'une autre")
    {
        Zone zone_a;
        Zone zone_b;

        zone_a.min = Point(3, 7);
        zone_a.max = Point(9, 11);

        zone_b.min = Point(3, 3);
        zone_b.max = Point(9, 7);
        REQUIRE(zone_a.voisin_parfait(zone_b) == true);

        zone_b.min = Point(9, 7);
        zone_b.max = Point(14, 11);
        REQUIRE(zone_a.voisin_parfait(zone_b) == true);

        zone_b.min = Point(3, 11);
        zone_b.max = Point(9, 15);
        REQUIRE(zone_a.voisin_parfait(zone_b) == true);

        zone_b.min = Point(0, 7);
        zone_b.max = Point(3, 11);
        REQUIRE(zone_a.voisin_parfait(zone_b) == true);

        zone_b.min = Point(4, 3);
        zone_b.max = Point(9, 7);
        REQUIRE(zone_a.voisin_parfait(zone_b) == false);

        zone_b.min = Point(0, 7);
        zone_b.max = Point(3, 9);
        REQUIRE(zone_a.voisin_parfait(zone_b) == false);

        zone_b.min = Point(3, 4);
        zone_b.max = Point(12, 7);
        REQUIRE(zone_a.voisin_parfait(zone_b) == false);
    }

    SECTION("Test de fusion de deux zones")
    {
        Zone zone_a;
        Zone zone_b;

        zone_a.min = Point(3, 7);
        zone_a.max = Point(9, 11);

        zone_b.min = Point(3, 3);
        zone_b.max = Point(9, 7);

        Zone res;
        res.min = Point(3, 3);
        res.max = Point(9, 11);

        REQUIRE(zone_a.fusion(zone_b) == res);

        // ---
        res.min = Point(3, 7);
        res.max = Point(14, 11);

        zone_b.min = Point(9, 7);
        zone_b.max = Point(14, 11);
        REQUIRE(zone_a.fusion(zone_b) == res);

        // ---
        res.min = Point(3, 7);
        res.max = Point(9, 15);

        zone_b.min = Point(3, 11);
        zone_b.max = Point(9, 15);
        REQUIRE(zone_a.fusion(zone_b) == res);
    }

    SECTION("Test si une zone est voisine d'une autre")
    {
        Zone zone_a;
        Zone zone_b;

        zone_a.min = Point(3, 7);
        zone_a.max = Point(9, 11);

        // -- bas
        zone_b.min = Point(3, 3);
        zone_b.max = Point(9, 7);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(4, 3);
        zone_b.max = Point(9, 7);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(4, 3);
        zone_b.max = Point(11, 7);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(4, 3);
        zone_b.max = Point(7, 7);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(1, 3);
        zone_b.max = Point(7, 7);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // -- droite
        zone_b.min = Point(9, 5);
        zone_b.max = Point(13, 9);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(9, 7);
        zone_b.max = Point(13, 9);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(9, 7);
        zone_b.max = Point(13, 11);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(9, 8);
        zone_b.max = Point(13, 10);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(9, 8);
        zone_b.max = Point(13, 13);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // -- haut
        zone_b.min = Point(3, 11);
        zone_b.max = Point(9, 15);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(4, 11);
        zone_b.max = Point(9, 15);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(4, 11);
        zone_b.max = Point(11, 15);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(4, 11);
        zone_b.max = Point(7, 15);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(1, 11);
        zone_b.max = Point(7, 15);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // -- gauche
        zone_b.min = Point(1, 8);
        zone_b.max = Point(3, 13);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(1, 7);
        zone_b.max = Point(3, 13);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(1, 11);
        zone_b.max = Point(3, 13);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(1, 11);
        zone_b.max = Point(3, 12);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_b.min = Point(1, 10);
        zone_b.max = Point(3, 14);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // -- faux
        zone_b.min = Point(12, 11);
        zone_b.max = Point(15, 15);
        REQUIRE(zone_a.est_voisin(zone_b) == false);

        zone_b.min = Point(13, 10);
        zone_b.max = Point(15, 15);
        REQUIRE(zone_a.est_voisin(zone_b) == false);

        zone_b.min = Point(1, 6);
        zone_b.max = Point(3, 8);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        zone_b.min = Point(10, 8);
        zone_b.max = Point(15, 11);
        REQUIRE(zone_a.est_voisin(zone_b) == false);

        zone_b.min = Point(9, 4);
        zone_b.max = Point(13, 7);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

        // --
        zone_a.min = Point(500, 250);
        zone_a.max = Point(750, 500);

        zone_b.min = Point(500, 500);
        zone_b.max = Point(1000, 1000);
        REQUIRE(zone_a.est_voisin(zone_b) == true);

    }
}
