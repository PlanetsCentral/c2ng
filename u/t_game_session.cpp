/**
  *  \file u/t_game_session.cpp
  *  \brief Test for game::Session
  */

#include <memory>
#include "game/session.hpp"

#include "t_game.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/access.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/ufo.hpp"
#include "game/map/universe.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/root.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"
#include "interpreter/subroutinevalue.hpp"

/** Test initialisation.
    A: create a session
    E: verify initial values */
void
TestGameSession::testInit()
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Initial values
    TS_ASSERT_EQUALS(testee.translator()("foo"), "foo");
    TS_ASSERT(testee.getRoot().get() == 0);
    TS_ASSERT(testee.getShipList().get() == 0);
    TS_ASSERT(testee.getGame().get() == 0);
    TS_ASSERT(testee.getEditableAreas().empty());
    TS_ASSERT(testee.world().fileTable().getFreeFile() != 0);
    TS_ASSERT(testee.world().globalPropertyNames().getIndexByName("HULL") != afl::data::NameMap::nil);
    TS_ASSERT_EQUALS(testee.getPluginDirectoryName(), "");

    // EditableAreas is modifiable
    game::Session::AreaSet_t a(game::Session::CommandArea);
    testee.setEditableAreas(a);
    TS_ASSERT_EQUALS(testee.getEditableAreas(), a);

    // Plugin directory is modifiable
    testee.setPluginDirectoryName("/pp");
    TS_ASSERT_EQUALS(testee.getPluginDirectoryName(), "/pp");
}

/** Test subobjects.
    A: create a session. Access subobjects.
    E: subobject references match */
void
TestGameSession::testSubobjects()
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session s(tx, fs);
    const game::Session& cs(s);

    TS_ASSERT_EQUALS(&s.translator(), &tx);
    TS_ASSERT_EQUALS(&s.world().fileSystem(), &fs);

    TS_ASSERT_EQUALS(&s.uiPropertyStack(), &cs.uiPropertyStack());
    TS_ASSERT_EQUALS(&s.notifications(), &cs.notifications());
    // TS_ASSERT_EQUALS(&s.world(), &cs.world());
    TS_ASSERT_EQUALS(&s.processList(), &cs.processList());
}

/** Test getReferenceName().
    A: create empty session. Call getReferenceName().
    E: must report unknown for all objects */
void
TestGameSession::testReferenceNameEmpty()
{
    using game::Reference;

    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    String_t s;
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(),                            game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Player, 3),        game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::PlainName).orElse(""), "(2000,3000)");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ship, 17),         game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Planet, 9),        game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Starbase, 9),      game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::IonStorm, 4),      game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Minefield, 150),   game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ufo, 42),          game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Hull, 15),         game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Engine, 2),        game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Beam, 3),          game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::PlainName).isValid(), false);
}

/** Test getReferenceName().
    A: create session, add some objects. Call getReferenceName().
    E: must report correct names for all objects */
