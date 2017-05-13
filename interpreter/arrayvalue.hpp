/**
  *  \file interpreter/arrayvalue.hpp
  *  \brief Class interpreter::ArrayValue
  */
#ifndef C2NG_INTERPRETER_ARRAYVALUE_HPP
#define C2NG_INTERPRETER_ARRAYVALUE_HPP

#include "afl/base/types.hpp"
#include "interpreter/indexablevalue.hpp"
#include "afl/base/ref.hpp"
#include "interpreter/arraydata.hpp"

namespace interpreter {

    class Arguments;

    /** Array reference.
        Arrays are always by-reference, because ArrayValue objects are cloned when put on the stack.
        The actual data is stored in an ArrayData object. */
    class ArrayValue : public IndexableValue {
     public:
        /** Constructor.
            \param data Array data object */
        explicit ArrayValue(afl::base::Ref<ArrayData> data);

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args);
        virtual void set(Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual Context* makeFirstContext();

        // BaseValue:
        virtual ArrayValue* clone() const;
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;

        /** Access underlying actual array.
            \return array */
        afl::base::Ref<ArrayData> getData();

     private:
        afl::base::Ref<ArrayData> m_data;
    };
}

#endif
