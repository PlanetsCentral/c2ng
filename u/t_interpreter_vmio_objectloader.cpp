/**
  *  \file u/t_interpreter_vmio_objectloader.cpp
  *  \brief Test for interpreter::vmio::ObjectLoader
  */

#include "interpreter/vmio/objectloader.hpp"

#include "t_interpreter_vmio.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/vmio/nullloadcontext.hpp"
#include "interpreter/arraydata.hpp"

/** Loading bytecode objects. */
void
TestInterpreterVmioObjectLoader::testLoadBCO()
{
    interpreter::vmio::NullLoadContext ctx;
    afl::charset::Utf8Charset cs;

    // Testee
    interpreter::vmio::ObjectLoader testee(cs, ctx);

    // Test data
    static const uint8_t FILE[] = {
        /* Header: not parsed by ObjectLoader */
        /* 0x43, 0x43, 0x6f, 0x62, 0x6a, 0x1a, 0x64, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, */

        /* A byte code object with Id 1:
             Local i
             For i:=1 To 10 Do Print "hello" */
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x0c, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
        0x00, 0x06, 0x05, 0x00, 0x00, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x01, 0x49, 0xff, 0xff, 0x07,
        0x00, 0x00, 0x00, 0x01, 0x0a, 0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0a, 0x00, 0x06,
        0x00, 0x00, 0x00, 0x10, 0x01, 0x0c, 0x00, 0x0e, 0x04, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x0d,
        0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x02, 0x03, 0x00, 0x07, 0x04, 0x74, 0x2e, 0x71,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00
    };
    afl::base::Ref<afl::io::Stream> s(*new afl::io::ConstMemoryStream(FILE));
    testee.load(s);

    // Verify
    afl::base::Ref<interpreter::BytecodeObject> bco(testee.getBCO(1));

    TS_ASSERT_EQUALS(bco->getFileName(), "t.q");
    TS_ASSERT_EQUALS(bco->getSubroutineName(), "");
    TS_ASSERT_EQUALS(bco->getMinArgs(), 0U);
    TS_ASSERT_EQUALS(bco->getMaxArgs(), 0U);

    TS_ASSERT_EQUALS(bco->getLineNumber(0), 1U);
    TS_ASSERT_EQUALS(bco->getLineNumber(1), 1U);
    TS_ASSERT_EQUALS(bco->getLineNumber(2), 2U);
    TS_ASSERT_EQUALS(bco->getLineNumber(10), 2U);
    TS_ASSERT_EQUALS(bco->getLineNumber(100), 2U);

    TS_ASSERT_EQUALS(bco->getNumInstructions(), 12U);
    TS_ASSERT_EQUALS((*bco)(0).major, interpreter::Opcode::maPush);
    TS_ASSERT_EQUALS((*bco)(0).minor, interpreter::Opcode::sBoolean);
    TS_ASSERT_EQUALS((*bco)(0).arg,   0xFFFFU);
    TS_ASSERT_EQUALS((*bco)(5).major, interpreter::Opcode::maBinary);
    TS_ASSERT_EQUALS((*bco)(5).minor, interpreter::biCompareLE);
    TS_ASSERT_EQUALS((*bco)(5).arg,   0);

    TS_ASSERT_EQUALS(bco->names().getNumNames(), 1U);
    TS_ASSERT_EQUALS(bco->getName(0), "I");

    TS_ASSERT_EQUALS(bco->literals().size(), 1U);
    TS_ASSERT_EQUALS(interpreter::toString(bco->getLiteral(0), true), "\"hello\"");
}

/** Loading bytecode and hash object. */
void
TestInterpreterVmioObjectLoader::testLoadHash()
{
    interpreter::vmio::NullLoadContext ctx;
    afl::charset::Utf8Charset cs;

    // Testee
    interpreter::vmio::ObjectLoader testee(cs, ctx);

    // Test data
    static const uint8_t FILE[] = {
        /* VM header:
        0x43, 0x43, 0x76, 0x6d, 0x31, 0x31, 0x2d, 0x31, 0x31, 0x2d, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31,
        0x3a, 0x31, 0x31, 0x3a, 0x31, 0x31, 0x1a, 0x64, 0x04, 0x00, 0x01, 0x00, 0x07, 0x00, */

        /* #2 BCO, 187 bytes (0xbb) */
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x0b, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00,
        0x00, 0x00, 0x78, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x76, 0x6d, 0x79, 0x06, 0x53, 0x41, 0x56, 0x45,
        0x4d, 0x45, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x09, 0x05, 0x00, 0x00,
        0x0d, 0x0b, 0x01, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x05, 0x02, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x09, 0x05, 0x00, 0x00, 0x0d, 0x0b, 0x01, 0x41,
        0x46, 0x4f, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,

        /* #1 BCO, 206 bytes (0xce) */
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xce, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x0f, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x07, 0x02, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00,
        0x00, 0x00, 0x78, 0x79, 0x03, 0x46, 0x4f, 0x4f, 0x01, 0x51, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x09, 0x0b, 0x00, 0x00, 0x15, 0x0b, 0x01, 0x00, 0x01, 0x0a, 0x01, 0x00, 0x05, 0x00, 0x01, 0x00,
        0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0b, 0x05, 0x02, 0x00, 0x05, 0x00, 0x02, 0x00,
        0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0b, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x04, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,

        /* #3 Hash, 40 bytes (0x28) */
        0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x01, 0x78, 0x01, 0x79, 0x00, 0x02, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00,

        /* #0 Process (not relevant)
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xee, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x01, 0x00, 0x74, 0x65, 0x73, 0x74,
        0x73, 0x74, 0x6d, 0x74, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x03, 0x00, 0x00, 0x00, 0x01, 0x51,
        0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x03, 0x00, 0x00, 0x00, 0x01, 0x41, 0x00, 0x8a, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x89, 0x01, 0x00, 0x00, 0x00 */
    };
    afl::base::Ref<afl::io::Stream> s(*new afl::io::ConstMemoryStream(FILE));
    testee.load(s);

    // Verify hash
    {
        afl::data::Hash::Ref_t h(testee.getHash(3));
        TS_ASSERT_EQUALS(h->getKeys().getNumNames(), 2U);

        int32_t iv;
        TS_ASSERT(interpreter::checkIntegerArg(iv, h->get("x")));
        TS_ASSERT_EQUALS(iv, 1);
        TS_ASSERT(interpreter::checkIntegerArg(iv, h->get("y")));
        TS_ASSERT_EQUALS(iv, 2);
        TS_ASSERT(h->get("z") == 0);
        TS_ASSERT(h->get("X") == 0);
    }

    // Verify BCOs
    {
        afl::base::Ref<interpreter::BytecodeObject> bco(testee.getBCO(2));
        TS_ASSERT_EQUALS(bco->getSubroutineName(), "FOO");
        TS_ASSERT_EQUALS(bco->getNumInstructions(), 11U);
    }
    {
        afl::base::Ref<interpreter::BytecodeObject> bco(testee.getBCO(1));
        TS_ASSERT_EQUALS(bco->getNumInstructions(), 15U);
    }
}

