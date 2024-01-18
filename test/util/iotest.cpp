/**
  *  \file test/util/iotest.cpp
  *  \brief Test for util::IO
  */

#include "util/io.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/test/testrunner.hpp"

/*
 *  Test storePascalString.
 */

// Border case: empty
AFL_TEST("util.IO:storePascalString:empty", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalString(sink, "", cs);
    a.check("01", ok);
    a.checkEqual("02", sink.getContent().size(), 1U);
    a.checkEqual("03", *sink.getContent().at(0), 0);
}

// Regular case
AFL_TEST("util.IO:storePascalString:normal", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalString(sink, "hello", cs);
    a.check("01", ok);
    a.checkEqual("02", sink.getContent().size(), 6U);
    a.check("03", sink.getContent().equalContent(afl::string::toBytes("\5hello")));
}

// Border case: 255 ch
AFL_TEST("util.IO:storePascalString:255-chars", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalString(sink, String_t(255, 'x'), cs);
    a.check("01", ok);
    a.checkEqual("02", sink.getContent().size(), 256U);
    a.checkEqual("03", *sink.getContent().at(0), 255);
    a.checkEqual("04", *sink.getContent().at(1), 'x');
    a.checkEqual("05", *sink.getContent().at(255), 'x');
}

// Border case: 256 ch
AFL_TEST("util.IO:storePascalString:256-chars", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalString(sink, String_t(256, 'x'), cs);
    a.check("01", !ok);
    a.checkEqual("02", sink.getContent().size(), 0U);
}

// Too much
AFL_TEST("util.IO:storePascalString:too-long", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalString(sink, String_t(999, 'x'), cs);
    a.check("01", !ok);
    a.checkEqual("02", sink.getContent().size(), 0U);
}

// Limit applies to encoded size!
AFL_TEST("util.IO:storePascalString:limit-encoded", a)
{
    // Build a long string
    String_t str;
    for (int i = 0; i < 128; ++i) {
        afl::charset::Utf8().append(str, 0x2248);     // U+2248 = 0xF7 = 247 in cp437
    }
    a.checkEqual("01. size", str.size(), 3*128U);

    // Pack in cp437
    afl::charset::CodepageCharset cpcs(afl::charset::g_codepage437);
    afl::io::InternalSink sink;
    bool ok = util::storePascalString(sink, str, cpcs);
    a.check("11", ok);
    a.checkEqual("12", sink.getContent().size(), 129U);
    a.checkEqual("13", *sink.getContent().at(0), 128);
    a.checkEqual("14", *sink.getContent().at(1), 247);
    a.checkEqual("15", *sink.getContent().at(128), 247);
}

/*
 *  Test storePascalStringTruncate.
 */

// Border case: empty
AFL_TEST("util.IO:storePascalStringTruncate:empty", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalStringTruncate(sink, "", cs);
    a.check("01", ok);
    a.checkEqual("02", sink.getContent().size(), 1U);
    a.checkEqual("03", *sink.getContent().at(0), 0);
}

// Regular case
AFL_TEST("util.IO:storePascalStringTruncate:normal", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalStringTruncate(sink, "hello", cs);
    a.check("01", ok);
    a.checkEqual("02", sink.getContent().size(), 6U);
    a.check("03", sink.getContent().equalContent(afl::string::toBytes("\5hello")));
}

// Border case: 255 ch
AFL_TEST("util.IO:storePascalStringTruncate:255-chars", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalStringTruncate(sink, String_t(255, 'x'), cs);
    a.check("01", ok);
    a.checkEqual("02", sink.getContent().size(), 256U);
    a.checkEqual("03", *sink.getContent().at(0), 255);
    a.checkEqual("04", *sink.getContent().at(1), 'x');
    a.checkEqual("05", *sink.getContent().at(255), 'x');
}

// Border case: 256 ch
AFL_TEST("util.IO:storePascalStringTruncate:256-chars", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalStringTruncate(sink, String_t(256, 'x'), cs);
    a.check("01", !ok);
    a.checkEqual("02", sink.getContent().size(), 256U);
    a.checkEqual("03", *sink.getContent().at(0), 255);
    a.checkEqual("04", *sink.getContent().at(1), 'x');
    a.checkEqual("05", *sink.getContent().at(255), 'x');
}

