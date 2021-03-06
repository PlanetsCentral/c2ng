/**
  *  \file u/t_game_teamsettings.cpp
  *  \brief Test for game::TeamSettings
  */

#include "game/teamsettings.hpp"

#include "t_game.hpp"
#include "afl/base/ref.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test initialisation.
    Object must report virgin state. */
void
TestGameTeamSettings::testInit()
{
    game::TeamSettings testee;
    TS_ASSERT(!testee.hasAnyTeams());
    TS_ASSERT(!testee.isNamedTeam(9));
}

/** Test setters and getters. */
void
TestGameTeamSettings::testSet()
{
    afl::string::NullTranslator tx;
    game::TeamSettings testee;

    // Set
    testee.setPlayerTeam(1, 2);
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(8, 4);
    testee.setPlayerTeam(999999999, 2);   // out-of-range, must not crash

    // Must preserve
    TS_ASSERT_EQUALS(testee.getPlayerTeam(1), 2);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(9), 2);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(8), 4);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(999999999), 0);  // out-of-range

    // Accessors
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(2), 3);     // 1, 2, 9
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 2);     // 4, 8

    // Some names
    testee.setTeamName(1, "One");
    TS_ASSERT_EQUALS(testee.getTeamName(1, tx), "One");
    TS_ASSERT_EQUALS(testee.getTeamName(2, tx), "Team 2");
    TS_ASSERT(testee.isNamedTeam(1));
    TS_ASSERT(!testee.isNamedTeam(2));
    TS_ASSERT(!testee.isNamedTeam(0));
    TS_ASSERT(!testee.isNamedTeam(999999999));

    // We now have teams
    TS_ASSERT(testee.hasAnyTeams());
}

/** Test other modifications. */
void
TestGameTeamSettings::testModify()
{
    game::TeamSettings testee;
    testee.setPlayerTeam(1, 2);       // 1,2,9 in team 2
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(4, 3);       // 3,4 in team 3
    testee.setPlayerTeam(8, 4);       // 8 in team 4

    // Verify counts
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(2), 3);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(3), 2);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 1);

    // Remove player 4. Because team 4 is not available they get 1 as the first free one
    testee.removePlayerTeam(4);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(4), 1);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 1);

    // Add 4 to 4.
    testee.setPlayerTeam(4, 4);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);

    // Remove 8 from 4. 1 is still free, but because 8 is also free, this one goes to 8.
    testee.removePlayerTeam(8);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(8), 8);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 1);
}

/** Test viewpoint functions. */
void
TestGameTeamSettings::testViewpoint()
{
    game::TeamSettings testee;
    testee.setPlayerTeam(1, 2);       // 1,2,9 in team 2
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(4, 3);       // 3,4 in team 3
    testee.setPlayerTeam(8, 4);       // 8 in team 4

    // Start with no viewpoint
    TS_ASSERT_EQUALS(testee.getViewpointPlayer(), 0);

    // Set viewpoint
    testee.setViewpointPlayer(9);
    TS_ASSERT_EQUALS(testee.getViewpointPlayer(), 9);

    // Relations
    TS_ASSERT_EQUALS(testee.getPlayerRelation(1), game::TeamSettings::AlliedPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(2), game::TeamSettings::AlliedPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(3), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(4), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(8), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(9), game::TeamSettings::ThisPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(10), game::TeamSettings::EnemyPlayer);

    // Colors
    TS_ASSERT_EQUALS(testee.getPlayerColor(1), util::SkinColor::Yellow);
    TS_ASSERT_EQUALS(testee.getPlayerColor(2), util::SkinColor::Yellow);
    TS_ASSERT_EQUALS(testee.getPlayerColor(3), util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.getPlayerColor(4), util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.getPlayerColor(8), util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.getPlayerColor(9), util::SkinColor::Green);
    TS_ASSERT_EQUALS(testee.getPlayerColor(10), util::SkinColor::Red);
}

/** Test Load/Save. */
void
TestGameTeamSettings::testLoadSave()
{
    // An actual team.cc file created by PCC1
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x74, 0x65, 0x61, 0x6d, 0x30, 0x1a, 0x03, 0x00, 0x01, 0x02, 0x05, 0x09, 0x05, 0x02, 0x05, 0x02, 0x09, 0x02, 0x09, 0x0c, 0x04, 0x04,
        0x04, 0x03, 0x04, 0x04, 0x04, 0x04, 0x03, 0x04, 0x03, 0x04, 0x05, 0x68, 0x75, 0x6d, 0x61, 0x6e, 0x12, 0x64, 0x69, 0x65, 0x20, 0x77, 0x6f, 0x20,
        0x69, 0x63, 0x68, 0x20, 0x76, 0x65, 0x72, 0x68, 0x61, 0x75, 0x65, 0x00, 0x05, 0x68, 0x75, 0x6d, 0x61, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x04, 0x69,
        0x63, 0x6b, 0x65, 0x00, 0x07, 0x6b, 0x6c, 0x65, 0x6d, 0x65, 0x6e, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("gamedir");
    dir->addStream("team9.cc", *new afl::io::ConstMemoryStream(DATA));

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;

    // Test
    game::TeamSettings testee;
    TS_ASSERT_THROWS_NOTHING(testee.load(*dir, 9, cs));
    TS_ASSERT_EQUALS(testee.getTeamName(1, tx), "human");
    TS_ASSERT_EQUALS(testee.getTeamName(9, tx), "icke");

    // Erase the file so it is guaranteed to be written back
    dir->erase("team9.cc");
    TS_ASSERT_THROWS(dir->openFile("team9.cc", afl::io::FileSystem::OpenRead), std::exception);

    // Write back
    testee.save(*dir, 9, cs);

    // Verify file has been recreated with identical content
    afl::base::Ref<afl::io::Stream> file = dir->openFile("team9.cc", afl::io::FileSystem::OpenRead);
    TS_ASSERT_EQUALS(file->createVirtualMapping()->get().equalContent(DATA), true);
}

