/**
  *  \file u/t_game_map_shippredictor.cpp
  *  \brief Test for game::map::ShipPredictor
  */

#include "game/map/shippredictor.hpp"

#include "t_game_map.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/universe.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace {
    using game::HostVersion;
    using game::map::Point;
    using game::map::Ship;
    using game::map::ShipPredictor;
    using game::Element;
    using game::config::HostConfiguration;

    const int X = 1200;
    const int Y = 1300;

    struct TestHarness {
        game::map::Universe univ;
        game::UnitScoreDefinitionList shipScores;
        game::spec::ShipList shipList;
        game::config::HostConfiguration config;
        game::HostVersion hostVersion;
        game::test::RegistrationKey key;

        TestHarness()
            : univ(), shipScores(), shipList(), config(), hostVersion(), key(game::RegistrationKey::Unknown, 6)
            { }
    };

    Ship& addEmerald(TestHarness& t, int shipId)
    {
        /*
         *  Test case: Emerald (from game "Schule", turn 61, ship #348)
         */
        const int HULL_ID = 61;
        const int ENGINE_ID = 6;

        // Emerald:
        game::spec::Hull& h = *t.shipList.hulls().create(HULL_ID);
        h.setMaxFuel(480);
        h.setMaxCargo(510);
        h.setMaxCrew(258);
        h.setNumEngines(2);
        h.setMass(218);        // we'll not add weapons; the plain hull only weighs 180 kt

        // HeavyNovaDrive 6:
        game::spec::Engine& e = *t.shipList.engines().create(ENGINE_ID);
        e.setFuelFactor(9, 72900);

        // Add a ship
        // - required properties
        Ship& s = *t.univ.ships().create(shipId);
        s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s.setOwner(1);
        s.setHull(HULL_ID);
        s.setEngineType(ENGINE_ID);
        s.setPosition(game::map::Point(X, Y));
        s.setWarpFactor(9);

        // - types and cargo need to be set to be able to compute a mass
        s.setBeamType(0);
        s.setNumBeams(0);
        s.setTorpedoType(0);
        s.setNumLaunchers(0);
        s.setNumBays(0);
        s.setCargo(Element::Neutronium, 100);
        s.setCargo(Element::Tritanium, 0);
        s.setCargo(Element::Duranium, 0);
        s.setCargo(Element::Molybdenum, 0);
        s.setCargo(Element::Supplies, 0);
        s.setCargo(Element::Money, 0);
        s.setCargo(Element::Colonists, 0);
        s.setAmmo(0);

        return s;
    }

    Ship& addMerlin(TestHarness& t, int shipId)
    {
        /*
         *  Test case: Merlin (from game "qvs0", turn 110, ship #2)
         */
        const int HULL_ID = 61;
        const int ENGINE_ID = 9;

        // Emerald:
        game::spec::Hull& h = *t.shipList.hulls().create(HULL_ID);
        h.setMaxFuel(450);
        h.setMaxCargo(2700);
        h.setMaxCrew(120);
        h.setNumEngines(10);
        h.setMass(928);        // we'll not add weapons; the plain hull only weighs 920 kt

        // Transwarp Drive:
        game::spec::Engine& e = *t.shipList.engines().create(ENGINE_ID);
        e.setFuelFactor(9, 8100);

        // Add a ship
        // - required properties
        Ship& s = *t.univ.ships().create(shipId);
        s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s.setOwner(1);
        s.setHull(HULL_ID);
        s.setEngineType(ENGINE_ID);
        s.setPosition(game::map::Point(X, Y));
        s.setWarpFactor(9);

        // - types and cargo need to be set to be able to compute a mass
        s.setBeamType(0);
        s.setNumBeams(0);
        s.setTorpedoType(0);
        s.setNumLaunchers(0);
        s.setNumBays(0);
        s.setCargo(Element::Neutronium, 100);
        s.setCargo(Element::Tritanium, 0);
        s.setCargo(Element::Duranium, 0);
        s.setCargo(Element::Molybdenum, 0);
        s.setCargo(Element::Supplies, 0);
        s.setCargo(Element::Money, 0);
        s.setCargo(Element::Colonists, 0);
        s.setAmmo(0);

        return s;
    }

    void finish(TestHarness& t)
    {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        t.univ.postprocess(game::PlayerSet_t::allUpTo(11),      // playingSet
                           game::PlayerSet_t::allUpTo(11),      // availablePlayers
                           game::map::Object::Editable,         // playability
                           t.hostVersion,                       // host
                           t.config,                            // config
                           42,                                  // turn
                           t.shipList,                          // shipList
                           tx,                                  // translator
                           log);                                // log
    }

    /* Canned test case: ship moving X light years burns Y kt fuel
       (THost distance anomaly) */
    void testFuelUsage(const char* label, int distance, int expected)
    {
        const int SHIP_ID = 348;

        TestHarness t;
        t.hostVersion = HostVersion(HostVersion::Host, MKVERSION(3,22,0));
        Ship& s = addEmerald(t, SHIP_ID);
        s.setWaypoint(Point(X + distance, Y));

        finish(t);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        testee.computeTurn();

        TSM_ASSERT_EQUALS(label, testee.getMovementFuelUsed(), expected);
        TSM_ASSERT_EQUALS(label, testee.getCloakFuelUsed(), 0);
        TSM_ASSERT_EQUALS(label, testee.getNumTurns(), 1);
        TSM_ASSERT_EQUALS(label, testee.isAtTurnLimit(), false);
    }

    /* Canned test case: ship having HAVE fuel needs NEED
       (PHost fuel consumption anomaly) */
    void testFuelUsagePHost(const char* label, int have, int need, HostVersion version)
    {
        const int SHIP_ID = 2;

        TestHarness t;
        t.hostVersion = version;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setWaypoint(Point(X + 75, Y + 34));
        s.setCargo(Element::Neutronium, have);
        t.config[HostConfiguration::UseAccurateFuelModel].set(true);

        finish(t);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        testee.computeTurn();

        TSM_ASSERT_EQUALS(label, testee.getMovementFuelUsed(), need);
        TSM_ASSERT_EQUALS(label, testee.getCloakFuelUsed(), 0);
        TSM_ASSERT_EQUALS(label, testee.getNumTurns(), 1);
        TSM_ASSERT_EQUALS(label, testee.isAtTurnLimit(), false);
    }

    /* Canned test case: alchemy friendly codes */
    void testAlchemy(const char* friendlyCode, int suppliesBefore, int tritaniumAfter, int duraniumAfter, int molybdenumAfter, int suppliesAfter, HostVersion host, bool expectAlchemy, bool expectFriendlyCode)
    {
        const int SHIP_ID = 59;

        afl::string::NullTranslator tx;
        String_t label = afl::string::Format("%s fc=%s s=%d", host.toString(tx), friendlyCode, suppliesBefore);

        TestHarness t;
        t.hostVersion = host;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 1);
        s.setCargo(Element::Tritanium, 10);
        s.setCargo(Element::Duranium, 20);
        s.setCargo(Element::Molybdenum, 30);
        s.setCargo(Element::Supplies, suppliesBefore);
        s.setFriendlyCode(String_t(friendlyCode));

        t.shipList.hulls().get(61)->changeHullFunction(t.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::HullFunction::MerlinAlchemy),
                                                       game::PlayerSet_t::fromInteger(-1), game::PlayerSet_t(), true);

        finish(t);

        game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, key);
        testee.computeTurn();

        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Tritanium),  tritaniumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Duranium),   duraniumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Molybdenum), molybdenumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Supplies),   suppliesAfter);
        TSM_ASSERT_EQUALS(label, testee.getUsedProperties().contains(ShipPredictor::UsedAlchemy), expectAlchemy);
        TSM_ASSERT_EQUALS(label, testee.getUsedProperties().contains(ShipPredictor::UsedFCode), expectFriendlyCode);
    }

    /* Canned test case: refinery friendly codes */
    void testRefinery(const char* friendlyCode, int suppliesBefore, int tritaniumAfter, int duraniumAfter, int molybdenumAfter, int suppliesAfter, int fuelAfter, HostVersion host, bool expectAlchemy, bool expectFriendlyCode)
    {
        const int SHIP_ID = 59;

        afl::string::NullTranslator tx;
        String_t label = afl::string::Format("%s fc=%s s=%d", host.toString(tx), friendlyCode, suppliesBefore);

        TestHarness t;
        t.hostVersion = host;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 1);
        s.setCargo(Element::Tritanium, 10);
        s.setCargo(Element::Duranium, 20);
        s.setCargo(Element::Molybdenum, 30);
        s.setCargo(Element::Supplies, suppliesBefore);
        s.setFriendlyCode(String_t(friendlyCode));

        t.shipList.hulls().get(61)->changeHullFunction(t.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::HullFunction::NeutronicRefinery),
                                                       game::PlayerSet_t::fromInteger(-1), game::PlayerSet_t(), true);

        finish(t);

        game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, key);
        testee.computeTurn();

        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Tritanium),  tritaniumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Duranium),   duraniumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Molybdenum), molybdenumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Supplies),   suppliesAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Neutronium), fuelAfter);
        TSM_ASSERT_EQUALS(label, testee.getUsedProperties().contains(ShipPredictor::UsedAlchemy), expectAlchemy);
        TSM_ASSERT_EQUALS(label, testee.getUsedProperties().contains(ShipPredictor::UsedFCode), expectFriendlyCode);
    }

    /* Canned test case: refinery friendly codes */
    void testAriesRefinery(const char* friendlyCode, int suppliesBefore, int tritaniumAfter, int duraniumAfter, int molybdenumAfter, int suppliesAfter, int fuelAfter, HostVersion host, bool expectAlchemy, bool expectFriendlyCode)
    {
        const int SHIP_ID = 59;

        afl::string::NullTranslator tx;
        String_t label = afl::string::Format("%s fc=%s s=%d", host.toString(tx), friendlyCode, suppliesBefore);

        TestHarness t;
        t.hostVersion = host;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 1);
        s.setCargo(Element::Tritanium, 10);
        s.setCargo(Element::Duranium, 20);
        s.setCargo(Element::Molybdenum, 30);
        s.setCargo(Element::Supplies, suppliesBefore);
        s.setFriendlyCode(String_t(friendlyCode));

        t.shipList.hulls().get(61)->changeHullFunction(t.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::HullFunction::AriesRefinery),
                                                       game::PlayerSet_t::fromInteger(-1), game::PlayerSet_t(), true);

        finish(t);

        game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, key);
        testee.computeTurn();

        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Tritanium),  tritaniumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Duranium),   duraniumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Molybdenum), molybdenumAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Supplies),   suppliesAfter);
        TSM_ASSERT_EQUALS(label, testee.getCargo(Element::Neutronium), fuelAfter);
        TSM_ASSERT_EQUALS(label, testee.getUsedProperties().contains(ShipPredictor::UsedAlchemy), expectAlchemy);
        TSM_ASSERT_EQUALS(label, testee.getUsedProperties().contains(ShipPredictor::UsedFCode), expectFriendlyCode);
    }

    void testMovement2(int waypointDX, int waypointDY, int warp, int movedDX, int movedDY, HostVersion host)
    {
        const int SHIP_ID = 77;

        afl::string::NullTranslator tx;
        String_t label = afl::string::Format("%s %d,%d", host.toString(tx), waypointDX, waypointDY);

        TestHarness t;
        t.hostVersion = host;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 1000);
        s.setWarpFactor(warp);
        s.setWaypoint(Point(X + waypointDX, Y + waypointDY));
        finish(t);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        testee.computeTurn();

        TSM_ASSERT_EQUALS(label, testee.getPosition().getX(), X + movedDX);
        TSM_ASSERT_EQUALS(label, testee.getPosition().getY(), Y + movedDY);
    }
}

