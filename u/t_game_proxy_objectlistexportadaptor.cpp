/**
  *  \file u/t_game_proxy_objectlistexportadaptor.cpp
  *  \brief Test for game::proxy::ObjectListExportAdaptor
  */

#include "game/proxy/objectlistexportadaptor.hpp"

#include "t_game_proxy.hpp"
#include "game/test/root.hpp"
#include "game/game.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "game/turn.hpp"
#include "game/map/universe.hpp"
#include "game/map/planet.hpp"
#include "afl/data/access.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/tagnode.hpp"

/** Test normal behaviour.
    Set up a normal situation and exercise general methods and sequences. */
void
TestGameProxyObjectListExportAdaptor::testIt()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);
    s.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    s.setGame(new game::Game());
    s.setShipList(new game::spec::ShipList());
    for (int i = 1; i < 30; ++i) {
        game::map::Planet* pl = s.getGame()->currentTurn().universe().planets().create(i);
        pl->setPosition(game::map::Point(i, 1000));
    }
    s.postprocessTurn(s.getGame()->currentTurn(), game::PlayerSet_t(1), game::PlayerSet_t(1), game::map::Object::Playable);

    std::vector<game::Id_t> ids;
    ids.push_back(10);
    ids.push_back(15);

    game::proxy::ObjectListExportAdaptor testee(s, game::proxy::ObjectListExportAdaptor::Planets, ids);

    // Verify links
    TS_ASSERT_EQUALS(&testee.fileSystem(), &fs);
    TS_ASSERT_EQUALS(&testee.translator(), &tx);

    // Verify configuration access
    // - initConfiguration
    s.getRoot()->userConfiguration()[game::config::UserConfiguration::ExportPlanetFields].set("ID,NAME");
    interpreter::exporter::Configuration config;
    testee.initConfiguration(config);
    TS_ASSERT_EQUALS(config.fieldList().toString(), "ID,NAME");

    // - saveConfiguration
    config.fieldList().add("OWNER");
    testee.saveConfiguration(config);
    TS_ASSERT_EQUALS(s.getRoot()->userConfiguration()[game::config::UserConfiguration::ExportPlanetFields](), "ID,NAME,OWNER");

    // Verify context and iteration
    {
        std::auto_ptr<interpreter::Context> ctx(testee.createContext());
        TS_ASSERT(ctx.get() != 0);

        interpreter::test::ContextVerifier ctxv(*ctx, "testIt");
        ctxv.verifyTypes();
        ctxv.verifyInteger("ID", 10);

        game::map::Object* obj = dynamic_cast<game::map::Object*>(ctx->getObject());
        TS_ASSERT(obj != 0);
        TS_ASSERT_EQUALS(obj->getId(), 10);

        // - second object
        TS_ASSERT_EQUALS(ctx->next(), true);
        ctxv.verifyInteger("ID", 15);

        obj = dynamic_cast<game::map::Object*>(ctx->getObject());
        TS_ASSERT(obj != 0);
        TS_ASSERT_EQUALS(obj->getId(), 15);

        // - No more objects
        TS_ASSERT_EQUALS(ctx->next(), false);
    }

    // Verify basics/cloning
    {
        std::auto_ptr<interpreter::Context> ctx(testee.createContext());
        TS_ASSERT(ctx.get() != 0);

        interpreter::test::ContextVerifier verif(*ctx, "testIt: basics");
        verif.verifyBasics();
        verif.verifyNotSerializable();
    }
}

/** Test abnormal case: empty session.
    We can still iterate, but objects pretend to have no content. */
void
TestGameProxyObjectListExportAdaptor::testNull()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    std::vector<game::Id_t> ids;
    ids.push_back(10);
    ids.push_back(15);

    game::proxy::ObjectListExportAdaptor testee(s, game::proxy::ObjectListExportAdaptor::Planets, ids);

    // Verify context and iteration
    {
        std::auto_ptr<interpreter::Context> ctx(testee.createContext());
        TS_ASSERT(ctx.get() != 0);

        // Cannot look up
        size_t propertyIndex;
        interpreter::Context::PropertyAccessor* propertyAccess = ctx->lookup("ID", propertyIndex);
        TS_ASSERT(propertyAccess == 0);

        // Cannot get an object
        afl::base::Deletable* obj = ctx->getObject();
        TS_ASSERT(obj == 0);

        // - second slot
        TS_ASSERT_EQUALS(ctx->next(), true);

        // - No more objects
        TS_ASSERT_EQUALS(ctx->next(), false);
    }
}

/** Test configuration handling, special case.
    An invalid value in the configuration is not an error. */
void
TestGameProxyObjectListExportAdaptor::testConfigError()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);
    s.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    s.getRoot()->userConfiguration()[game::config::UserConfiguration::ExportShipFields].set("-");

    std::vector<game::Id_t> ids;
    ids.push_back(10);
    ids.push_back(15);

    game::proxy::ObjectListExportAdaptor testee(s, game::proxy::ObjectListExportAdaptor::Ships, ids);

    interpreter::exporter::Configuration config;
    TS_ASSERT_THROWS_NOTHING(testee.initConfiguration(config));
    TS_ASSERT_EQUALS(config.fieldList().toString(), "");
}

