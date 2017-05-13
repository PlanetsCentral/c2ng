/**
  *  \file u/t_interpreter_exporter_dbfexporter.cpp
  *  \brief Test for interpreter::exporter::DbfExporter
  */

#include "interpreter/exporter/dbfexporter.hpp"

#include "t_interpreter_exporter.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalstream.hpp"
#include "interpreter/exporter/fieldlist.hpp"

using afl::data::BooleanValue;
using afl::data::FloatValue;
using afl::data::IntegerValue;
using afl::data::StringValue;

/** Simple test. */
void
TestInterpreterExporterDbfExporter::testIt()
{
    // Prepare a field list
    interpreter::exporter::FieldList list;
    list.addList("ID@5,NAME@30");

    // Output receiver
    afl::io::InternalStream outputStream;

    // Testee
    interpreter::exporter::DbfExporter testee(outputStream);
    static const interpreter::TypeHint hints[] = { interpreter::thInt, interpreter::thString };

    // Test sequence
    testee.startTable(list, hints);
    testee.startRecord();
    {
        IntegerValue iv(9);
        StringValue sv("SHIP  9");
        testee.addField(&iv, "ID", interpreter::thInt);
        testee.addField(&sv, "NAME", interpreter::thString);
    }
    testee.endRecord();

    testee.startRecord();
    {
        IntegerValue iv(20);
        StringValue sv("SHIP  20");
        testee.addField(&iv, "ID", interpreter::thInt);
        testee.addField(&sv, "NAME", interpreter::thString);
    }
    testee.endRecord();
    testee.endTable();

    // Verify against dbf file created with PCC2 c2export
    static const uint8_t EXPECTED[] = {
        0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x61, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x49, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x4e, 0x41, 0x4d, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00,
        0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x39, 0x53, 0x48, 0x49, 0x50, 0x20, 0x20, 0x39, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x32, 0x30, 0x53, 0x48, 0x49, 0x50, 0x20,
        0x20, 0x32, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00
    };
    afl::base::ConstBytes_t bytes = outputStream.getContent();
    TS_ASSERT_EQUALS(bytes.size(), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(bytes.at(0), EXPECTED, sizeof(EXPECTED));
}

/** Additional test with more fields. */
void
TestInterpreterExporterDbfExporter::testIt2()
{
    // Prepare a field list
    interpreter::exporter::FieldList list;
    list.addList("ID@5,NAME@20,MARKED,WAYPOINT.DIST");

    // Output receiver
    afl::io::InternalStream outputStream;

    // Testee
    interpreter::exporter::DbfExporter testee(outputStream);
    static const interpreter::TypeHint hints[] = { interpreter::thInt, interpreter::thString, interpreter::thBool, interpreter::thFloat };

    // Test sequence
    testee.startTable(list, hints);
    testee.startRecord();
    {
        IntegerValue iv(9);
        StringValue sv("Nine");
        BooleanValue bv(false);
        FloatValue fv(31.622777);
        testee.addField(&iv, "ID", interpreter::thInt);
        testee.addField(&sv, "NAME", interpreter::thString);
        testee.addField(&bv, "MARKED", interpreter::thBool);
        testee.addField(&fv, "WAYPOINT.DIST", interpreter::thFloat);
    }
    testee.endRecord();

    testee.startRecord();
    {
        IntegerValue iv(20);
        StringValue sv("Twenty");
        BooleanValue bv(true);
        IntegerValue fv(0);
        testee.addField(&iv, "ID", interpreter::thInt);
        testee.addField(&sv, "NAME", interpreter::thString);
        testee.addField(&bv, "MARKED", interpreter::thBool);
        testee.addField(&fv, "WAYPOINT.DIST", interpreter::thFloat);
    }
    testee.endRecord();
    testee.endTable();

    // Verify against dbf file created with PCC2 c2export
    static const uint8_t EXPECTED[] = {
        0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xa1, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x49, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x4e, 0x41, 0x4d, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00,
        0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x4d, 0x41, 0x52, 0x4b, 0x45, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x57, 0x41, 0x59, 0x50, 0x4f, 0x49, 0x4e, 0x54, 0x2e, 0x44, 0x49, 0x4e, 0x00, 0x00, 0x00, 0x00,
        0x0a, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x39, 0x4e, 0x69, 0x6e, 0x65, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4e, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x33, 0x31, 0x2e, 0x36, 0x32, 0x20, 0x20, 0x20, 0x20, 0x32, 0x30, 0x54, 0x77, 0x65, 0x6e,
        0x74, 0x79, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x59, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x2e, 0x30, 0x30, 0x00
    };
    afl::base::ConstBytes_t bytes = outputStream.getContent();
    TS_ASSERT_EQUALS(bytes.size(), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(bytes.at(0), EXPECTED, sizeof(EXPECTED));
}

