/**
  *  \file game/interface/shipcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_SHIPCONTEXT_HPP
#define C2NG_GAME_INTERFACE_SHIPCONTEXT_HPP

#include "interpreter/context.hpp"
#include "afl/base/ref.hpp"
#include "game/root.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/map/ship.hpp"

namespace game { namespace interface {

    class ShipContext : public interpreter::Context {
     public:
        ShipContext(int id,
                    Session& session,
                    afl::base::Ref<Root> root,
                    afl::base::Ref<Game> game,
                    afl::base::Ref<game::spec::ShipList> shipList);
        ~ShipContext();

        // Context:
        virtual ShipContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual ShipContext* clone() const;
        virtual game::map::Ship* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        static ShipContext* create(int id, Session& session);

     private:
        int m_id;
        Session& m_session;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<Game> m_game;
        afl::base::Ref<game::spec::ShipList> m_shipList;
    };

} }

#endif