// Too much
AFL_TEST("util.IO:storePascalStringTruncate:too-long", a)
{
    afl::charset::Utf8Charset cs;
    afl::io::InternalSink sink;
    bool ok = util::storePascalStringTruncate(sink, String_t(999, 'x'), cs);
    a.check("01", !ok);
    a.checkEqual("02", sink.getContent().size(), 256U);
    a.checkEqual("03", *sink.getContent().at(0), 255);
    a.checkEqual("04", *sink.getContent().at(1), 'x');
    a.checkEqual("05", *sink.getContent().at(255), 'x');
}

// Limit applies to encoded size!
AFL_TEST("util.IO:storePascalStringTruncate:limit-encoded", a)
{
    // Build a long string
    String_t str;
    for (int i = 0; i < 128; ++i) {
        afl::charset::Utf8().append(str, 0x2248);     // U+2248 = 0xF7 = 247 in cp437
    }
    a.checkEqual("01. size", str.size(), 3*128U);

    // Pack in cp437
    afl::charset::CodepageCharset cpcs(afl::charset::g_codepage437);
    afl::io::InternalSink sink;
    bool ok = util::storePascalStringTruncate(sink, str, cpcs);
    a.check("11", ok);
    a.checkEqual("12", sink.getContent().size(), 129U);
    a.checkEqual("13", *sink.getContent().at(0), 128);
    a.checkEqual("14", *sink.getContent().at(1), 247);
    a.checkEqual("15", *sink.getContent().at(128), 247);
}

/*
 *  Test loadPascalString.
 */


// Trivial case
AFL_TEST("util.IO:loadPascalString:empty", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    static const uint8_t data[] = {0};
    afl::io::ConstMemoryStream ms(data);
    a.checkEqual("", util::loadPascalString(ms, cs), "");
}

// Normal case
AFL_TEST("util.IO:loadPascalString:normal", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    static const uint8_t data[] = {7, 'h','i',' ',0x81 /* U+00FC */, 0xDB /* U+2588 */,'x','y'};
    afl::io::ConstMemoryStream ms(data);
    a.checkEqual("", util::loadPascalString(ms, cs), "hi \xC3\xBC\xE2\x96\x88xy");
}

// Error case: truncated at length byte
AFL_TEST("util.IO:loadPascalString:missing-length", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::io::ConstMemoryStream ms(afl::base::Nothing);
    AFL_CHECK_THROWS(a, util::loadPascalString(ms, cs), afl::except::FileProblemException);
}

// Error case: truncated at payload byte
AFL_TEST("util.IO:loadPascalString:truncated", a)
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    static const uint8_t data[] = {3,'y','y'};
    afl::io::ConstMemoryStream ms(data);
    AFL_CHECK_THROWS(a, util::loadPascalString(ms, cs), afl::except::FileProblemException);
}

/*
 *  Test appendFileNameExtension.
 */

AFL_TEST("util.IO:appendFileNameExtension", a)
{
    // NullFileSystem uses PosixFileNames.
    afl::io::NullFileSystem fs;

    a.checkEqual("01", util::appendFileNameExtension(fs, "readme", "txt", false), "readme.txt");
    a.checkEqual("02", util::appendFileNameExtension(fs, "readme", "txt", true),  "readme.txt");

    a.checkEqual("11", util::appendFileNameExtension(fs, "readme.doc", "txt", false), "readme.doc");
    a.checkEqual("12", util::appendFileNameExtension(fs, "readme.doc", "txt", true),  "readme.txt");

    a.checkEqual("21", util::appendFileNameExtension(fs, ".emacs", "txt", false), ".emacs.txt");
    a.checkEqual("22", util::appendFileNameExtension(fs, ".emacs", "txt", true),  ".emacs.txt");

    a.checkEqual("31", util::appendFileNameExtension(fs, "/a/b/c", "txt", false), "/a/b/c.txt");
    a.checkEqual("32", util::appendFileNameExtension(fs, "/a/b/c", "txt", true),  "/a/b/c.txt");
}