void
TestGameSession::testReferenceNameNonempty()
{
    using game::Reference;

    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate ship list
    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    shipList->hulls().create(15)->setName("SMALL FREIGHTER");
    shipList->engines().create(2)->setName("2-cyl. engine");
    shipList->beams().create(3)->setName("Pink Laser");
    shipList->launchers().create(7)->setName("Mark 7 Torpedo");
    testee.setShipList(shipList);

    // Populate root
    afl::base::Ptr<game::Root> root = game::test::makeRoot(game::HostVersion()).asPtr();
    root->playerList().create(3)->setName(game::Player::ShortName, "The Romulans");
    testee.setRoot(root);

    // Populate game
    afl::base::Ptr<game::Game> g = new game::Game();
    g->currentTurn().universe().planets().create(9)->setName("Pluto");
    g->currentTurn().universe().ships().create(17)->setName("Voyager");
    g->currentTurn().universe().ionStorms().create(4)->setName("Kathrina");
    g->currentTurn().universe().minefields().create(150);
    g->currentTurn().universe().ufos().addUfo(42, 1, 1)->setName("Hui");
    testee.setGame(g);

    // Query plain names
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(),                            game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Player, 3),        game::PlainName).orElse(""), "The Romulans");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::PlainName).orElse(""), "(2000,3000)");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ship, 17),         game::PlainName).orElse(""), "Voyager");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Planet, 9),        game::PlainName).orElse(""), "Pluto");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Starbase, 9),      game::PlainName).orElse(""), "Pluto");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::IonStorm, 4),      game::PlainName).orElse(""), "Kathrina");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Minefield, 150),   game::PlainName).orElse(""), "Deleted Mine Field #150");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ufo, 42),          game::PlainName).orElse(""), "Hui");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Hull, 15),         game::PlainName).orElse(""), "SMALL FREIGHTER");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Engine, 2),        game::PlainName).orElse(""), "2-cyl. engine");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Beam, 3),          game::PlainName).orElse(""), "Pink Laser");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::PlainName).orElse(""), "Mark 7 Torpedo");

    // Query detailed names
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(),                            game::DetailedName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Player, 3),        game::DetailedName).orElse(""), "Player #3: The Romulans");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(game::map::Point(2000,3000)), game::DetailedName).orElse(""), "(2000,3000)");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ship, 17),         game::DetailedName).orElse(""), "Ship #17: Voyager");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Planet, 9),        game::DetailedName).orElse(""), "Planet #9: Pluto");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Starbase, 9),      game::DetailedName).orElse(""), "Starbase #9: Pluto");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::IonStorm, 4),      game::DetailedName).orElse(""), "Ion storm #4: Kathrina");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Minefield, 150),   game::DetailedName).orElse(""), "Deleted Mine Field #150");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ufo, 42),          game::DetailedName).orElse(""), "Ufo #42: Hui");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Hull, 15),         game::DetailedName).orElse(""), "Hull #15: SMALL FREIGHTER");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Engine, 2),        game::DetailedName).orElse(""), "Engine #2: 2-cyl. engine");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Beam, 3),          game::DetailedName).orElse(""), "Beam Weapon #3: Pink Laser");
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Torpedo, 7),       game::DetailedName).orElse(""), "Torpedo Type #7: Mark 7 Torpedo");

    // Access off-by-one Ids (that is, container exists but object doesn't)
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(),                            game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Player, 4),        game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ship, 18),         game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Planet, 8),        game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Starbase, 8),      game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::IonStorm, 5),      game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Minefield, 152),   game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Ufo, 43),          game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Hull, 16),         game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Engine, 3),        game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Beam, 4),          game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(Reference(Reference::Torpedo, 8),       game::PlainName).isValid(), false);
}

/** Test InterpreterInterface implementation.
    A: create session. Call InterpreterInterface methods.
    E: correct results produced. */
void
TestGameSession::testInterpreterInterface()
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate ship list
    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    shipList->hulls().create(3)->setName("SCOUT");
    testee.setShipList(shipList);

    // Populate root
    afl::base::Ptr<game::Root> root = game::test::makeRoot(game::HostVersion()).asPtr();
    root->playerList().create(5)->setName(game::Player::AdjectiveName, "Pirate");
    testee.setRoot(root);

    // Populate game
    afl::base::Ptr<game::Game> g = new game::Game();
    g->currentTurn().universe().ships().create(17)->setName("Voyager");
    testee.setGame(g);

    // Verify
    game::InterpreterInterface& iface = testee.interface();

    // - getComment(), hasTask() - return defaults because not configured in this test
    TS_ASSERT_EQUALS(iface.getComment(iface.Ship, 17), "");
    TS_ASSERT_EQUALS(iface.hasTask(iface.Ship, 17), false);

    // - getHullShortName
    TS_ASSERT_EQUALS(iface.getHullShortName(3).orElse(""), "SCOUT");
    TS_ASSERT_EQUALS(iface.getHullShortName(10).isValid(), false);

    // - getPlayerAdjective
    TS_ASSERT_EQUALS(iface.getPlayerAdjective(5).orElse(""), "Pirate");
    TS_ASSERT_EQUALS(iface.getPlayerAdjective(10).isValid(), false);
}

