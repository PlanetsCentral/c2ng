/**
  *  \file game/interface/iteratorcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_ITERATORCONTEXT_HPP
#define C2NG_GAME_INTERFACE_ITERATORCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/game.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/context.hpp"
#include "interpreter/singlecontext.hpp"
#include "game/interface/iteratorprovider.hpp"

namespace game {
    class Session;
}

namespace game { namespace interface {

    class IteratorContext : public interpreter::SingleContext, public interpreter::Context::PropertyAccessor {
     public:
        IteratorContext(afl::base::Ref<IteratorProvider> provider);
        ~IteratorContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual IteratorContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

     private:
        afl::base::Ref<IteratorProvider> m_provider;
    };

    afl::data::Value* IFIterator(game::Session& session, interpreter::Arguments& args);

    interpreter::Context* makeIteratorValue(Session& session, int nr, bool reportRangeError);

    interpreter::Context* createObjectContext(game::map::Object* obj, Session& session);

} }

#endif