/** Test error cases. ShipPredictor must not crash or hang. */
void
TestGameMapShipPredictor::testErrorCases()
{
    // Non-existant ship
    {
        TestHarness t;
        ShipPredictor p(t.univ, 99, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeMovement();
        p.computeTurn();
        TS_ASSERT_EQUALS(p.getNumTurns(), 0);
    }

    // Ship exists but hull doesn't.
    {
        const int SHIP_ID = 32;
        TestHarness t;
        Ship& s = *t.univ.ships().create(SHIP_ID);
        s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s.setOwner(1);
        s.setHull(77);
        s.setEngineType(7);
        s.setPosition(game::map::Point(1000, 1000));
        s.setWaypoint(game::map::Point(1200, 1200));
        s.setWarpFactor(9);

        {
            ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
            p.computeMovement();
        }
        {
            ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
            p.computeTurn();
        }
    }
}

/** Test fuel usage computation for THost.
    This checks the distance computation anomaly: a ship moving 3.00 ly burns the same
    amount of fuel as one moving 2.00 ly. */
void
TestGameMapShipPredictor::testFuelUsageHost()
{
    ::testFuelUsage("1 ly -> 2 kt",  1,  2);
    ::testFuelUsage("2 ly -> 5 kt",  2,  5);
    ::testFuelUsage("3 ly -> 5 kt",  3,  5);
    ::testFuelUsage("4 ly -> 11 kt", 4, 11);
}

/** Test fuel usage computation for PHost, UseAccurateFuelModel.
    This checks the fuel prediction anomaly: before 4.0e/3.4h, it was close to impossible
    to end up with 0 fuel. */
void
TestGameMapShipPredictor::testFuelUsagePHost()
{
    ::testFuelUsagePHost("79 old", 79, 78, HostVersion(HostVersion::PHost, MKVERSION(3,2,5)));
    ::testFuelUsagePHost("78 old", 78, 79, HostVersion(HostVersion::PHost, MKVERSION(3,2,5)));

    ::testFuelUsagePHost("79 new", 79, 78, HostVersion(HostVersion::PHost, MKVERSION(4,0,5)));
    ::testFuelUsagePHost("78 new", 78, 78, HostVersion(HostVersion::PHost, MKVERSION(4,0,5)));
}

/** Test multiple cases of alchemy. */
void
TestGameMapShipPredictor::testAlchemy()
{
    const HostVersion PHOST = HostVersion(HostVersion::PHost, MKVERSION(4,0,5));
    const HostVersion THOST = HostVersion(HostVersion::Host,  MKVERSION(3,22,47));

    // Normal
    ::testAlchemy("xyz", 900, 110, 120, 130,   0, PHOST, true,  false);
    ::testAlchemy("xyz", 900, 110, 120, 130,   0, THOST, true,  false);
    ::testAlchemy("xyz",  30,  13,  23,  33,   3, PHOST, true,  false);
    ::testAlchemy("xyz",  30,  13,  23,  33,   3, THOST, true,  false);
    ::testAlchemy("xyz",   0,  10,  20,  30,   0, PHOST, false, false);
    ::testAlchemy("xyz",   0,  10,  20,  30,   0, THOST, false, false);

    // NAL
    ::testAlchemy("NAL", 900,  10,  20,  30, 900, PHOST, false, true);
    ::testAlchemy("NAL", 900,  10,  20,  30, 900, THOST, false, true);

    // alX
    ::testAlchemy("alt", 900, 310,  20,  30,   0, PHOST, true,  true);
    ::testAlchemy("alt", 900, 310,  20,  30,   0, THOST, true,  true);
    ::testAlchemy("ald", 900,  10, 320,  30,   0, PHOST, true,  true);
    ::testAlchemy("ald", 900,  10, 320,  30,   0, THOST, true,  true);
    ::testAlchemy("alm", 900,  10,  20, 330,   0, PHOST, true,  true);
    ::testAlchemy("alm", 900,  10,  20, 330,   0, THOST, true,  true);

    ::testAlchemy("alt",  30,  20,  20,  30,   0, PHOST, true,  true);
    ::testAlchemy("alt",  30,  20,  20,  30,   0, THOST, true,  true);
    // ::testAlchemy("alt",  30,  19,  20,  30,   3, THOST, true,  true);

    // naX
    ::testAlchemy("nat", 900,  10, 170, 180,   0, PHOST, true,  true);
    ::testAlchemy("nat", 900, 110, 120, 130,   0, THOST, true,  false);
    ::testAlchemy("nad", 900, 160,  20, 180,   0, PHOST, true,  true);
    ::testAlchemy("nad", 900, 110, 120, 130,   0, THOST, true,  false);
    ::testAlchemy("nam", 900, 160, 170,  30,   0, PHOST, true,  true);
    ::testAlchemy("nam", 900, 110, 120, 130,   0, THOST, true,  false);
}

/** Test multiple cases of refinery. Note the PHost version dependency. */
void
TestGameMapShipPredictor::testRefinery()
{
    const HostVersion OLDPHOST = HostVersion(HostVersion::PHost, MKVERSION(4,0,5));
    const HostVersion PHOST    = HostVersion(HostVersion::PHost, MKVERSION(4,1,5));
    const HostVersion THOST    = HostVersion(HostVersion::Host,  MKVERSION(3,22,47));

    // Normal
    ::testRefinery("xyz", 900,  0,  0,  0, 840, 61, PHOST, true,  false);
    ::testRefinery("xyz", 900,  0,  0,  0, 840, 61, THOST, true,  false);
    ::testRefinery("xyz",  30,  0,  0, 30,   0, 31, PHOST, true,  false);
    ::testRefinery("xyz",  30,  0,  0, 30,   0, 31, THOST, true,  false);

    // NAL
    ::testRefinery("NAL", 900, 10, 20, 30, 900,  1, PHOST, false, true);
    ::testRefinery("NAL", 900, 10, 20, 30, 900,  1, THOST, false, true);

    // alX
    ::testRefinery("alt", 900,  0, 20, 30, 890, 11, PHOST, true,  true);
    ::testRefinery("ald", 900, 10,  0, 30, 880, 21, PHOST, true,  true);
    ::testRefinery("alm", 900, 10, 20,  0, 870, 31, PHOST, true,  true);

    ::testRefinery("alt", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false);
    ::testRefinery("ald", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false);
    ::testRefinery("alm", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false);

    ::testRefinery("alt", 900,  0,  0,  0, 840, 61, THOST, true, false);
    ::testRefinery("ald", 900,  0,  0,  0, 840, 61, THOST, true, false);
    ::testRefinery("alm", 900,  0,  0,  0, 840, 61, THOST, true, false);

    // naX
    ::testRefinery("nat", 900, 10,  0,  0, 850, 51, PHOST, true,  true);
    ::testRefinery("nad", 900,  0, 20,  0, 860, 41, PHOST, true,  true);
    ::testRefinery("nam", 900,  0,  0, 30, 870, 31, PHOST, true,  true);

    ::testRefinery("nat", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false);
    ::testRefinery("nad", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false);
    ::testRefinery("nam", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false);

    ::testRefinery("nat", 900,  0,  0,  0, 840, 61, THOST, true, false);
    ::testRefinery("nad", 900,  0,  0,  0, 840, 61, THOST, true, false);
    ::testRefinery("nam", 900,  0,  0,  0, 840, 61, THOST, true, false);
}

/** Test multiple cases of refinery. Note the PHost version dependency. */
void
TestGameMapShipPredictor::testAriesRefinery()
{
    const HostVersion OLDPHOST = HostVersion(HostVersion::PHost, MKVERSION(4,0,5));
    const HostVersion PHOST    = HostVersion(HostVersion::PHost, MKVERSION(4,1,5));
    const HostVersion THOST    = HostVersion(HostVersion::Host,  MKVERSION(3,22,47));

    // Normal
    ::testAriesRefinery("xyz", 40,  0,  0,  0,  40, 61, PHOST, true,  false);
    ::testAriesRefinery("xyz", 40,  0,  0,  0,  40, 61, THOST, true,  false);

    // NAL
    ::testAriesRefinery("NAL", 40, 10, 20, 30,  40,  1, PHOST, false, true);
    // ::testAriesRefinery("NAL", 40,  0,  0,  0,  40, 61, THOST, true,  false); <- FIXME: HOST does not permit NAL for Aries

    // alX
    ::testAriesRefinery("alt", 40,  0, 20, 30,  40, 11, PHOST, true,  true);
    ::testAriesRefinery("ald", 40, 10,  0, 30,  40, 21, PHOST, true,  true);
    ::testAriesRefinery("alm", 40, 10, 20,  0,  40, 31, PHOST, true,  true);

    ::testAriesRefinery("alt", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false);
    ::testAriesRefinery("ald", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false);
    ::testAriesRefinery("alm", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false);

    ::testAriesRefinery("alt", 40,  0,  0,  0,  40, 61, THOST, true, false);
    ::testAriesRefinery("ald", 40,  0,  0,  0,  40, 61, THOST, true, false);
    ::testAriesRefinery("alm", 40,  0,  0,  0,  40, 61, THOST, true, false);

    // naX
    ::testAriesRefinery("nat", 40, 10,  0,  0,  40, 51, PHOST, true,  true);
    ::testAriesRefinery("nad", 40,  0, 20,  0,  40, 41, PHOST, true,  true);
    ::testAriesRefinery("nam", 40,  0,  0, 30,  40, 31, PHOST, true,  true);

    ::testAriesRefinery("nat", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false);
    ::testAriesRefinery("nad", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false);
    ::testAriesRefinery("nam", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false);

    ::testAriesRefinery("nat", 40,  0,  0,  0,  40, 61, THOST, true, false);
    ::testAriesRefinery("nad", 40,  0,  0,  0,  40, 61, THOST, true, false);
    ::testAriesRefinery("nam", 40,  0,  0,  0,  40, 61, THOST, true, false);
}

/** Test multiple cases of movement. */
void
TestGameMapShipPredictor::testMovement()
{
    const int SHIP_ID = 42;

    // Base case
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 100);
        s.setWaypoint(Point(X + 15, Y));
        s.setWarpFactor(9);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeMovement();

        TS_ASSERT(!p.isAtTurnLimit());
        TS_ASSERT(p.isAtWaypoint());
        TS_ASSERT_EQUALS(p.getMovementFuelUsed(), 41);
    }

    // Timeout case (warp 1)
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 100);
        s.setWaypoint(Point(X + 100, Y));
        s.setWarpFactor(1);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeMovement();

        TS_ASSERT(p.isAtTurnLimit());
        TS_ASSERT(!p.isAtWaypoint());
        TS_ASSERT_EQUALS(p.getMovementFuelUsed(), 0);
        TS_ASSERT_EQUALS(p.getPosition(), Point(X + 30, Y));
    }

    // Timeout case (warp 0)
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 100);
        s.setWaypoint(Point(X + 100, Y));
        s.setWarpFactor(0);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeMovement();

        TS_ASSERT(p.isAtTurnLimit());
        TS_ASSERT(!p.isAtWaypoint());
        TS_ASSERT_EQUALS(p.getMovementFuelUsed(), 0);
        TS_ASSERT_EQUALS(p.getPosition(), Point(X, Y));
    }

    // Out of fuel
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 10);
        s.setWaypoint(Point(X + 15, Y));
        s.setWarpFactor(9);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeMovement();

        TS_ASSERT(!p.isAtTurnLimit());
        TS_ASSERT_EQUALS(p.getMovementFuelUsed(), 29);
        TS_ASSERT_EQUALS(p.getCargo(Element::Neutronium), 0);
    }

    // Out of fuel (2)
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 10);
        s.setWaypoint(Point(X + 15, Y));
        s.setWarpFactor(9);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeTurn();

        TS_ASSERT_EQUALS(p.getMovementFuelUsed(), 29);
        TS_ASSERT_EQUALS(p.getCargo(Element::Neutronium), -19);
    }

    // Training
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 10);
        s.setWaypoint(Point(X + 15, Y));
        s.setWarpFactor(9);
        s.setMission(38, 0, 0);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeTurn();

        TS_ASSERT(!p.isAtWaypoint());
        TS_ASSERT(!p.isAtTurnLimit());
        TS_ASSERT_EQUALS(p.getMovementFuelUsed(), 0);
        TS_ASSERT_EQUALS(p.getCargo(Element::Neutronium), 10);
        TS_ASSERT_EQUALS(p.getWarpFactor(), 0);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedMission), true);
    }
}