/** Test task handling/inquiry. */
void
TestGameSession::testTask()
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Populate root
    afl::base::Ptr<game::Root> root = game::test::makeRoot(game::HostVersion()).asPtr();
    testee.setRoot(root);

    // Populate game
    afl::base::Ptr<game::Game> g = new game::Game();
    game::map::Planet* p = g->currentTurn().universe().planets().create(17);
    testee.setGame(g);

    // Initial inquiry
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkPlanetTask, false), game::Session::NoTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkBaseTask, false),   game::Session::NoTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkPlanetTask, true),  game::Session::NoTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkBaseTask, true),    game::Session::NoTask);

    // Create CC$AUTOEXEC mock (we only want the process to suspend)
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->addArgument("A", false);
    bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);
    testee.world().setNewGlobalValue("CC$AUTOEXEC", new interpreter::SubroutineValue(bco));

    // Create auto task (content doesn't matter; it's all given to CC$AUTOEXEC)
    afl::base::Ptr<interpreter::TaskEditor> editor = testee.getAutoTaskEditor(17, interpreter::Process::pkPlanetTask, true);
    TS_ASSERT(editor.get() != 0);
    String_t command[] = { "whatever" };
    editor->addAtEnd(command);
    editor->setPC(0);
    testee.releaseAutoTaskEditor(editor);

    // Inquiry
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkPlanetTask, false), game::Session::ActiveTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkBaseTask, false),   game::Session::OtherTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkPlanetTask, true),  game::Session::NoTask);
    TS_ASSERT_EQUALS(testee.getTaskStatus(p, interpreter::Process::pkBaseTask, true),    game::Session::NoTask);
}

/** Test file character set handling. */
void
TestGameSession::testFileCharsetHandling()
{
    afl::io::InternalFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session testee(tx, fs);

    // Initial file system content
    const char*const SCRIPT =
        "t := chr(246)\n"
        "open '/file.txt' for output as #1\n"
        "print #1, t\n"
        "close #1\n"
        "a := ''\n"
        "open '/data.dat' for output as #1\n"
        "setstr a, 0, 20, t\n"
        "put #1, a, 20\n"
        "close #1\n";
    fs.createDirectory("/gd");
    fs.openFile("/gd/t.q", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(SCRIPT));

    // Create a root. This sets the charset.
    testee.setRoot(new game::Root(fs.openDirectory("/gd"),
                                  *new game::test::SpecificationLoader(),
                                  game::HostVersion(),
                                  std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Registered, 10)),
                                  std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                  std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepage437)),
                                  game::Root::Actions_t()));

    // Build a script process
    interpreter::World& w = testee.world();
    interpreter::Process& p = testee.processList().create(w, "testFileCharsetHandling");
    afl::base::Ptr<afl::io::Stream> in = w.openLoadFile("t.q");
    TS_ASSERT(in.get() != 0);               // Fails if Session/Root does not correctly provide the load directory
    p.pushFrame(w.compileFile(*in, "origin", 1), false);

    // Run the process
    uint32_t pgid = testee.processList().allocateProcessGroup();
    testee.processList().resumeProcess(p, pgid);
    testee.processList().startProcessGroup(pgid);
    testee.processList().run();

    // Verify
    TS_ASSERT_EQUALS(p.getState(), interpreter::Process::Ended);

    // Verify file content
    uint8_t tmp[100];
    size_t n;

    // - text file
    TS_ASSERT_THROWS_NOTHING(n = fs.openFile("/file.txt", afl::io::FileSystem::OpenRead)->read(tmp));
    TS_ASSERT_LESS_THAN_EQUALS(2U, n);      // at least two characters [first is payload, second (and more) for system newline]
    TS_ASSERT_EQUALS(tmp[0], 0x94);         // 0x94 = U+00F6 in codepage 437, fails if Session/Root does not correctly provide the charset

    // - binary file
    TS_ASSERT_THROWS_NOTHING(n = fs.openFile("/data.dat", afl::io::FileSystem::OpenRead)->read(tmp));
    TS_ASSERT_EQUALS(n, 20U);
    TS_ASSERT_EQUALS(tmp[0], 0x94);
    TS_ASSERT_EQUALS(tmp[1], 0x20);
    TS_ASSERT_EQUALS(tmp[2], 0x20);
    TS_ASSERT_EQUALS(tmp[19], 0x20);
}

