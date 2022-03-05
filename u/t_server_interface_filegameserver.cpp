/**
  *  \file u/t_server_interface_filegameserver.cpp
  *  \brief Test for server::interface::FileGameServer
  */

#include "server/interface/filegameserver.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/filegameclient.hpp"

using afl::string::Format;
using afl::data::Access;
using afl::data::Segment;
using afl::data::Value;
using server::interface::FileGame;

namespace {
    class FileGameMock : public server::interface::FileGame, public afl::test::CallReceiver {
     public:
        FileGameMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        void getGameInfo(String_t path, GameInfo& result)
            {
                checkCall(Format("getGameInfo(%s)", path));
                result = consumeReturnValue<GameInfo>();
            }
        void listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result)
            {
                checkCall(Format("listGameInfo(%s)", path));
                size_t n = consumeReturnValue<size_t>();
                while (n-- > 0) {
                    result.pushBackNew(consumeReturnValue<GameInfo*>());
                }
            }
        void getKeyInfo(String_t path, KeyInfo& result)
            {
                checkCall(Format("getKeyInfo(%s)", path));
                result = consumeReturnValue<KeyInfo>();
            }
        void listKeyInfo(String_t path, const Filter& filter, afl::container::PtrVector<KeyInfo>& result)
            {
                checkCall(Format("listKeyInfo(%s,%s,%d)", path, filter.keyId.orElse("-"), int(filter.unique)));
                size_t n = consumeReturnValue<size_t>();
                while (n-- > 0) {
                    result.pushBackNew(consumeReturnValue<KeyInfo*>());
                }
            }
    };
}

