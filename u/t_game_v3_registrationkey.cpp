/**
  *  \file u/t_game_v3_registrationkey.cpp
  *  \brief Test for game::v3::RegistrationKey
  */

#include "game/v3/registrationkey.hpp"

#include "t_game_v3.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"

namespace {
    std::auto_ptr<afl::charset::Charset> makeCharset()
    {
        return std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepage437));
    }
}

/** Test initial state. */
void
TestGameV3RegistrationKey::testInit()
{
    game::v3::RegistrationKey testee(makeCharset());
    TS_ASSERT_EQUALS(testee.getStatus(), game::RegistrationKey::Unknown);
    TS_ASSERT_EQUALS(testee.getMaxTechLevel(game::HullTech), 6);
    TS_ASSERT_EQUALS(testee.getKeyId(), "292f010cc69d850b82a83950fb6ba06959334007");
    TS_ASSERT_DIFFERS(testee.getLine(game::RegistrationKey::Line1), "");
    TS_ASSERT_DIFFERS(testee.getLine(game::RegistrationKey::Line2), "");
    TS_ASSERT_DIFFERS(testee.getLine(game::RegistrationKey::Line3), "");
    TS_ASSERT_DIFFERS(testee.getLine(game::RegistrationKey::Line4), "");
}

void
TestGameV3RegistrationKey::testFileRoundtrip()
{
    // Environment
    static const uint8_t FILE_CONTENT[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x5e, 0x04, 0x00, 0x00, 0x36, 0x07, 0x00, 0x00,
        0xe7, 0x09, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x50, 0x14, 0x00, 0x00,
        0xe8, 0x20, 0x00, 0x00, 0x7b, 0x22, 0x00, 0x00, 0xb0, 0x2c, 0x00, 0x00,
        0x29, 0x2e, 0x00, 0x00, 0xe8, 0x3a, 0x00, 0x00, 0x3d, 0x40, 0x00, 0x00,
        0x80, 0x13, 0x00, 0x00, 0xeb, 0x4b, 0x00, 0x00, 0xf0, 0x49, 0x00, 0x00,
        0xe3, 0x49, 0x00, 0x00, 0xa0, 0x5c, 0x00, 0x00, 0x31, 0x57, 0x00, 0x00,
        0xc6, 0x6c, 0x00, 0x00, 0x97, 0x5d, 0x00, 0x00, 0xc8, 0x73, 0x00, 0x00,
        0xb5, 0x6b, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00,
        0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0x5e, 0x04, 0x00, 0x00,
        0x42, 0x0a, 0x00, 0x00, 0x5e, 0x11, 0x00, 0x00, 0x5c, 0x17, 0x00, 0x00,
        0xa9, 0x1a, 0x00, 0x00, 0xd2, 0x21, 0x00, 0x00, 0x1a, 0x27, 0x00, 0x00,
        0x00, 0x0d, 0x00, 0x00, 0x4f, 0x17, 0x00, 0x00, 0x5c, 0x17, 0x00, 0x00,
        0xd0, 0x1a, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00,
        0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00,
        0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00,
        0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00,
        0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00,
        0xfb, 0xd5, 0x07, 0x00
    };
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    dir->addStream("fizz.bin", *new afl::io::ConstMemoryStream(FILE_CONTENT));

    // Test
    game::v3::RegistrationKey testee(makeCharset());
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    testee.initFromDirectory(*dir, log, tx);

    afl::io::InternalStream out;
    testee.saveToStream(out);

    // Verify
    TS_ASSERT_EQUALS(out.getSize(), sizeof(FILE_CONTENT));
    TS_ASSERT_SAME_DATA(out.getContent().at(0), FILE_CONTENT, sizeof(FILE_CONTENT));
}

void
TestGameV3RegistrationKey::testBufferRoundtrip()
{
    // Environment
    static const uint8_t BUFFER_CONTENT[] = {
        0x5e, 0x04, 0x00, 0x00, 0x36, 0x07, 0x00, 0x00, 0xe7, 0x09, 0x00, 0x00,
        0x80, 0x06, 0x00, 0x00, 0x50, 0x14, 0x00, 0x00, 0xe8, 0x20, 0x00, 0x00,
        0x7b, 0x22, 0x00, 0x00, 0xb0, 0x2c, 0x00, 0x00, 0x29, 0x2e, 0x00, 0x00,
        0xe8, 0x3a, 0x00, 0x00, 0x3d, 0x40, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00,
        0xeb, 0x4b, 0x00, 0x00, 0xf0, 0x49, 0x00, 0x00, 0xe3, 0x49, 0x00, 0x00,
        0xa0, 0x5c, 0x00, 0x00, 0x31, 0x57, 0x00, 0x00, 0xc6, 0x6c, 0x00, 0x00,
        0x97, 0x5d, 0x00, 0x00, 0xc8, 0x73, 0x00, 0x00, 0xb5, 0x6b, 0x00, 0x00,
        0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,
        0xa0, 0x28, 0x00, 0x00, 0x5e, 0x04, 0x00, 0x00, 0x42, 0x0a, 0x00, 0x00,
        0x5e, 0x11, 0x00, 0x00, 0x5c, 0x17, 0x00, 0x00, 0xa9, 0x1a, 0x00, 0x00,
        0xd2, 0x21, 0x00, 0x00, 0x1a, 0x27, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00,
        0x4f, 0x17, 0x00, 0x00, 0x5c, 0x17, 0x00, 0x00, 0xd0, 0x1a, 0x00, 0x00,
        0x40, 0x1d, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00,
        0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00,
        0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00,
        0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00,
        0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0xfb, 0xd5, 0x07, 0x00
    };
    static_assert(sizeof(BUFFER_CONTENT) == game::v3::RegistrationKey::KEY_SIZE_BYTES, "KEY_SIZE_BYTES");

    // Test
    game::v3::RegistrationKey testee(makeCharset());
    testee.unpackFromBytes(BUFFER_CONTENT);

    uint8_t out[game::v3::RegistrationKey::KEY_SIZE_BYTES];
    testee.packIntoBytes(out);

    // Verify
    TS_ASSERT_SAME_DATA(out, BUFFER_CONTENT, sizeof(BUFFER_CONTENT));
}