/** Test multiple cases of movement. */
void
TestGameMapShipPredictor::testMovement2()
{
    const HostVersion HOST(HostVersion::Host, MKVERSION(3,22,40));

    // Test cases from http://phost.de/~stefan/movement.html
    // - Inexact, 4 quadrants
    ::testMovement2( 16,  82, 9,   16,  80, HOST);
    ::testMovement2(-16,  82, 9,  -16,  80, HOST);
    ::testMovement2( 16, -82, 9,   16, -80, HOST);
    ::testMovement2(-16, -82, 9,  -16, -80, HOST);

    ::testMovement2( 67,  53, 9,   64,  51, HOST);
    ::testMovement2(-67,  53, 9,  -64,  51, HOST);
    ::testMovement2( 67, -53, 9,   64, -51, HOST);
    ::testMovement2(-67, -53, 9,  -64, -51, HOST);

    // - exact, maximum distance
    ::testMovement2( 48,  66, 9,   48,  66, HOST);
    ::testMovement2(-48,  66, 9,  -48,  66, HOST);
    ::testMovement2( 48, -66, 9,   48, -66, HOST);
    ::testMovement2(-48, -66, 9,  -48, -66, HOST);

    // Some simple cases
    ::testMovement2( 10,  20, 5,   10,  20, HOST);
    ::testMovement2( 10,  20, 9,   10,  20, HOST);
    ::testMovement2(100,   0, 4,   16,   0, HOST);
    ::testMovement2(100,   0, 9,   81,   0, HOST);
}

