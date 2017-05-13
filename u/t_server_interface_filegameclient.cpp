/**
  *  \file u/t_server_interface_filegameclient.cpp
  *  \brief Test for server::interface::FileGameClient
  */

#include "server/interface/filegameclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/stringvalue.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;
using afl::data::StringValue;
using afl::data::Vector;
using afl::data::VectorValue;

namespace {
    Hash::Ref_t makeGameResponse(String_t path, String_t name)
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("path", new StringValue(path));
        h->setNew("name", new StringValue(name));
        h->setNew("game", new StringValue("7"));
        h->setNew("finished", new StringValue("0"));
        h->setNew("hosttime", new StringValue("12324"));
        h->setNew("missing", new VectorValue(Vector::create(Segment().pushBackString("xyplan.dat"))));
        h->setNew("conflict", new VectorValue(Vector::create(Segment().pushBackInteger(3))));
        h->setNew("races", new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackString("Fed").pushBackInteger(3).pushBackString("Bird"))));
        return h;
    }

    Hash::Ref_t makeKeyResponse(String_t path, String_t label1, String_t label2)
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("path", new StringValue(path));
        h->setNew("file", new StringValue(path + "/fizz.bin"));
        h->setNew("reg", new StringValue("1"));
        h->setNew("key1", new StringValue(label1));
        h->setNew("key2", new StringValue(label2));
        return h;
    }
}

