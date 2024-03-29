/**
  *  \file u/t_game_vcr_flak_battle.cpp
  *  \brief Test for game::vcr::flak::Battle
  */

#include "game/vcr/flak/battle.hpp"

#include "t_game_vcr_flak.hpp"
#include "afl/base/countof.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/componentvector.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "game/vcr/flak/algorithm.hpp"
#include "game/vcr/flak/configuration.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"
#include "game/vcr/flak/setup.hpp"
#include "game/vcr/score.hpp"

namespace {
    uint8_t FILE_CONTENT[] = {
        0xb8, 0x02, 0x00, 0x00, 0x23, 0x0a, 0xde, 0x09, 0xc9, 0x7a, 0x3d, 0x6d, 0x60, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x88, 0x02, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x64, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xe0, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x65, 0x42, 0x00, 0x00, 0x29, 0x01, 0x00, 0x00, 0x09, 0x00, 0x04, 0x00, 0x02, 0x00, 0x64, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xe3, 0x55, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x04, 0x00, 0x06, 0x00, 0x02, 0x00, 0x64, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xa0, 0x92, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x42, 0x61, 0x72, 0x69, 0x75,
        0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x6e, 0x00,
        0x2b, 0x00, 0x09, 0x00, 0x51, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x0c, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x64, 0x00, 0x01, 0x00, 0x83, 0x00, 0x00, 0x00,
        0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x47, 0x69, 0x62, 0x61, 0x72, 0x69, 0x61,
        0x6e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x0b, 0x04, 0xc9, 0x00,
        0x09, 0x00, 0x53, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x36, 0x00, 0x6f, 0x01, 0x64, 0x00, 0x10, 0x00, 0xf5, 0x01, 0x00, 0x00, 0xf4, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x4e, 0x69, 0x74, 0x72, 0x6f, 0x67, 0x65, 0x6e, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x0b, 0x04, 0x36, 0x01, 0x09, 0x00,
        0x53, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x36, 0x00, 0xe2, 0x01, 0x64, 0x00, 0x10, 0x00, 0x7c, 0x02, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00,
        0xff, 0xff, 0x54, 0x68, 0x65, 0x74, 0x61, 0x20, 0x56, 0x49, 0x49, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0xba, 0x01, 0x09, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x07, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0d, 0x00, 0x26, 0x00,
        0xe6, 0x00, 0x64, 0x00, 0x1a, 0x00, 0xca, 0x01, 0x00, 0x00, 0xf4, 0x01, 0x01, 0x00, 0x00, 0x00,
        0x52, 0x4b, 0x20, 0x56, 0x61, 0x6e, 0x64, 0x69, 0x75, 0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x06, 0x08, 0xb4, 0x02, 0x09, 0x00, 0x4f, 0x00, 0x01, 0x00,
        0x07, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x69, 0x00, 0x21, 0x03,
        0x64, 0x00, 0x20, 0x00, 0xe7, 0x03, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b,
        0x20, 0x53, 0x74, 0x72, 0x6f, 0x6e, 0x74, 0x69, 0x75, 0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x00, 0x00, 0x06, 0x08, 0xce, 0x03, 0x09, 0x00, 0x4f, 0x00, 0x01, 0x00, 0x07, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x5f, 0x00, 0x53, 0x03, 0x64, 0x00,
        0x20, 0x00, 0x19, 0x04, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x75, 0x72, 0x74,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x00, 0x00, 0x13, 0x03, 0x96, 0x01, 0x04, 0x00, 0x23, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x09, 0x00,
        0x0d, 0x00, 0x59, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x02, 0x64, 0x00, 0x01, 0x00,
        0xa2, 0x03, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0xff, 0xff, 0x47, 0x72, 0x61, 0x75, 0x74, 0x76,
        0x6f, 0x72, 0x6e, 0x69, 0x78, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
        0x13, 0x03, 0xd1, 0x02, 0x04, 0x00, 0x23, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x09, 0x00, 0x0d, 0x00,
        0x64, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x02, 0x64, 0x00, 0x01, 0x00, 0xa2, 0x03,
        0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0xff, 0xff, 0x06, 0x00, 0x32, 0x00, 0x07, 0x00, 0x2a, 0x00,
        0x06, 0x00, 0x1f, 0x00, 0x07, 0x00, 0x1c, 0x00, 0x06, 0x00, 0x2f, 0x00, 0x07, 0x00, 0x28, 0x00,
        0x00, 0x00, 0x33, 0x00, 0x01, 0x00, 0x32, 0x00, 0x02, 0x00, 0x3a, 0x00, 0x03, 0x00, 0x2a, 0x00,
        0x04, 0x00, 0x2a, 0x00, 0x05, 0x00, 0x2c, 0x00
    };

