/**
  *  \file u/t_game_v3_attachmentconfiguration.cpp
  *  \brief Test for game::v3::AttachmentConfiguration
  */

#include "game/v3/attachmentconfiguration.hpp"

#include "t_game_v3.hpp"
#include "afl/sys/log.hpp"
#include "game/v3/attachmentunpacker.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"

using afl::io::ConstMemoryStream;
using game::config::UserConfiguration;
using game::v3::AttachmentUnpacker;

#define UTILDAT_PREFIX_WITH_TIMESTAMP(TIMESTAMP)                        \
    0x0d, 0x00, 0x59, 0x00,                                             \
    TIMESTAMP,  0x0c, 0x00, 0x01, 0x00, 0x04, 0x00, 0x04, 0xda, 0xb0, 0x10, 0xec, 0x94, 0x3d, 0x36, \
    0x04, 0xad, 0xe9, 0x90, 0x38, 0xd4, 0x8d, 0xb7, 0x11, 0x5e, 0xef, 0x6a, 0x0e, 0x79, 0xe8, 0x84, \
    0xc0, 0xbd, 0x6f, 0x03, 0xe7, 0xbe, 0xed, 0xeb, 0x46, 0x4c, 0x41, 0x4b, 0x30, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a

/* older timestamp */
#define OLD_TIMESTAMP       0x30, 0x31, 0x2d, 0x30, 0x31, 0x2d, 0x32, 0x30, 0x31, 0x30, 0x31, 0x32, 0x3a, 0x30, 0x30, 0x3a, 0x30, 0x31
#define OLD_UTILDAT_PREFIX  UTILDAT_PREFIX_WITH_TIMESTAMP(OLD_TIMESTAMP)

/* newer timestamp */
#define NEW_TIMESTAMP       0x30, 0x31, 0x2d, 0x30, 0x31, 0x2d, 0x32, 0x30, 0x32, 0x30, 0x31, 0x32, 0x3a, 0x30, 0x30, 0x3a, 0x30, 0x31
#define NEW_UTILDAT_PREFIX  UTILDAT_PREFIX_WITH_TIMESTAMP(NEW_TIMESTAMP)


/** Test attachment configuration, same timestamp again.
    A: create AttachmentUnpacker. Load util.dat file.
    E: reports UI needed. After saving, reports no UI needed. */
void
TestGameV3AttachmentConfiguration::testSame()
{
    // Create AttachmentUnpacker and UserConfiguration
    AttachmentUnpacker unpacker;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserConfiguration config;

    // Load util.dat file
    static const uint8_t FILE[] = {
        OLD_UTILDAT_PREFIX,
        34,0, 3+13,0,
        'f','i','l','e','.','t','x','t',0,0,0,0, 0, 'x','y','\n',
    };
    ConstMemoryStream ms(FILE);
    TS_ASSERT_THROWS_NOTHING(unpacker.loadUtilData(ms, 1, log, tx));
    TS_ASSERT_EQUALS(unpacker.getNumAttachments(), 1U);

    // Default configuration says: do user selection
    TS_ASSERT_EQUALS(checkNewAttachments(config, unpacker), false);

    // Mark processed; configuration now says: skip
    TS_ASSERT_THROWS_NOTHING(markAttachmentsProcessed(config, unpacker));
    TS_ASSERT_EQUALS(checkNewAttachments(config, unpacker), true);
}

/** Test attachment configuration, different timestamp.
    A: create AttachmentUnpacker. Load util.dat file, save it. Create another AttachmentUnpacker, load another file.
    E: reports UI needed for second AttachmentUnpacker. */
void
TestGameV3AttachmentConfiguration::testDifferent()
{
    // Create AttachmentUnpacker and UserConfiguration
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserConfiguration config;

    {
        // Load first util.dat file
        AttachmentUnpacker unpacker;
        static const uint8_t FILE[] = {
            OLD_UTILDAT_PREFIX,
            34,0, 3+13,0,
            'f','i','l','e','.','t','x','t',0,0,0,0, 0, 'x','y','\n',
        };
        ConstMemoryStream ms(FILE);
        TS_ASSERT_THROWS_NOTHING(unpacker.loadUtilData(ms, 1, log, tx));
        TS_ASSERT_EQUALS(unpacker.getNumAttachments(), 1U);
        TS_ASSERT_EQUALS(checkNewAttachments(config, unpacker), false);
        TS_ASSERT_THROWS_NOTHING(markAttachmentsProcessed(config, unpacker));
    }
    
    {
        // Load second util.dat file
        AttachmentUnpacker unpacker;
        static const uint8_t FILE[] = {
            NEW_UTILDAT_PREFIX,
            34,0, 3+13,0,
            'f','i','l','e','.','t','x','t',0,0,0,0, 0, 'x','y','\n',
        };
        ConstMemoryStream ms(FILE);
        TS_ASSERT_THROWS_NOTHING(unpacker.loadUtilData(ms, 1, log, tx));
        TS_ASSERT_EQUALS(unpacker.getNumAttachments(), 1U);
        
        // Default configuration says: do user selection
        TS_ASSERT_EQUALS(checkNewAttachments(config, unpacker), false);
    }
}

