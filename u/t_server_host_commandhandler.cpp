/**
  *  \file u/t_server_host_commandhandler.cpp
  *  \brief Test for server::host::CommandHandler
  */

#include "server/host/commandhandler.hpp"

#include <stdexcept>
#include "t_server_host.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "game/test/files.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/cron.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::data::Access;
using afl::data::Segment;
using afl::net::redis::HashKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using afl::string::Format;
using server::Value_t;
using server::interface::HostCron;
using server::interface::HostGame;

namespace {
    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_hostFile(), m_userFile(), m_null(), m_mail(m_null), m_runner(), m_fs(afl::io::FileSystem::getInstance()),
              m_root(m_db, m_hostFile, m_userFile, m_mail, m_runner, m_fs, makeConfig()),
              m_hostFileClient(m_hostFile)
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        server::interface::FileBase& hostFile()
            { return m_hostFileClient; }

        void addUser(String_t userId);

        String_t createTurn();

     private:
        static server::host::Configuration makeConfig();

        afl::net::redis::InternalDatabase m_db;
        server::file::InternalFileServer m_hostFile;
        server::file::InternalFileServer m_userFile;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::FileSystem& m_fs;
        server::host::Root m_root;
        server::interface::FileBaseClient m_hostFileClient;
    };

    class CronMock : public server::host::Cron,
                     public afl::test::CallReceiver
    {
     public:
        CronMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual Event_t getGameEvent(int32_t gameId)
            {
                checkCall(Format("getGameEvent(%d)", gameId));
                return consumeReturnValue<Event_t>();
            }

        virtual void listGameEvents(std::vector<Event_t>& /*result*/)
            { }

        virtual void handleGameChange(int32_t /*gameId*/)
            { }

        virtual void suspendScheduler(server::Time_t /*absTime*/)
            { }
    };
}

void
TestHarness::addUser(String_t userId)
{
    StringSetKey(m_db, "user:all").add(userId);
    StringKey(m_db, "uid:" + userId).set(userId);
    HashKey(m_db, "user:" + userId + ":profile").stringField("email").set(userId + "@examp.le");
}

String_t
TestHarness::createTurn()
{
    String_t result;
    result += '\7';          // player
    result += '\0';
    result.append(4, '\0');  // number of commands
    result += "11-22-333344:55:66";     // timestamp
    result += "xx";          // unused
    result += "yy";          // timestamp checksum
    result.append(256, 'z'); // DOS trailer
    return result;
}

server::host::Configuration
TestHarness::makeConfig()
{
    server::host::Configuration config;
    config.workDirectory = "/tmp";
    return config;
}

/** Simple test.
    Verifies correct command dispatching.

    A: Set up a minimal environment. Execute a command from each section.
    E: Commands are executed and produce correct results. */
void
TestServerHostCommandHandler::testIt()
{
    // Environment
    CronMock cron("testIt");
    TestHarness h;
    server::host::Session session;
    h.root().setCron(&cron);
    h.addUser("zz");
    h.hostFile().createDirectoryTree("bin");
    h.hostFile().createDirectoryTree("defaults");
    h.hostFile().putFile("bin/checkturn.sh", "exit 0");

    h.hostFile().createDirectoryTree("sdir");
    h.hostFile().putFile("sdir/beamspec.dat", afl::string::fromBytes(game::test::getDefaultBeams()));
    h.hostFile().putFile("sdir/torpspec.dat", afl::string::fromBytes(game::test::getDefaultTorpedoes()));
    h.hostFile().putFile("sdir/engspec.dat",  afl::string::fromBytes(game::test::getDefaultEngines()));
    h.hostFile().putFile("sdir/hullspec.dat", afl::string::fromBytes(game::test::getDefaultHulls()));
    h.hostFile().putFile("sdir/truehull.dat", afl::string::fromBytes(game::test::getDefaultHullAssignments()));
    h.hostFile().putFile("sdir/race.nm", afl::string::fromBytes(game::test::getDefaultRaceNames()));

    // Testee
    server::host::CommandHandler testee(h.root(), session);

    // Calls into CommandHandler
    // - invalid
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.call(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);

    // - ping
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PING")), "PONG");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("ping")), "PONG");

    // - user
    testee.callVoid(Segment().pushBackString("USER").pushBackString("1024"));
    TS_ASSERT_EQUALS(session.getUser(), "1024");
    session.setUser("");

    // - help
    TS_ASSERT(testee.callString(Segment().pushBackString("HELP")).size() > 30);

    // Actual commands.
    // This produces a working command sequence
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("HOSTADD").pushBackString("H").pushBackString("").pushBackString("").pushBackString("h")));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("MASTERADD").pushBackString("M").pushBackString("").pushBackString("").pushBackString("m")));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("SHIPLISTADD").pushBackString("S").pushBackString("sdir").pushBackString("").pushBackString("s")));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("TOOLADD").pushBackString("T").pushBackString("").pushBackString("").pushBackString("t")));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("STAT").pushBackString("game")));

    int gid = testee.callInt(Segment().pushBackString("NEWGAME"));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMESETTYPE").pushBackInteger(gid).pushBackString("public")));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMESETSTATE").pushBackInteger(gid).pushBackString("running")));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("SCHEDULEADD").pushBackInteger(gid).pushBackString("MANUAL")));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("PLAYERJOIN").pushBackInteger(gid).pushBackInteger(7).pushBackString("zz")));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("TRN").pushBackString(h.createTurn()).pushBackString("GAME").pushBackInteger(gid).pushBackString("SLOT").pushBackInteger(7)));
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("SPECSHIPLIST").pushBackString("S").pushBackString("json").pushBackString("beamspec")));

    cron.expectCall("getGameEvent(1)");
    cron.provideReturnValue(HostCron::Event(1, HostCron::MasterAction, 99));
    std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("CRONGET").pushBackInteger(1)));
    TS_ASSERT_EQUALS(Access(p)("action").toString(), "master");
    TS_ASSERT_EQUALS(Access(p)("time").toInteger(), 99);
}

/** Test HELP command.
    A: invoke all variants of the HELP command
    E: section help returned correctly. Section pages are distinct from main page. Correct links on main page. */
void
TestServerHostCommandHandler::testHelp()
{
    // Environment
    TestHarness h;
    server::host::Session session;

    // Testee
    server::host::CommandHandler testee(h.root(), session);

    String_t mainHelp = testee.callString(Segment().pushBackString("HELP"));

    static const char*const SECTIONS[] = { "HOST", "MASTER", "TOOL", "SHIPLIST", "CRON", "FILE", "GAME", "PLAYER", "SCHEDULE", "SLOT", "HIST", "KEY", "SPEC", 0 };
    for (size_t i = 0; SECTIONS[i] != 0; ++i) {
        // Verify help page
        String_t sectionHelp = testee.callString(Segment().pushBackString("HELP").pushBackString(SECTIONS[i]));
        TS_ASSERT(sectionHelp.size() > 30);
        TS_ASSERT(sectionHelp != mainHelp);
        TS_ASSERT(mainHelp.find(String_t(SECTIONS[i]) + "->") != String_t::npos);

        // Verify case-blindness
        TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("HELP").pushBackString(afl::string::strLCase(SECTIONS[i]))), sectionHelp);
    }

    // Bad page name is not an error, but returns the main page
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("HELP").pushBackString("whatever")), mainHelp);
}