/** Simple test. */
void
TestServerInterfaceFileGameServer::testIt()
{
    FileGameMock mock("testIt");
    server::interface::FileGameServer testee(mock);

    // getGameInfo
    {
        FileGame::GameInfo gi;
        gi.pathName = "p";
        gi.gameName = "g";
        gi.hostVersion = "Gh 3";
        gi.gameId = 99;
        gi.hostTime = 13579;
        gi.isFinished = false;
        gi.slots.push_back(FileGame::Slot_t(2, "Liz"));
        gi.slots.push_back(FileGame::Slot_t(9, "Bot"));
        gi.missingFiles.push_back("race.nm");
        gi.conflictSlots.push_back(2);
        gi.conflictSlots.push_back(3);
        gi.conflictSlots.push_back(5);

        mock.expectCall("getGameInfo(pp)");
        mock.provideReturnValue<FileGame::GameInfo>(gi);

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("STATGAME").pushBackString("pp")));
        TS_ASSERT(p.get() != 0);

        Access a(p);
        TS_ASSERT_EQUALS(a("path").toString(), "p");
        TS_ASSERT_EQUALS(a("name").toString(), "g");
        TS_ASSERT_EQUALS(a("hostversion").toString(), "Gh 3");
        TS_ASSERT_EQUALS(a("game").toInteger(), 99);
        TS_ASSERT_EQUALS(a("hosttime").toInteger(), 13579);
        TS_ASSERT_EQUALS(a("finished").toInteger(), 0);
        TS_ASSERT_EQUALS(a("races").getArraySize(), 4U);
        TS_ASSERT_EQUALS(a("races")[0].toInteger(), 2);
        TS_ASSERT_EQUALS(a("races")[1].toString(), "Liz");
        TS_ASSERT_EQUALS(a("races")[2].toInteger(), 9);
        TS_ASSERT_EQUALS(a("races")[3].toString(), "Bot");
        TS_ASSERT_EQUALS(a("missing").getArraySize(), 1U);
        TS_ASSERT_EQUALS(a("missing")[0].toString(), "race.nm");
        TS_ASSERT_EQUALS(a("conflict").getArraySize(), 3U);
        TS_ASSERT_EQUALS(a("conflict")[0].toInteger(), 2);
        TS_ASSERT_EQUALS(a("conflict")[1].toInteger(), 3);
        TS_ASSERT_EQUALS(a("conflict")[2].toInteger(), 5);
        mock.checkFinish();
    }

    // listGameInfo
    {
        std::auto_ptr<FileGame::GameInfo> gi;
        mock.provideReturnValue<size_t>(2);

        gi.reset(new FileGame::GameInfo());
        gi->pathName = "q/1";
        gi->gameName = "g1";
        gi->gameId = 99;
        gi->hostTime = 13579;
        gi->isFinished = false;
        mock.provideReturnValue<FileGame::GameInfo*>(gi.release());

        gi.reset(new FileGame::GameInfo());
        gi->pathName = "q/2";
        gi->gameName = "g2";
        gi->gameId = 77;
        gi->hostTime = 0;
        gi->isFinished = true;
        mock.provideReturnValue<FileGame::GameInfo*>(gi.release());

        mock.expectCall("listGameInfo(q)");

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("LSGAME").pushBackString("q")));
        TS_ASSERT(p.get() != 0);

        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0]("path").toString(), "q/1");
        TS_ASSERT_EQUALS(a[0]("finished").toInteger(), 0);
        TS_ASSERT_EQUALS(a[1]("path").toString(), "q/2");
        TS_ASSERT_EQUALS(a[1]("finished").toInteger(), 1);

        mock.checkFinish();
    }

    // getKeyInfo (classic)
    {
        FileGame::KeyInfo ki;
        ki.pathName = "a/k";
        ki.fileName = "a/k/keyfile";
        ki.isRegistered = true;
        ki.label1 = "L1";
        ki.label2 = "L2";

        mock.expectCall("getKeyInfo(a/k)");
        mock.provideReturnValue<FileGame::KeyInfo>(ki);

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("STATREG").pushBackString("a/k")));
        TS_ASSERT(p.get() != 0);

        Access a(p);
        TS_ASSERT_EQUALS(a("path").toString(), "a/k");
        TS_ASSERT_EQUALS(a("file").toString(), "a/k/keyfile");
        TS_ASSERT_EQUALS(a("reg").toInteger(), 1);
        TS_ASSERT_EQUALS(a("key1").toString(), "L1");
        TS_ASSERT_EQUALS(a("key2").toString(), "L2");
        TS_ASSERT(a("useCount").isNull());
        TS_ASSERT(a("id").isNull());

        mock.checkFinish();
    }

    // getKeyInfo (full)
    {
        FileGame::KeyInfo ki;
        ki.pathName = "a/k";
        ki.fileName = "a/k/keyfile";
        ki.isRegistered = true;
        ki.label1 = "L1";
        ki.label2 = "L2";
        ki.useCount = 32;
        ki.keyId = String_t("ididid");

        mock.expectCall("getKeyInfo(a/k)");
        mock.provideReturnValue<FileGame::KeyInfo>(ki);

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("STATREG").pushBackString("a/k")));
        TS_ASSERT(p.get() != 0);

        Access a(p);
        TS_ASSERT_EQUALS(a("path").toString(), "a/k");
        TS_ASSERT_EQUALS(a("file").toString(), "a/k/keyfile");
        TS_ASSERT_EQUALS(a("reg").toInteger(), 1);
        TS_ASSERT_EQUALS(a("key1").toString(), "L1");
        TS_ASSERT_EQUALS(a("key2").toString(), "L2");
        TS_ASSERT_EQUALS(a("useCount").toInteger(), 32);
        TS_ASSERT_EQUALS(a("id").toString(), "ididid");

        mock.checkFinish();
    }

    // listKeyInfo
    {
        std::auto_ptr<FileGame::KeyInfo> ki;
        mock.provideReturnValue<size_t>(3);

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/p1";
        ki->isRegistered = true;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/p2";
        ki->isRegistered = true;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/sw";
        ki->isRegistered = false;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        mock.expectCall("listKeyInfo(r,-,0)");

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("LSREG").pushBackString("r")));
        TS_ASSERT(p.get() != 0);

        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0]("path").toString(), "r/p1");
        TS_ASSERT_EQUALS(a[0]("reg").toInteger(), true);
        TS_ASSERT_EQUALS(a[1]("path").toString(), "r/p2");
        TS_ASSERT_EQUALS(a[1]("reg").toInteger(), true);
        TS_ASSERT_EQUALS(a[2]("path").toString(), "r/sw");
        TS_ASSERT_EQUALS(a[2]("reg").toInteger(), false);

        mock.checkFinish();
    }

    // listKeyInfo with options
    {
        mock.provideReturnValue<size_t>(0);
        mock.expectCall("listKeyInfo(r,kid,0)");

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("LSREG").pushBackString("r").pushBackString("ID").pushBackString("kid")));
        TS_ASSERT(p.get() != 0);
        mock.checkFinish();
    }

    // listKeyInfo with options
    {
        mock.provideReturnValue<size_t>(0);
        mock.expectCall("listKeyInfo(r,-,1)");

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("LSREG").pushBackString("r").pushBackString("UNIQ")));
        TS_ASSERT(p.get() != 0);
        mock.checkFinish();
    }

    // Variants
    mock.expectCall("listKeyInfo(zz,-,0)");
    mock.provideReturnValue<size_t>(0);
    testee.callVoid(Segment().pushBackString("lsreg").pushBackString("zz"));
}

