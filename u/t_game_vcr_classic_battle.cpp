/**
  *  \file u/t_game_vcr_classic_battle.cpp
  *  \brief Test for game::vcr::classic::Battle
  */

#include "game/vcr/classic/battle.hpp"

#include "t_game_vcr_classic.hpp"
#include "game/test/shiplist.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/nulltranslator.hpp"

namespace {
    game::vcr::Object makeLeftShip()
    {
        game::vcr::Object left;
        left.setMass(150);
        left.setCrew(2);
        left.setId(14);
        left.setOwner(2);
        left.setBeamType(0);
        left.setNumBeams(0);
        left.setNumBays(0);
        left.setTorpedoType(0);
        left.setNumLaunchers(0);
        left.setNumTorpedoes(0);
        left.setNumFighters(0);
        left.setShield(100);
        return left;
    }

    game::vcr::Object makeRightShip()
    {
        game::vcr::Object right;
        right.setMass(233);
        right.setCrew(240);
        right.setId(434);
        right.setOwner(3);
        right.setBeamType(5);
        right.setNumBeams(6);
        right.setNumBays(0);
        right.setTorpedoType(7);
        right.setNumLaunchers(4);
        right.setNumTorpedoes(0);
        right.setNumFighters(0);
        right.setShield(100);
        return right;
    }
}


void
TestGameVcrClassicBattle::testSample()
{
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    game::config::HostConfiguration config;

    afl::string::NullTranslator tx;

    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0, 0);
    t.setType(game::vcr::classic::Host, 0);

    // Verify
    game::map::Point pos;
    TS_ASSERT_EQUALS(t.getNumObjects(), 2U);
    TS_ASSERT_EQUALS(t.getObject(0, false)->getId(), 14);
    TS_ASSERT_EQUALS(t.getObject(1, false)->getId(), 434);
    TS_ASSERT_EQUALS(t.getObject(0, false)->getCrew(), 2);
    TS_ASSERT_EQUALS(t.getObject(1, false)->getCrew(), 240);
    TS_ASSERT(t.getObject(2, false) == 0);
    TS_ASSERT_EQUALS(t.getPosition(pos), false);
    TS_ASSERT_EQUALS(t.getAlgorithmName(tx), "Host");

    TS_ASSERT_EQUALS(t.getSignature(), 0);
    TS_ASSERT_EQUALS(t.getSeed(), 42);
    TS_ASSERT_EQUALS(t.getCapabilities(), 0);

    // Prepare result
    t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
    TS_ASSERT_EQUALS(t.getObject(0, true)->getId(), 14);
    TS_ASSERT_EQUALS(t.getObject(1, true)->getId(), 434);
    TS_ASSERT_EQUALS(t.getObject(0, true)->getCrew(), 0);
    TS_ASSERT_EQUALS(t.getObject(1, true)->getCrew(), 240);
    TS_ASSERT_EQUALS(t.getOutcome(config, shipList, 0), 3);   // "captured by 3"
    TS_ASSERT_EQUALS(t.getOutcome(config, shipList, 1), 0);   // "survived"
    TS_ASSERT_EQUALS(t.getResultSummary(2, config, shipList, util::NumberFormatter(false, false), tx), "They have captured our ship.");
    TS_ASSERT_EQUALS(t.getResultSummary(3, config, shipList, util::NumberFormatter(false, false), tx), "We captured their ship.");
}

void
TestGameVcrClassicBattle::testPosition()
{
    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0, 0);
    t.setPosition(game::map::Point(500, 600));

    // Verify
    game::map::Point pos;
    TS_ASSERT_EQUALS(t.getPosition(pos), true);
    TS_ASSERT_EQUALS(pos.getX(), 500);
    TS_ASSERT_EQUALS(pos.getY(), 600);
}

void
TestGameVcrClassicBattle::testPoints()
{
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    game::config::HostConfiguration config;
    config[config.NumExperienceLevels].set(3);

    afl::string::NullTranslator tx;

    // Configure from pcc-v2/tests/vcr/vcr2.dat #1
    game::vcr::classic::Battle t(makeLeftShip(), makeRightShip(), 42, 0, 0);
    t.setType(game::vcr::classic::PHost4, 0);
    t.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
    TS_ASSERT_EQUALS(t.getResultSummary(2, config, shipList, util::NumberFormatter(false, false), tx), "They have captured our ship (2 BP, 5 EP).");
    TS_ASSERT_EQUALS(t.getResultSummary(3, config, shipList, util::NumberFormatter(false, false), tx), "We captured their ship (2 BP, 5 EP).");
}