    void initConfig(game::config::HostConfiguration& config)
    {
        // Host configuration from game "FLAK0"
        static const char*const OPTIONS[][2] = {
            { "EModBayRechargeRate",      "4,8,5,0" },
            { "EModBayRechargeBonus",     "0,0,0,0" },
            { "EModBeamRechargeRate",     "0,1,1,2" },
            { "EModBeamRechargeBonus",    "0,0,1,1" },
            { "EModTubeRechargeRate",     "1,2,3,5" },
            { "EModBeamHitFighterCharge", "0,0,0,0" },
            { "EModTorpHitOdds",          "1,2,3,5" },
            { "EModBeamHitOdds",          "4,4,5,8" },
            { "EModBeamHitBonus",         "2,2,3,5" },
            { "EModStrikesPerFighter",    "0,0,0,1" },
            { "EModFighterBeamExplosive", "0,0,0,0" },
            { "EModFighterBeamKill",      "0,0,0,0" },
            { "EModFighterMovementSpeed", "0,0,0,0" },
            { "EModTorpHitBonus",         "1,2,3,4" },
            { "EModTubeRechargeBonus",    "1,1,2,3" },
            { "EModShieldDamageScaling",  "0" },
            { "EModShieldKillScaling",    "0" },
            { "EModHullDamageScaling",    "0" },
            { "EModCrewKillScaling",      "-3,-6,-9,-12" },
            { "AllowAlternativeCombat",   "Yes" },
            { "AllowEngineShieldBonus",   "Yes" },
            { "BeamFiringRange",          "25000" },
            { "BeamHitShipCharge",        "600" },
            { "BeamHitFighterCharge",     "460" },
            { "BeamHitOdds",              "70" },
            { "BeamHitBonus",             "12" },
            { "BeamRechargeRate",         "4" },
            { "BeamRechargeBonus",        "4" },
            { "FireOnAttackFighters",     "Yes" },
            { "BayLaunchInterval",        "2" },
            { "BayRechargeRate",          "40" },
            { "BayRechargeBonus",         "1" },
            { "FighterBeamExplosive",     "9" },
            { "FighterBeamKill",          "9" },
            { "FighterFiringRange",       "3000" },
            { "FighterKillOdds",          "0" },
            { "FighterMovementSpeed",     "300" },
            { "PlayerRace",               "1,2,3,4,5,6,7,8,9,10,11" },
            { "StrikesPerFighter",        "5" },
            { "TorpFiringRange",          "30000" },
            { "TorpHitOdds",              "50" },
            { "TorpHitBonus",             "13" },
            { "TubeRechargeRate",         "30" },
            { "TubeRechargeBonus",        "7" },
            { "CrewKillScaling",          "30" },
            { "HullDamageScaling",        "20" },
            { "ShieldDamageScaling",      "40" },
            { "ShieldKillScaling",        "0" },
            { "ShipMovementSpeed",        "100" },
            { "StandoffDistance",         "10000" },
        };
        for (size_t i = 0; i < countof(OPTIONS); ++i) {
            config.setOption(OPTIONS[i][0], OPTIONS[i][1], game::config::ConfigurationOption::Game);
        }
    }

    void initBeams(game::spec::ShipList& list)
    {
        // Beams from game FLAK0
        //                            Las KOZ Dis Pha Dis ERa Ion TlB Inp MtS
        static const int KILL[]   = {  1, 10,  7, 15, 40, 20, 10, 45, 70, 40 };
        static const int DAMAGE[] = {  3,  1, 10, 25, 10, 40, 60, 55, 35, 80 };
        for (int i = 1; i <= 10; ++i) {
            game::spec::Beam* b = list.beams().create(i);
            b->setKillPower(KILL[i-1]);
            b->setDamagePower(DAMAGE[i-1]);
        }
    }

