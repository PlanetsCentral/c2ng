/**
  *  \file u/t_game_proxy_currentstarbaseadaptor.cpp
  *  \brief Test for game::proxy::CurrentStarbaseAdaptor
  */

#include "game/proxy/currentstarbaseadaptor.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"

namespace {
    void addShip(game::Session& session, game::Id_t id, int x, int y, game::map::Object::Playability playability, String_t fcode, String_t name)
    {
        game::map::Ship* sh = session.getGame()->currentTurn().universe().ships().create(id);

        game::map::ShipData sd;
        sd.friendlyCode = fcode;
        sd.x = x;
        sd.y = y;
        sd.name = name;
        sd.owner = 9;
        sh->addCurrentShipData(sd, game::PlayerSet_t(9));
        sh->setPlayability(playability);
    }
}

/** Test operation on empty session: construction throws. */
void
TestGameProxyCurrentStarbaseAdaptor::testEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    TS_ASSERT_THROWS(game::proxy::CurrentStarbaseAdaptor(session, 99), game::Exception);
}

/** Test normal operation.
    As far as CurrentStarbaseAdaptor is concerned, the planet must exist. */
void
TestGameProxyCurrentStarbaseAdaptor::testNormal()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    game::map::Planet* pl = session.getGame()->currentTurn().universe().planets().create(55);

    game::proxy::CurrentStarbaseAdaptor testee(session, 55);
    TS_ASSERT_EQUALS(&testee.session(), &session);
    TS_ASSERT_EQUALS(&testee.planet(), pl);
}

/** Test findShipCloningHere(). */
void
TestGameProxyCurrentStarbaseAdaptor::testFindShipCloningHere()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    session.getGame()->currentTurn().universe().planets().create(55)
        ->setPosition(game::map::Point(777, 888));

    // Add some ships
    addShip(session, 10, 666, 666, game::map::Object::Playable, "cln", "fred");
    addShip(session, 20, 777, 888, game::map::Object::Playable, "xxx", "barney");
    addShip(session, 30, 777, 888, game::map::Object::Playable, "cln", "wilma");
    addShip(session, 40, 777, 888, game::map::Object::Playable, "cln", "betty");

    // Verify
    game::proxy::CurrentStarbaseAdaptor testee(session, 55);
    game::Id_t id = 0;
    String_t name;
    TS_ASSERT(testee.findShipCloningHere(id, name));
    TS_ASSERT_EQUALS(id, 30);
    TS_ASSERT_EQUALS(name, "wilma");
}