/** Test race name handling, "ask" case.
    A: load util.dat containing race names. Configure Unpack.RaceNames=ask.
    E: reports UI needed, attachment selected */
void
TestGameV3AttachmentConfiguration::testRaceNameDefault()
{
    // Create AttachmentUnpacker and UserConfiguration
    AttachmentUnpacker unpacker;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserConfiguration config;
    config.setOption("Unpack.RaceNames", "ask", game::config::ConfigurationOption::Game);

    // Load util.dat file
    static const uint8_t FILE[] = {
        OLD_UTILDAT_PREFIX,
        34,0, 3+13,0,
        'r','a','c','e','.','n','m',0,0,0,0,0, 0, 'x','y','\n',
    };
    ConstMemoryStream ms(FILE);
    TS_ASSERT_THROWS_NOTHING(unpacker.loadUtilData(ms, 1, log, tx));
    TS_ASSERT_EQUALS(unpacker.getNumAttachments(), 1U);
    AttachmentUnpacker::Attachment* att = unpacker.getAttachmentByIndex(0);

    // Verify: do user selection, attachment selected
    TS_ASSERT_EQUALS(checkNewAttachments(config, unpacker), false);
    TS_ASSERT_EQUALS(unpacker.isAttachmentSelected(att), true);
}

/** Test race name handling, "accept" case.
    A: load util.dat containing race names. Configure Unpack.RaceNames=accept.
    E: reports no UI needed, attachment selected */
void
TestGameV3AttachmentConfiguration::testRaceNameAccept()
{
    // Create AttachmentUnpacker and UserConfiguration
    AttachmentUnpacker unpacker;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserConfiguration config;
    config.setOption("Unpack.RaceNames", "accept", game::config::ConfigurationOption::Game);

    // Load util.dat file
    static const uint8_t FILE[] = {
        OLD_UTILDAT_PREFIX,
        34,0, 3+13,0,
        'r','a','c','e','.','n','m',0,0,0,0,0, 0, 'x','y','\n',
    };
    ConstMemoryStream ms(FILE);
    TS_ASSERT_THROWS_NOTHING(unpacker.loadUtilData(ms, 1, log, tx));
    TS_ASSERT_EQUALS(unpacker.getNumAttachments(), 1U);
    AttachmentUnpacker::Attachment* att = unpacker.getAttachmentByIndex(0);

    // Verify: do user selection, attachment selected
    TS_ASSERT_EQUALS(checkNewAttachments(config, unpacker), true);
    TS_ASSERT_EQUALS(unpacker.isAttachmentSelected(att), true);
}

/** Test race name handling, "reject" case.
    A: load util.dat containing race names. Configure Unpack.RaceNames=reject.
    E: reports no UI needed, attachment unselected */
void
TestGameV3AttachmentConfiguration::testRaceNameReject()
{
    // Create AttachmentUnpacker and UserConfiguration
    AttachmentUnpacker unpacker;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserConfiguration config;
    config.setOption("Unpack.RaceNames", "reject", game::config::ConfigurationOption::Game);

    // Load util.dat file
    static const uint8_t FILE[] = {
        OLD_UTILDAT_PREFIX,
        34,0, 3+13,0,
        'r','a','c','e','.','n','m',0,0,0,0,0, 0, 'x','y','\n',
    };
    ConstMemoryStream ms(FILE);
    TS_ASSERT_THROWS_NOTHING(unpacker.loadUtilData(ms, 1, log, tx));
    TS_ASSERT_EQUALS(unpacker.getNumAttachments(), 1U);
    AttachmentUnpacker::Attachment* att = unpacker.getAttachmentByIndex(0);

    // Verify: do user selection, attachment selected
    TS_ASSERT_EQUALS(checkNewAttachments(config, unpacker), true);
    TS_ASSERT_EQUALS(unpacker.isAttachmentSelected(att), false);
}

/** Test empty AttachmentUnpacker.
    A: create empty AttachmentUnpacker.
    E: returns no UI needed. */
void
TestGameV3AttachmentConfiguration::testEmpty()
{
    // Create AttachmentUnpacker and UserConfiguration
    AttachmentUnpacker unpacker;
    UserConfiguration config;

    TS_ASSERT_EQUALS(checkNewAttachments(config, unpacker), true);
}