    void initTorpedoes(game::spec::ShipList& list)
    {
        // Torpedoes from game FLAK0
        //                            SpR PMB FuB InB PhT Gra Ark AmB Kat SFD
        static const int KILL[]   = { 10, 60, 25, 60, 15, 30, 60, 25, 80, 50 };
        static const int DAMAGE[] = { 25,  3, 50, 20, 82, 75, 50, 90, 40, 99 };
        for (int i = 1; i <= 10; ++i) {
            game::spec::TorpedoLauncher* tl = list.launchers().create(i);
            tl->setKillPower(KILL[i-1]);
            tl->setDamagePower(DAMAGE[i-1]);
        }
    }
}

/** Simple functionality test */
void
TestGameVcrFlakBattle::testIt()
{
    // Environment
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    afl::string::NullTranslator tx;
    initConfig(config);
    initBeams(shipList);
    initTorpedoes(shipList);

    // Setup
    std::auto_ptr<game::vcr::flak::Setup> setup(new game::vcr::flak::Setup());
    afl::charset::Utf8Charset cs;
    setup->load("testIt", FILE_CONTENT, cs, tx);

    game::vcr::flak::Battle testee(setup);

    // Verify content
    // - getNumObjects
    TS_ASSERT_EQUALS(testee.getNumObjects(), 8U);

    // - getObject (before)
    TS_ASSERT_EQUALS(testee.getObject(0, false)->getId(), 43);
    TS_ASSERT_EQUALS(testee.getObject(1, false)->getId(), 201);
    TS_ASSERT_EQUALS(testee.getObject(2, false)->getId(), 310);
    TS_ASSERT_EQUALS(testee.getObject(7, false)->getId(), 721);

    TS_ASSERT_EQUALS(testee.getObject(0, false)->getOwner(), 9);
    TS_ASSERT_EQUALS(testee.getObject(1, false)->getOwner(), 9);
    TS_ASSERT_EQUALS(testee.getObject(2, false)->getOwner(), 9);
    TS_ASSERT_EQUALS(testee.getObject(7, false)->getOwner(), 4);

    TS_ASSERT_EQUALS(testee.getObject(0, false)->getShield(), 100);
    TS_ASSERT_EQUALS(testee.getObject(1, false)->getShield(), 100);
    TS_ASSERT_EQUALS(testee.getObject(2, false)->getShield(), 100);
    TS_ASSERT_EQUALS(testee.getObject(7, false)->getShield(), 100);

    TS_ASSERT(testee.getObject(8, false) == 0); // out of range

    // - getNumGroups
    TS_ASSERT_EQUALS(testee.getNumGroups(), 4U);

    // - getGroupInfo
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).firstObject, 0U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).numObjects, 3U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).x, 12000);
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).y, 0);
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).owner, 9);
    TS_ASSERT_EQUALS(testee.getGroupInfo(0, config).speed, 100);

    TS_ASSERT_EQUALS(testee.getGroupInfo(1, config).firstObject, 3U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(1, config).numObjects, 1U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(1, config).x, 16997);
    TS_ASSERT_EQUALS(testee.getGroupInfo(1, config).y, 297);
    TS_ASSERT_EQUALS(testee.getGroupInfo(1, config).owner, 9);
    TS_ASSERT_EQUALS(testee.getGroupInfo(1, config).speed, 0);

    TS_ASSERT_EQUALS(testee.getGroupInfo(2, config).firstObject, 4U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(2, config).numObjects, 2U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(2, config).x, 21987);
    TS_ASSERT_EQUALS(testee.getGroupInfo(2, config).y, 768);
    TS_ASSERT_EQUALS(testee.getGroupInfo(2, config).owner, 9);
    TS_ASSERT_EQUALS(testee.getGroupInfo(2, config).speed, 100);

    TS_ASSERT_EQUALS(testee.getGroupInfo(3, config).firstObject, 6U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(3, config).numObjects, 2U);
    TS_ASSERT_EQUALS(testee.getGroupInfo(3, config).x, -28000);
    TS_ASSERT_EQUALS(testee.getGroupInfo(3, config).y, 0);
    TS_ASSERT_EQUALS(testee.getGroupInfo(3, config).owner, 4);
    TS_ASSERT_EQUALS(testee.getGroupInfo(3, config).speed, 100);

    // - getOutcome
    testee.prepareResult(config, shipList, game::vcr::Battle::NeedQuickOutcome);
    TS_ASSERT_EQUALS(testee.getOutcome(config, shipList, 0), 0);
    TS_ASSERT_EQUALS(testee.getOutcome(config, shipList, 1), 0);
    TS_ASSERT_EQUALS(testee.getOutcome(config, shipList, 2), -1);
    TS_ASSERT_EQUALS(testee.getOutcome(config, shipList, 7), -1);

    TS_ASSERT_EQUALS(testee.getOutcome(config, shipList, 8), 0); // out of range

    // - getPlayability
    TS_ASSERT_EQUALS(testee.getPlayability(config, shipList), game::vcr::Battle::IsPlayable);

    // - getAlgorithmName
    TS_ASSERT_EQUALS(testee.getAlgorithmName(tx), "FLAK");

    // - isESBActive
    TS_ASSERT_EQUALS(testee.isESBActive(config), true);

    // - getPosition
    game::map::Point pt;
    TS_ASSERT_EQUALS(testee.getPosition().get(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 2595);
    TS_ASSERT_EQUALS(pt.getY(), 2526);

    // - prepareResult/getObject (after)
    testee.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
    TS_ASSERT_EQUALS(testee.getObject(0, true)->getDamage(),   0);
    TS_ASSERT_EQUALS(testee.getObject(1, true)->getDamage(),   0);
    TS_ASSERT_EQUALS(testee.getObject(2, true)->getDamage(), 105);
    TS_ASSERT_EQUALS(testee.getObject(7, true)->getDamage(), 101);

    TS_ASSERT_EQUALS(testee.getObject(0, true)->getShield(),  35);
    TS_ASSERT_EQUALS(testee.getObject(1, true)->getShield(), 100);
    TS_ASSERT_EQUALS(testee.getObject(2, true)->getShield(),   0);
    TS_ASSERT_EQUALS(testee.getObject(7, true)->getShield(),   0);

    TS_ASSERT(testee.getObject(8, true) == 0); // out of range

    // - computeScores
    // We're destroying 2*665 = 1330 kt using 6 ships, 5 surviving. That's 266 kt destroyed per ship.
    // Using PALAggressorPointsPer10KT=2, PALAggressorKillPointsPer10KT=10, that's 1.2*1330 = 1596 kt, or 319.2 points per ship.
    // We're attacking with 120+367+482+130+801+851 = 2751
    // Using EPCombatKillScaling=800, EPCombatDamageScaling=200, we get 1330000/2751 = 483 EP.
    // Check for first two units.
    {
        game::vcr::Score s;
        TS_ASSERT_EQUALS(testee.computeScores(s, 0, config, shipList), true);
        TS_ASSERT_EQUALS(s.getBuildMillipoints().min(), 319200);
        TS_ASSERT_EQUALS(s.getBuildMillipoints().max(), 319200);
        TS_ASSERT_EQUALS(s.getExperience().min(), 483);
        TS_ASSERT_EQUALS(s.getExperience().max(), 483);
        TS_ASSERT_EQUALS(s.getTonsDestroyed().min(), 266);
        TS_ASSERT_EQUALS(s.getTonsDestroyed().max(), 266);
    }
    {
        game::vcr::Score s;
        TS_ASSERT_EQUALS(testee.computeScores(s, 1, config, shipList), true);
        TS_ASSERT_EQUALS(s.getBuildMillipoints().min(), 319200);
        TS_ASSERT_EQUALS(s.getBuildMillipoints().max(), 319200);
        TS_ASSERT_EQUALS(s.getExperience().min(), 483);
        TS_ASSERT_EQUALS(s.getExperience().max(), 483);
        TS_ASSERT_EQUALS(s.getTonsDestroyed().min(), 266);
        TS_ASSERT_EQUALS(s.getTonsDestroyed().max(), 266);
    }

    // Units #2, #7 didn't survive and therefore doesn't get any points
    {
        game::vcr::Score s;
        TS_ASSERT_EQUALS(testee.computeScores(s, 2, config, shipList), false);
        TS_ASSERT_EQUALS(testee.computeScores(s, 7, config, shipList), false);
    }
}