/** Test damage handling. */
void
TestGameMapShipPredictor::testDamage()
{
    const int SHIP_ID = 42;

    // Damage speed limit
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 10);
        s.setWaypoint(Point(X + 15, Y));
        s.setWarpFactor(9);
        s.setDamage(50);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeTurn();

        TS_ASSERT_EQUALS(p.getWarpFactor(), 5);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedDamageLimit), true);
    }

    // Damage speed limit with self repair
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 10);
        s.setCargo(Element::Supplies, 102);     // fixes 20 damage -> 30 remaining
        s.setWaypoint(Point(X + 15, Y));
        s.setWarpFactor(9);
        s.setDamage(50);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeTurn();

        TS_ASSERT_EQUALS(p.getWarpFactor(), 7);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedDamageLimit), true);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedRepair), true);
        TS_ASSERT_EQUALS(p.getCargo(Element::Supplies), 2);
    }

    // Self repair, no damage limit
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 10);
        s.setCargo(Element::Supplies, 400);     // fixes 80 damage
        s.setWaypoint(Point(X + 15, Y));
        s.setWarpFactor(9);
        s.setDamage(50);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeTurn();

        TS_ASSERT_EQUALS(p.getWarpFactor(), 9);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedDamageLimit), false);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedRepair), true);
        TS_ASSERT_EQUALS(p.getCargo(Element::Supplies), 150);
    }

    // Base repair, no damage limit
    {
        TestHarness t;
        Ship& s = addEmerald(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 10);
        s.setCargo(Element::Supplies, 400);
        s.setWarpFactor(9);
        s.setDamage(50);

        game::map::Planet& b = *t.univ.planets().create(123);
        b.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(1));
        b.addCurrentBaseData(game::map::BaseData(), game::PlayerSet_t(1));
        b.setPosition(Point(X, Y));
        b.setOwner(1);
        b.setBaseShipyardOrder(game::FixShipyardAction, SHIP_ID);

        finish(t);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        p.computeTurn();

        TS_ASSERT_EQUALS(p.getWarpFactor(), 9);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedDamageLimit), false);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedRepair), false);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedShipyard), true);
        TS_ASSERT_EQUALS(p.getCargo(Element::Supplies), 400);
    }
}

