/**
  *  \file interpreter/exporter/jsonexporter.hpp
  *  \brief Class interpreter::exporter::JsonExporter
  */
#ifndef C2NG_INTERPRETER_EXPORTER_JSONEXPORTER_HPP
#define C2NG_INTERPRETER_EXPORTER_JSONEXPORTER_HPP

#include "afl/io/textwriter.hpp"
#include "interpreter/exporter/exporter.hpp"

namespace interpreter { namespace exporter {

    /** Export to JSON text.
        Generates output as an array of hashes. */
    class JsonExporter : public Exporter {
     public:
        /** Constructor.
            \param file File to write to */
        explicit JsonExporter(afl::io::TextWriter& file);

        // Exporter:
        virtual void startTable(const FieldList& fields, afl::base::Memory<const TypeHint> types);
        virtual void startRecord();
        virtual void addField(afl::data::Value* value, const String_t& name, TypeHint type);
        virtual void endRecord();
        virtual void endTable();

     private:
        afl::io::TextWriter& m_file;
        bool m_firstField;
        bool m_firstRecord;
    };

} }

#endif