void
TestServerInterfaceFileGameClient::testIt()
{
    using server::interface::FileGame;

    CommandHandlerMock mock;
    server::interface::FileGameClient testee(mock);

    // getGameInfo - null answer
    {
        mock.expectCall("STATGAME|a/b");
        mock.provideReturnValue(0);

        FileGame::GameInfo gi;
        TS_ASSERT_THROWS_NOTHING(testee.getGameInfo("a/b", gi));
        TS_ASSERT_EQUALS(gi.gameName, "");
        TS_ASSERT_EQUALS(gi.pathName, "");
        TS_ASSERT_EQUALS(gi.gameId, 0);
        TS_ASSERT_EQUALS(gi.missingFiles.size(), 0U);
        TS_ASSERT_EQUALS(gi.conflictSlots.size(), 0U);
        TS_ASSERT_EQUALS(gi.slots.size(), 0U);
        TS_ASSERT_EQUALS(gi.isFinished, false);
    }

    // getGameInfo - real answer
    {
        mock.expectCall("STATGAME|x/y/z");
        mock.provideReturnValue(new HashValue(makeGameResponse("x/y/z/a", "Game A")));

        FileGame::GameInfo gi;
        TS_ASSERT_THROWS_NOTHING(testee.getGameInfo("x/y/z", gi));
        TS_ASSERT_EQUALS(gi.gameName, "Game A");
        TS_ASSERT_EQUALS(gi.pathName, "x/y/z/a");
        TS_ASSERT_EQUALS(gi.gameId, 7);
        TS_ASSERT_EQUALS(gi.missingFiles.size(), 1U);
        TS_ASSERT_EQUALS(gi.missingFiles[0], "xyplan.dat");
        TS_ASSERT_EQUALS(gi.conflictSlots.size(), 1U);
        TS_ASSERT_EQUALS(gi.conflictSlots[0], 3);
        TS_ASSERT_EQUALS(gi.slots.size(), 2U);
        TS_ASSERT_EQUALS(gi.slots[0].first, 1);
        TS_ASSERT_EQUALS(gi.slots[0].second, "Fed");
        TS_ASSERT_EQUALS(gi.slots[1].first, 3);
        TS_ASSERT_EQUALS(gi.slots[1].second, "Bird");
        TS_ASSERT_EQUALS(gi.isFinished, false);
    }

    // getGameInfo - answer with bogus value (must not crash)
    {
        Hash::Ref_t h = makeGameResponse("x/y/z/a", "Game A");
        h->setNew("game", new StringValue("blub"));
        mock.expectCall("STATGAME|x/y/z");
        mock.provideReturnValue(new HashValue(h));

        FileGame::GameInfo gi;
        TS_ASSERT_THROWS_NOTHING(testee.getGameInfo("x/y/z", gi));
        TS_ASSERT_EQUALS(gi.gameName, "Game A");
        TS_ASSERT_EQUALS(gi.pathName, "x/y/z/a");
        TS_ASSERT_EQUALS(gi.gameId, 0);
        TS_ASSERT_EQUALS(gi.missingFiles.size(), 1U);
        TS_ASSERT_EQUALS(gi.conflictSlots.size(), 1U);
        TS_ASSERT_EQUALS(gi.slots.size(), 2U);
        TS_ASSERT_EQUALS(gi.isFinished, false);
    }

    // listGameInfo - null answer
    {
        mock.expectCall("LSGAME|a/b");
        mock.provideReturnValue(0);

        afl::container::PtrVector<FileGame::GameInfo> result;
        TS_ASSERT_THROWS_NOTHING(testee.listGameInfo("a/b", result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // listGameInfo - real answer
    {
        mock.expectCall("LSGAME|z");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().
                                                               pushBackNew(new HashValue(makeGameResponse("z/1", "Game One"))).
                                                               pushBackNew(new HashValue(makeGameResponse("z/2", "Game Two"))).
                                                               pushBackNew(new HashValue(makeGameResponse("z/3/a", "Game Three A"))))));

        afl::container::PtrVector<FileGame::GameInfo> result;
        TS_ASSERT_THROWS_NOTHING(testee.listGameInfo("z", result));
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT_EQUALS(result[0]->gameName, "Game One");
        TS_ASSERT_EQUALS(result[0]->pathName, "z/1");
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(result[1]->gameName, "Game Two");
        TS_ASSERT_EQUALS(result[1]->pathName, "z/2");
        TS_ASSERT(result[2] != 0);
        TS_ASSERT_EQUALS(result[2]->gameName, "Game Three A");
        TS_ASSERT_EQUALS(result[2]->pathName, "z/3/a");
    }

    // listGameInfo - mixed answer (produces empty game)
    {
        mock.expectCall("LSGAME|zq");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().
                                                               pushBackNew(0).
                                                               pushBackNew(new HashValue(makeGameResponse("zq/qq", "Q"))))));

        afl::container::PtrVector<FileGame::GameInfo> result;
        TS_ASSERT_THROWS_NOTHING(testee.listGameInfo("zq", result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT_EQUALS(result[0]->gameName, "");
        TS_ASSERT_EQUALS(result[0]->pathName, "");
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(result[1]->gameName, "Q");
        TS_ASSERT_EQUALS(result[1]->pathName, "zq/qq");
    }

    // getKeyInfo - null answer
    {
        mock.expectCall("STATREG|r");
        mock.provideReturnValue(0);

        FileGame::KeyInfo result;
        TS_ASSERT_THROWS_NOTHING(testee.getKeyInfo("r", result));
        TS_ASSERT_EQUALS(result.pathName, "");
        TS_ASSERT_EQUALS(result.fileName, "");
        TS_ASSERT_EQUALS(result.isRegistered, false);
        TS_ASSERT_EQUALS(result.label1, "");
        TS_ASSERT_EQUALS(result.label2, "");
    }

    // getKeyInfo - real answer
    {
        mock.expectCall("STATREG|r2");
        mock.provideReturnValue(new HashValue(makeKeyResponse("r2", "Name", "Address")));

        FileGame::KeyInfo result;
        TS_ASSERT_THROWS_NOTHING(testee.getKeyInfo("r2", result));
        TS_ASSERT_EQUALS(result.pathName, "r2");
        TS_ASSERT_EQUALS(result.fileName, "r2/fizz.bin");
        TS_ASSERT_EQUALS(result.isRegistered, true);
        TS_ASSERT_EQUALS(result.label1, "Name");
        TS_ASSERT_EQUALS(result.label2, "Address");
    }

    // getKeyInfo - answer with bogus value (must not crash)
    {
        Hash::Ref_t h = makeKeyResponse("r2", "Name", "Address");
        h->setNew("reg", new StringValue("wut?"));
        mock.expectCall("STATREG|r2");
        mock.provideReturnValue(new HashValue(h));

        FileGame::KeyInfo result;
        TS_ASSERT_THROWS_NOTHING(testee.getKeyInfo("r2", result));
        TS_ASSERT_EQUALS(result.pathName, "r2");
        TS_ASSERT_EQUALS(result.fileName, "r2/fizz.bin");
        TS_ASSERT_EQUALS(result.isRegistered, false);
        TS_ASSERT_EQUALS(result.label1, "Name");
        TS_ASSERT_EQUALS(result.label2, "Address");
    }

    // listKeyInfo - null answer
    {
        mock.expectCall("LSREG|r3");
        mock.provideReturnValue(0);

        afl::container::PtrVector<FileGame::KeyInfo> result;
        TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("r3", result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // listKeyInfo - real answer
    {
        mock.expectCall("LSREG|z");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().
                                                               pushBackNew(new HashValue(makeKeyResponse("z/1", "Key One", "Adr 1"))).
                                                               pushBackNew(new HashValue(makeKeyResponse("z/2", "Key Two", "Adr 2"))).
                                                               pushBackNew(new HashValue(makeKeyResponse("z/3/a", "Key Three A", "Adr 3a"))))));

        afl::container::PtrVector<FileGame::KeyInfo> result;
        TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("z", result));
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT_EQUALS(result[0]->label1, "Key One");
        TS_ASSERT_EQUALS(result[0]->pathName, "z/1");
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(result[1]->label1, "Key Two");
        TS_ASSERT_EQUALS(result[1]->pathName, "z/2");
        TS_ASSERT(result[2] != 0);
        TS_ASSERT_EQUALS(result[2]->label1, "Key Three A");
        TS_ASSERT_EQUALS(result[2]->pathName, "z/3/a");
    }
}

