/**
  *  \file interpreter/structuretype.hpp
  */
#ifndef C2NG_INTERPRETER_STRUCTURETYPE_HPP
#define C2NG_INTERPRETER_STRUCTURETYPE_HPP

#include "afl/data/namemap.hpp"
#include "interpreter/basevalue.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"

namespace interpreter {

    /** Structure type.
        Contains everything that makes up a structure type.
        Actually, a structure type is just the member name/slot mapping, so
        this could have been a typedef to IntVariableNames, but making this
        a type keeps the door open for future expansion. */
    class StructureTypeData : public afl::base::RefCounted {
     public:
        StructureTypeData();
        ~StructureTypeData();

        afl::data::NameMap names;
    };

    /** Handle to a structure type. */
    class StructureType : public BaseValue {
     public:
        StructureType(afl::base::Ref<StructureTypeData> type);
        ~StructureType();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;

        // Value:
        virtual StructureType* clone() const;

        afl::base::Ref<StructureTypeData> getType() const
            { return m_type; }

     private:
        afl::base::Ref<StructureTypeData> m_type;
    };
}

#endif
