/**
  *  \file interpreter/test/contextverifier.cpp
  *  \brief Class interpreter::test::ContextVerifier
  */

#include <map>
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/values.hpp"

using afl::test::Assert;

interpreter::test::ContextVerifier::ContextVerifier(Context& ctx, afl::test::Assert as)
    : m_context(ctx),
      m_assert(as)
{ }

void
interpreter::test::ContextVerifier::verifyTypes() const
{
    // Propery Collector Helper Class
    typedef std::map<String_t, TypeHint> Map_t;

    class PropertyCollector : public PropertyAcceptor {
     public:
        PropertyCollector(Map_t& map, Assert as)
            : m_data(map), m_assert(as)
            { }

        virtual void addProperty(const String_t& name, TypeHint th)
            {
                Map_t::iterator it = m_data.find(name);
                m_assert(name).check("duplicate name", it == m_data.end());
                m_data.insert(std::make_pair(name, th));
            }

     private:
        Map_t& m_data;
        Assert m_assert;
    };

    // Collect all properties
    Map_t map;
    PropertyCollector collector(map, m_assert);
    m_context.enumProperties(collector);

    // Iterate through properties.
    // Each must successfully look up and resolve to the correct type.
    int numNonNullProperties = 0;
    for (Map_t::iterator it = map.begin(); it != map.end(); ++it) {
        // Look up
        const Assert me(m_assert(it->first));
        Context::PropertyIndex_t index;
        Context*const foundContext = m_context.lookup(it->first, index);
        me.check("lookup failure", foundContext != 0);

        // Get. If it's non-null, it must be valid.
        std::auto_ptr<afl::data::Value> value(foundContext->get(index));
        if (value.get() != 0) {
            ++numNonNullProperties;
            switch (it->second) {
             case thNone:
                // No constraints
                break;

             case thBool:
                me.check("expect bool", dynamic_cast<afl::data::BooleanValue*>(value.get()) != 0);
                break;
                
             case thInt:
                me.check("expect integer", dynamic_cast<afl::data::IntegerValue*>(value.get()) != 0);
                break;

             case thFloat:
                me.check("expect float", dynamic_cast<afl::data::FloatValue*>(value.get()) != 0);
                break;

             case thString:
                me.check("expect string", dynamic_cast<afl::data::StringValue*>(value.get()) != 0);
                break;
             case thProcedure: {
                CallableValue* cv = dynamic_cast<CallableValue*>(value.get());
                me.check("expect callable", cv != 0);
                me.check("expect procedure", cv->isProcedureCall());
                break;
             }
             case thFunction: {
                CallableValue* cv = dynamic_cast<CallableValue*>(value.get());
                me.check("expect callable", cv != 0);
                me.check("expect non-procedure", !cv->isProcedureCall());
                break;
             }
             case thArray:
                me.check("expect indexable", dynamic_cast<IndexableValue*>(value.get()) != 0);
                break;
            }

            // Clone it. Both must have same stringification (otherwise, it's not a clone, right?)
            std::auto_ptr<afl::data::Value> clone(value->clone());
            me.check("expect clonable", clone.get() != 0);
            me.checkEqual("expect same toString(false)", toString(value.get(), false), toString(clone.get(), false));
            me.checkEqual("expect same toString(true)",  toString(value.get(), true),  toString(clone.get(), true));
        }
    }

    // Must have a nonzero number of non-null properties to sort out bogus implementations that only return null
    m_assert.check("expect non-null properties", numNonNullProperties > 0);
}

void
interpreter::test::ContextVerifier::verifyInteger(const char* name, int value) const
{
    Assert me(m_assert(name));

    // Look up
    Context::PropertyIndex_t index;
    Context* foundContext = m_context.lookup(name, index);
    me.check("lookup", foundContext != 0);

    // Get it
    std::auto_ptr<afl::data::Value> result(foundContext->get(index));
    me.check("expect non-null", result.get() != 0);

    afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(result.get());
    me.check("expect integer", iv != 0);
    me.checkEqual("expect value", iv->getValue(), value);
}

void
interpreter::test::ContextVerifier::verifyBoolean(const char* name, bool value) const
{
    Assert me(m_assert(name));

    // Look up
    Context::PropertyIndex_t index;
    Context* foundContext = m_context.lookup(name, index);
    me.check("lookup", foundContext != 0);

    // Get it
    std::auto_ptr<afl::data::Value> result(foundContext->get(index));
    me.check("expect non-null", result.get() != 0);

    afl::data::BooleanValue* bv = dynamic_cast<afl::data::BooleanValue*>(result.get());
    me.check("expect boolean", bv != 0);
    me.checkEqual("expect value", int(bv->getValue()), int(value));
}

void
interpreter::test::ContextVerifier::verifyString(const char* name, const char* value) const
{
    Assert me(m_assert(name));

    // Look up
    Context::PropertyIndex_t index;
    Context* foundContext = m_context.lookup(name, index);
    me.check("lookup", foundContext != 0);

    // Get it
    std::auto_ptr<afl::data::Value> result(foundContext->get(index));
    me.check("expect non-null", result.get() != 0);

    afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(result.get());
    me.check("expect string", sv != 0);
    me.checkEqual("expect value", sv->getValue(), value);
}

void
interpreter::test::ContextVerifier::verifyNull(const char* name) const
{
    Assert me(m_assert(name));
    std::auto_ptr<afl::data::Value> result(getValue(name));
    me.check("expect null", result.get() == 0);
}

afl::data::Value*
interpreter::test::ContextVerifier::getValue(const char* name) const
{
    Assert me(m_assert(name));
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context* foundContext = m_context.lookup(name, index);
    me.check("lookup", foundContext != 0);

    return foundContext->get(index);
}
