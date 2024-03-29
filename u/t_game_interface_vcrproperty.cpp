/**
  *  \file u/t_game_interface_vcrproperty.cpp
  *  \brief Test for game::interface::VcrProperty
  */

#include "game/interface/vcrproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/vcr/test/battle.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::vcr::test::Battle;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    game::vcr::Object makeShip(game::Id_t id, int owner)
    {
        game::vcr::Object o;
        o.setId(id);
        o.setOwner(owner);
        o.setIsPlanet(false);
        o.setName("X");
        return o;
    }

    Battle& addBattle(game::Session& session)
    {
        afl::base::Ptr<game::vcr::test::Database> db = new game::vcr::test::Database();
        Battle& b = db->addBattle();
        b.addObject(makeShip(10, 5), 0);
        b.addObject(makeShip(20, 6), 7);
        b.addObject(makeShip(30, 7), 7);
        session.getGame()->currentTurn().setBattles(db);
        return b;
    }
}

/** General functionality test. */
void
TestGameInterfaceVcrProperty::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new game::Game());
    Battle& b = addBattle(session);

    b.setAuxiliaryInformation(Battle::aiSeed,    1111);
    b.setAuxiliaryInformation(Battle::aiMagic,   2222);
    b.setAuxiliaryInformation(Battle::aiType,    3333);
    b.setAuxiliaryInformation(Battle::aiFlags,   4444);
    b.setAuxiliaryInformation(Battle::aiAmbient, 5555);
    b.setAlgorithmName("Algo");
    b.setPosition(game::map::Point(1492, 1998));

    // Verify scalars
    verifyNewInteger("ivpSeed",      getVcrProperty(0, game::interface::ivpSeed,      session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), 1111);
    verifyNewInteger("ivpMagic",     getVcrProperty(0, game::interface::ivpMagic,     session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), 2222);
    verifyNewInteger("ivpType",      getVcrProperty(0, game::interface::ivpType,      session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), 3333);
    verifyNewString ("ivpAlgorithm", getVcrProperty(0, game::interface::ivpAlgorithm, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), "Algo");
    verifyNewInteger("ivpFlags",     getVcrProperty(0, game::interface::ivpFlags,     session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), 4444);
    verifyNewInteger("ivpNumUnits",  getVcrProperty(0, game::interface::ivpNumUnits,  session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), 3);
    verifyNewInteger("ivpLocX",      getVcrProperty(0, game::interface::ivpLocX,      session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), 1492);
    verifyNewInteger("ivpLocY",      getVcrProperty(0, game::interface::ivpLocY,      session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), 1998);
    verifyNewInteger("ivpAmbient",   getVcrProperty(0, game::interface::ivpAmbient,   session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()), 5555);

    // Verify 'Units'. Must have correct dimension.
    std::auto_ptr<afl::data::Value> units(getVcrProperty(0, game::interface::ivpUnits,     session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()));
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(units.get());
    TS_ASSERT(cv != 0);
    TS_ASSERT_EQUALS(cv->getDimension(1), 4);

    // Out-of-range access
    verifyNewNull("ivpSeed range",  getVcrProperty(1, game::interface::ivpSeed,  session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()));
    verifyNewNull("ivpUnits range", getVcrProperty(1, game::interface::ivpUnits, session, *session.getRoot(), session.getGame()->currentTurn(), *session.getShipList()));
}