/** Test torpedo related operations. */
void
TestGameMapShipPredictor::testTorpedoes()
{
    const int SHIP_ID = 235;

    // mkt
    {
        TestHarness t;
        game::test::initStandardTorpedoes(t.shipList);
        game::test::initStandardBeams(t.shipList);

        Ship& s = addEmerald(t, SHIP_ID);
        s.setTorpedoType(6);
        s.setNumLaunchers(3);
        s.setCargo(Element::Tritanium, 20);
        s.setCargo(Element::Duranium, 15);
        s.setCargo(Element::Molybdenum, 30);
        s.setCargo(Element::Money, 2000);
        s.setCargo(Element::fromTorpedoType(6), 3);
        s.setFriendlyCode(String_t("mkt"));

        finish(t);

        game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, key);
        p.computeTurn();

        TS_ASSERT_EQUALS(p.getCargo(Element::fromTorpedoType(6)), 18);  // 15 built
        TS_ASSERT_EQUALS(p.getCargo(Element::Tritanium), 5);
        TS_ASSERT_EQUALS(p.getCargo(Element::Duranium), 0);
        TS_ASSERT_EQUALS(p.getCargo(Element::Molybdenum), 15);
        TS_ASSERT_EQUALS(p.getCargo(Element::Money), 2000 - 13*15);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedFCode), true);
    }

    // Lay Mines
    {
        TestHarness t;
        game::test::initStandardTorpedoes(t.shipList);
        game::test::initStandardBeams(t.shipList);

        Ship& s = addEmerald(t, SHIP_ID);
        s.setTorpedoType(6);
        s.setNumLaunchers(3);
        s.setCargo(Element::fromTorpedoType(6), 20);
        s.setFriendlyCode(String_t("mdh"));
        s.setMission(3, 0, 0);

        finish(t);

        game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, key);
        p.computeTurn();

        TS_ASSERT_EQUALS(p.getCargo(Element::fromTorpedoType(6)), 10);  // 10 laid
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedFCode), true);
        TS_ASSERT_EQUALS(p.getUsedProperties().contains(ShipPredictor::UsedMission), true);
    }
}