/** Test getFileNameExtension(). */
AFL_TEST("util.IO:getFileNameExtension", a)
{
    afl::io::NullFileSystem fs;
    a.checkEqual("01", util::getFileNameExtension(fs, "file.txt"), ".txt");
    a.checkEqual("02", util::getFileNameExtension(fs, "file.tar.gz"), ".gz");
    a.checkEqual("03", util::getFileNameExtension(fs, ".hidden"), "");
    a.checkEqual("04", util::getFileNameExtension(fs, "file"), "");

    a.checkEqual("11", util::getFileNameExtension(fs, "/dir/file.txt"), ".txt");
    a.checkEqual("12", util::getFileNameExtension(fs, "/dir/file.tar.gz"), ".gz");
    a.checkEqual("13", util::getFileNameExtension(fs, "/dir/.hidden"), "");
    a.checkEqual("14", util::getFileNameExtension(fs, "/dir/file"), "");

    a.checkEqual("21", util::getFileNameExtension(fs, "/dir.ext/file.txt"), ".txt");
    a.checkEqual("22", util::getFileNameExtension(fs, "/dir.ext/file.tar.gz"), ".gz");
    a.checkEqual("23", util::getFileNameExtension(fs, "/dir.ext/.hidden"), "");
    a.checkEqual("24", util::getFileNameExtension(fs, "/dir.ext/file"), "");
}


/*
 *  Test createDirectoryTree().
 */

// Must succeed on a NullFileSystem (i.e. not fail due to directory not being created)
AFL_TEST("util.IO:createDirectoryTree:NullFileSystem", a)
{
    afl::io::NullFileSystem fs;
    AFL_CHECK_SUCCEEDS(a, util::createDirectoryTree(fs, "/foo/bar/baz"));
}

// Must succeed on an InternalFileSystem, directory must actually exist afterwards
AFL_TEST("util.IO:createDirectoryTree:InternalFileSystem", a)
{
    afl::io::InternalFileSystem fs;
    AFL_CHECK_SUCCEEDS(a("createDirectoryTree"), util::createDirectoryTree(fs, "/foo/bar/baz"));
    AFL_CHECK_SUCCEEDS(a("openFile"), fs.openFile("/foo/bar/baz/quux", afl::io::FileSystem::Create));
}

// Must succeed on an InternalFileSystem if it partially exists
AFL_TEST("util.IO:createDirectoryTree:InternalFileSystem:partial", a)
{
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/foo");
    fs.createDirectory("/foo/bar");
    AFL_CHECK_SUCCEEDS(a("createDirectory"), util::createDirectoryTree(fs, "/foo/bar/baz"));
    AFL_CHECK_SUCCEEDS(a("openFile"), fs.openFile("/foo/bar/baz/quux", afl::io::FileSystem::Create));
}

AFL_TEST("util.IO:makeSearchDirectory", a)
{
    // Prepare test setting
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/a");
    fs.createDirectory("/b");
    fs.createDirectory("/c");
    fs.openFile("/a/fa", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("1"));
    fs.openFile("/b/fa", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("2"));
    fs.openFile("/c/fc", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("3"));

    // Empty
    {
        afl::base::Ref<afl::io::Directory> dir = util::makeSearchDirectory(fs, afl::base::Nothing);
        AFL_CHECK_THROWS(a("01. empty"), dir->openFile("fa", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
    }

    // Single
    {
        String_t dirNames[] = { "/b" };
        afl::base::Ref<afl::io::Directory> dir = util::makeSearchDirectory(fs, dirNames);
        uint8_t tmp[1];
        dir->openFile("fa", afl::io::FileSystem::OpenRead)->fullRead(tmp);
        a.checkEqual("11. open single", tmp[0], '2');
        AFL_CHECK_THROWS(a("12. fail single"), dir->openFile("fx", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
    }

    // Multiple
    {
        String_t dirNames[] = { "/b", "/a", "/c" };
        afl::base::Ref<afl::io::Directory> dir = util::makeSearchDirectory(fs, dirNames);
        uint8_t tmp[1];
        dir->openFile("fa", afl::io::FileSystem::OpenRead)->fullRead(tmp);
        a.checkEqual("21. open multi", tmp[0], '2');
        dir->openFile("fc", afl::io::FileSystem::OpenRead)->fullRead(tmp);
        a.checkEqual("22. open multi", tmp[0], '3');
        AFL_CHECK_THROWS(a("23. fail multi"), dir->openFile("fx", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
    }
}