/** Loading bytecode and an array object. */
void
TestInterpreterVmioObjectLoader::testLoadArray()
{
    interpreter::vmio::NullLoadContext ctx;
    afl::charset::Utf8Charset cs;

    // Testee
    interpreter::vmio::ObjectLoader testee(cs, ctx);

    // Test data
    /* Created using
         sub foo(a)
            saveme "array.vm"
         endsub

         local rr(5,5)
         rr(3,3) := 1
         rr(2,2) := "a"
         foo rr
       in PCC2's teststmt. */
    static const uint8_t FILE[] = {
        /* VM header:
        0x43, 0x43, 0x76, 0x6d, 0x31, 0x31, 0x2d, 0x31, 0x31, 0x2d, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31,
        0x3a, 0x31, 0x31, 0x3a, 0x31, 0x31, 0x1a, 0x64, 0x04, 0x00, 0x01, 0x00, 0x07, 0x00, */

        /* #2 BCO, 126 bytes (0x7e) */
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x06, 0x08, 0x00, 0x00, 0x00, 0x61, 0x72, 0x72, 0x61, 0x79, 0x2e, 0x76, 0x6d, 0x06, 0x53,
        0x41, 0x56, 0x45, 0x4d, 0x45, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
        0x05, 0x01, 0x41, 0x46, 0x4f, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,

        /* #1 BCO, 216 bytes (0xd8) */
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xd8, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x13, 0x00, 0x00, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x07, 0x02, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00, 0x00, 0x00, 0x61, 0x03, 0x46, 0x4f,
        0x4f, 0x02, 0x52, 0x52, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x0b, 0x05, 0x00, 0x06, 0x00,
        0x05, 0x00, 0x06, 0x00, 0x02, 0x00, 0x13, 0x0b, 0x01, 0x00, 0x01, 0x0a, 0x03, 0x00, 0x06, 0x00,
        0x03, 0x00, 0x06, 0x00, 0x01, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0b, 0x05,
        0x02, 0x00, 0x06, 0x00, 0x02, 0x00, 0x06, 0x00, 0x01, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x0b, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x05,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,

        /* #3 Array, 147 bytes (0x93) */
        0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x93, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x13, 0x00, 0x00, 0x00, 0x73, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00,
        0x00, 0x00, 0x61,

        /* #0 Process (not relevant
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x01, 0x00, 0x74, 0x65, 0x73, 0x74,
        0x73, 0x74, 0x6d, 0x74, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00, 0x02, 0x52,
        0x52, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00, 0x01, 0x41, 0x00, 0x8a, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x89, 0x01, 0x00, 0x00, 0x00 */
    };
    afl::base::Ref<afl::io::Stream> s(*new afl::io::ConstMemoryStream(FILE));
    testee.load(s);

    // Verify hash
    {
        afl::base::Ref<interpreter::ArrayData> a(testee.getArray(3));
        TS_ASSERT_EQUALS(a->getNumDimensions(), 2U);
        TS_ASSERT_EQUALS(a->getDimension(0), 5U);
        TS_ASSERT_EQUALS(a->getDimension(1), 5U);

        int32_t iv;
        String_t sv;
        TS_ASSERT(interpreter::checkIntegerArg(iv, a->content[5*3+3]));
        TS_ASSERT_EQUALS(iv, 1);
        TS_ASSERT(interpreter::checkStringArg(sv, a->content[5*2+2]));
        TS_ASSERT_EQUALS(sv, "a");

        TS_ASSERT_EQUALS(a->content.size(), size_t(5*3+3+1));
    }

    // Verify BCOs
    {
        afl::base::Ref<interpreter::BytecodeObject> bco(testee.getBCO(2));
        TS_ASSERT_EQUALS(bco->getSubroutineName(), "FOO");
        TS_ASSERT_EQUALS(bco->getNumInstructions(), 3U);
    }
    {
        afl::base::Ref<interpreter::BytecodeObject> bco(testee.getBCO(1));
        TS_ASSERT_EQUALS(bco->getNumInstructions(), 19U);
    }
}

