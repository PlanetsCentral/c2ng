/**
  *  \file u/t_game_map_locker.cpp
  *  \brief Test for game::map::Locker
  */

#include "game/map/locker.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/map/drawing.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"

namespace {
    using game::Reference;
    using game::map::Configuration;
    using game::map::Drawing;
    using game::map::Explosion;
    using game::map::Locker;
    using game::map::Minefield;
    using game::map::Planet;
    using game::map::Point;
    using game::map::Ship;
    using game::map::Ufo;
    using game::map::Universe;

    void createPlanet(Universe& univ, int id, Point pt)
    {
        Planet* p = univ.planets().create(id);
        p->setPosition(pt);

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p->internalCheck(Configuration(), tx, log);
    }

    void createShip(Universe& univ, int id, Point pt)
    {
        Ship* p = univ.ships().create(id);
        p->addShipXYData(pt, 1, 100, game::PlayerSet_t(1));
        p->internalCheck();
    }

    void createUfo(Universe& univ, int id, Point pt)
    {
        Ufo* p = univ.ufos().addUfo(id, 7, 1);
        TS_ASSERT(p);
        p->setPosition(pt);
        p->setRadius(5);
    }
}

/** Test addPoint().
    A: call addPoint() several times.
    E: correct point is chosen */
void
TestGameMapLocker::testPoint()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    t.addPoint(Point(1010, 1000), true);
    t.addPoint(Point(1000, 1010), true);
    t.addPoint(Point(1005, 1005), true);
    t.addPoint(Point( 990, 1000), true);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(1005, 1005));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference());
}

/** Test addPoint() with setRangeLimit().
    A: call setRangeLimit(); call addPoint() several times.
    E: correct point is chosen */
void
TestGameMapLocker::testPointLimit()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    t.setRangeLimit(Point(900, 900), Point(1004, 1004));
    t.addPoint(Point(1010, 1000), true);
    t.addPoint(Point(1000, 1010), true);
    t.addPoint(Point(1005, 1005), true);
    t.addPoint(Point( 990, 1000), true);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(990, 1000));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference());
}

/** Test null behaviour.
    A: do not add any points.
    E: original position is returned. */
void
TestGameMapLocker::testNull()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(1000, 1000));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference());
}

/** Test filtering behaviour.
    A: setMarkedOnly(); add some points.
    E: only marked position is returned. */
void
TestGameMapLocker::testFiltered()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    t.setMarkedOnly(true);
    t.addPoint(Point(1010, 1000), false);
    t.addPoint(Point(1000, 1010), true);
    t.addPoint(Point(1005, 1005), false);
    t.addPoint(Point( 990, 1000), false);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(1000, 1010));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference());
}

/** Test locking on planets.
    A: create some planets.
    E: correct position and object returned. */
void
TestGameMapLocker::testPlanets()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    createPlanet(u, 50, Point(1020, 1000));
    createPlanet(u, 52, Point(1000, 1019));
    createPlanet(u, 54, Point(1000, 1021));

    t.addUniverse(u, -1, 0);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(1000, 1019));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference(Reference::Planet, 52));
}

/** Test locking on ships.
    A: create some ships.
    E: correct position and object returned. */
void
TestGameMapLocker::testShips()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    createShip(u, 70, Point(1020, 1000));
    createShip(u, 72, Point(1000, 1019));
    createShip(u, 74, Point(1000, 1021));

    t.addUniverse(u, -1, 0);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(1000, 1019));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference(Reference::Ship, 72));
}

/** Test locking on Ufos.
    A: create some Ufos.
    E: correct position and object returned. */
void
TestGameMapLocker::testUfos()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    createUfo(u, 10, Point(1010, 1000));
    createUfo(u, 11, Point( 995, 1005));
    createUfo(u, 12, Point(1001, 1009));

    t.addUniverse(u, -1, 0);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(995, 1005));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference(Reference::Ufo, 11));
}

/** Test locking on minefields.
    A: create some minefields.
    E: correct position and object returned. */
void
TestGameMapLocker::testMinefields()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    u.minefields().create(1)->addReport(Point(1010, 1000), 1, Minefield::IsMine, Minefield::UnitsKnown, 50, 1, Minefield::MinefieldScanned);
    u.minefields().create(5)->addReport(Point(1005,  995), 1, Minefield::IsMine, Minefield::UnitsKnown, 50, 1, Minefield::MinefieldScanned);
    u.minefields().create(8)->addReport(Point(1000, 1010), 1, Minefield::IsMine, Minefield::UnitsKnown, 50, 1, Minefield::MinefieldScanned);

    t.addUniverse(u, -1, 0);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(1005, 995));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference(Reference::Minefield, 5));
}

/** Test locking on drawings.
    A: create some drawings.
    E: correct position returned. */
void
TestGameMapLocker::testDrawings()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    u.drawings().addNew(new Drawing(Point(990, 1000), Drawing::MarkerDrawing));
    u.drawings().addNew(new Drawing(Point(995, 1000), Drawing::CircleDrawing));    // ignored by Locker
    u.drawings().addNew(new Drawing(Point(1020, 1000), Drawing::MarkerDrawing));

    t.addUniverse(u, -1, 0);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(990, 1000));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference());
}

/** Test locking on explosions.
    A: create some explosions.
    E: correct position returned. */
void
TestGameMapLocker::testExplosions()
{
    Configuration fig;
    Locker t(Point(1000, 1000), fig);

    Universe u;
    u.explosions().add(Explosion(1, Point(990, 1000)));
    u.explosions().add(Explosion(2, Point(995, 1000)));
    u.explosions().add(Explosion(3, Point(1020, 1000)));

    // Explosions are considered drawings
    t.addDrawings(u, 0);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(995, 1000));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference());
}

/** Test locking with wrapped map.
    A: create wrapped map. Add some points; closest is across the seam.
    E: point across the seam returned; mapped to be near clicked point. */
void
TestGameMapLocker::testWrap()
{
    Configuration fig;
    fig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    Locker t(Point(1010, 1010), fig);

    t.addPoint(Point(1200, 1200), true);
    t.addPoint(Point(2900, 2950), true);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(900, 950));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference());
}

/** Test locking with circular wrap.
    A: create circular map. Add some points; closest is across the seam.
    E: point across the seam returned. */
void
TestGameMapLocker::testCircular()
{
    Configuration fig;
    fig.setConfiguration(Configuration::Circular, Point(2000, 2000), Point(1000, 1000));

    Locker t(Point(2000, 990), fig);

    t.addPoint(Point(2000, 800), true);
    t.addPoint(Point(2000, 2950), true);

    TS_ASSERT_EQUALS(t.getFoundPoint(), Point(2000, 950));
    TS_ASSERT_EQUALS(t.getFoundObject(), Reference());

}