/** Test errors. */
void
TestServerInterfaceFileGameServer::testErrors()
{
    FileGameMock mock("testErrors");
    server::interface::FileGameServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("BADCMD")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("LSREG")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("LSREG").pushBackString("a").pushBackString("b")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("LSREG").pushBackString("a").pushBackString("ID")), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip behaviour. */
void
TestServerInterfaceFileGameServer::testRoundtrip()
{
    FileGameMock mock("testRoundtrip");
    server::interface::FileGameServer level1(mock);
    server::interface::FileGameClient level2(level1);
    server::interface::FileGameServer level3(level2);
    server::interface::FileGameClient level4(level3);

    // getGameInfo
    {
        FileGame::GameInfo gi;
        gi.pathName = "p";
        gi.gameName = "g";
        gi.hostVersion = "HV 2.0";
        gi.gameId = 99;
        gi.hostTime = 13579;
        gi.isFinished = false;
        gi.slots.push_back(FileGame::Slot_t(2, "Liz"));
        gi.missingFiles.push_back("race.nm");
        gi.conflictSlots.push_back(5);

        mock.expectCall("getGameInfo(pp)");
        mock.provideReturnValue<FileGame::GameInfo>(gi);

        FileGame::GameInfo out;
        TS_ASSERT_THROWS_NOTHING(level4.getGameInfo("pp", out));
        TS_ASSERT_EQUALS(out.pathName, "p");
        TS_ASSERT_EQUALS(out.gameName, "g");
        TS_ASSERT_EQUALS(out.hostVersion, "HV 2.0");
        TS_ASSERT_EQUALS(out.gameId, 99);
        TS_ASSERT_EQUALS(out.hostTime, 13579);
        TS_ASSERT_EQUALS(out.isFinished, false);
        TS_ASSERT_EQUALS(out.slots.size(), 1U);
        TS_ASSERT_EQUALS(out.slots[0].first, 2);
        TS_ASSERT_EQUALS(out.slots[0].second, "Liz");
        TS_ASSERT_EQUALS(out.missingFiles.size(), 1U);
        TS_ASSERT_EQUALS(out.missingFiles[0], "race.nm");
        TS_ASSERT_EQUALS(out.conflictSlots.size(), 1U);
        TS_ASSERT_EQUALS(out.conflictSlots[0], 5);
        mock.checkFinish();
    }

    // listGameInfo
    {
        std::auto_ptr<FileGame::GameInfo> gi;
        mock.provideReturnValue<size_t>(1);

        gi.reset(new FileGame::GameInfo());
        gi->pathName = "q/1";
        gi->gameName = "g1";
        gi->gameId = 99;
        gi->hostTime = 13579;
        gi->isFinished = false;
        mock.provideReturnValue<FileGame::GameInfo*>(gi.release());

        mock.expectCall("listGameInfo(q)");

        afl::container::PtrVector<FileGame::GameInfo> out;
        TS_ASSERT_THROWS_NOTHING(level4.listGameInfo("q", out));

        TS_ASSERT_EQUALS(out.size(), 1U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT_EQUALS(out[0]->pathName, "q/1");
        TS_ASSERT_EQUALS(out[0]->gameId, 99);

        mock.checkFinish();
    }

    // getKeyInfo (classic)
    {
        FileGame::KeyInfo ki;
        ki.pathName = "e/k";
        ki.fileName = "e/k/keyfile";
        ki.isRegistered = true;
        ki.label1 = "e1";
        ki.label2 = "e2";

        mock.expectCall("getKeyInfo(e/k)");
        mock.provideReturnValue<FileGame::KeyInfo>(ki);

        FileGame::KeyInfo out;
        TS_ASSERT_THROWS_NOTHING(level4.getKeyInfo("e/k", out));
        TS_ASSERT_EQUALS(out.pathName, "e/k");
        TS_ASSERT_EQUALS(out.fileName, "e/k/keyfile");
        TS_ASSERT_EQUALS(out.isRegistered, true);
        TS_ASSERT_EQUALS(out.label1, "e1");
        TS_ASSERT_EQUALS(out.label2, "e2");
        TS_ASSERT(!out.useCount.isValid());
        TS_ASSERT(!out.keyId.isValid());

        mock.checkFinish();
    }

    // getKeyInfo (full)
    {
        FileGame::KeyInfo ki;
        ki.pathName = "e/k";
        ki.fileName = "e/k/keyfile";
        ki.isRegistered = true;
        ki.label1 = "e1";
        ki.label2 = "e2";
        ki.useCount = 44;
        ki.keyId = "kid";

        mock.expectCall("getKeyInfo(e/k)");
        mock.provideReturnValue<FileGame::KeyInfo>(ki);

        FileGame::KeyInfo out;
        TS_ASSERT_THROWS_NOTHING(level4.getKeyInfo("e/k", out));
        TS_ASSERT_EQUALS(out.pathName, "e/k");
        TS_ASSERT_EQUALS(out.fileName, "e/k/keyfile");
        TS_ASSERT_EQUALS(out.isRegistered, true);
        TS_ASSERT_EQUALS(out.label1, "e1");
        TS_ASSERT_EQUALS(out.label2, "e2");
        TS_ASSERT_EQUALS(out.useCount.orElse(-1), 44);
        TS_ASSERT_EQUALS(out.keyId.orElse(""), "kid");

        mock.checkFinish();
    }

    // listKeyInfo
    {
        std::auto_ptr<FileGame::KeyInfo> ki;
        mock.provideReturnValue<size_t>(2);

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/p1";
        ki->isRegistered = true;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/sw";
        ki->isRegistered = false;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        mock.expectCall("listKeyInfo(r,-,0)");

        afl::container::PtrVector<FileGame::KeyInfo> out;
        TS_ASSERT_THROWS_NOTHING(level4.listKeyInfo("r", FileGame::Filter(), out));

        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT_EQUALS(out[0]->pathName, "r/p1");
        TS_ASSERT_EQUALS(out[0]->isRegistered, true);
        TS_ASSERT(out[1] != 0);
        TS_ASSERT_EQUALS(out[1]->pathName, "r/sw");
        TS_ASSERT_EQUALS(out[1]->isRegistered, false);

        mock.checkFinish();
    }

    // listKeyInfo with options
    {
        mock.provideReturnValue<size_t>(0);
        mock.expectCall("listKeyInfo(r,kkkk,1)");

        FileGame::Filter f;
        f.keyId = "kkkk";
        f.unique = true;
        afl::container::PtrVector<FileGame::KeyInfo> out;
        TS_ASSERT_THROWS_NOTHING(level4.listKeyInfo("r", f, out));

        TS_ASSERT_EQUALS(out.size(), 0U);
        mock.checkFinish();
    }
}

