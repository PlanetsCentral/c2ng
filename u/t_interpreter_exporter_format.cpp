/**
  *  \file u/t_interpreter_exporter_format.cpp
  *  \brief Test for interpreter::exporter::Format
  */

#include "interpreter/exporter/format.hpp"

#include "t_interpreter_exporter.hpp"
#include "afl/string/nulltranslator.hpp"

/** Simple test. */
void
TestInterpreterExporterFormat::testIt()
{
    // toString. Test all values to catch the table disagreeing with the enum
    TS_ASSERT_EQUALS(toString(interpreter::exporter::TextFormat),        "text");
    TS_ASSERT_EQUALS(toString(interpreter::exporter::TableFormat),       "table");
    TS_ASSERT_EQUALS(toString(interpreter::exporter::CommaSVFormat),     "csv");
    TS_ASSERT_EQUALS(toString(interpreter::exporter::TabSVFormat),       "tsv");
    TS_ASSERT_EQUALS(toString(interpreter::exporter::SemicolonSVFormat), "ssv");
    TS_ASSERT_EQUALS(toString(interpreter::exporter::JSONFormat),        "json");
    TS_ASSERT_EQUALS(toString(interpreter::exporter::HTMLFormat),        "html");
    TS_ASSERT_EQUALS(toString(interpreter::exporter::DBaseFormat),       "dbf");

    // Extensions
    TS_ASSERT_EQUALS(getFileNameExtension(interpreter::exporter::HTMLFormat),  "html");
    TS_ASSERT_EQUALS(getFileNameExtension(interpreter::exporter::JSONFormat),  "js");
    TS_ASSERT_EQUALS(getFileNameExtension(interpreter::exporter::TableFormat), "txt");

    // Description/iteration
    for (size_t i = 0; i < interpreter::exporter::NUM_FORMATS; ++i) {
        afl::string::NullTranslator tx;
        TS_ASSERT_DIFFERS(getFormatDescription(interpreter::exporter::Format(i), tx), "");
    }

    // parseFormat
    interpreter::exporter::Format fmt;
    TS_ASSERT(parseFormat("dbf", fmt));
    TS_ASSERT_EQUALS(fmt, interpreter::exporter::DBaseFormat);

    TS_ASSERT(parseFormat("CSV", fmt));
    TS_ASSERT_EQUALS(fmt, interpreter::exporter::CommaSVFormat);

    TS_ASSERT(!parseFormat(String_t(), fmt));
    TS_ASSERT_EQUALS(fmt, interpreter::exporter::CommaSVFormat); // unchanged

    TS_ASSERT(!parseFormat("js", fmt));
    TS_ASSERT_EQUALS(fmt, interpreter::exporter::CommaSVFormat); // unchanged
}

