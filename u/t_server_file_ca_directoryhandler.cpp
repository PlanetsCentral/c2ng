/**
  *  \file u/t_server_file_ca_directoryhandler.cpp
  *  \brief Test for server::file::ca::DirectoryHandler
  */

#include <memory>
#include "server/file/ca/directoryhandler.hpp"

#include "t_server_file_ca.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/ca/objectstore.hpp"
#include "u/helper/callreceiver.hpp"
#include "afl/string/format.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "server/file/ca/directoryentry.hpp"

using server::file::ca::ObjectId;
using server::file::ca::ObjectStore;
using server::file::DirectoryHandler;

namespace {
    // A ReferenceUpdater that just records the update, but doesn't do anything fancy.
    // Tests using the NullReferenceUpdater will therefore not expire objects.
    class NullReferenceUpdater : public server::file::ca::ReferenceUpdater {
     public:
        virtual void updateDirectoryReference(const String_t& /*name*/, const ObjectId& newId)
            { m_id = newId; }

        const ObjectId& getId() const
            { return m_id; }

     private:
        ObjectId m_id;
    };

    // A ReferenceUpdater that unlinks the previous reference, to expire objects.
    // Tests using the RootReferenceUpdater will expire objects.
    class RootReferenceUpdater : public server::file::ca::ReferenceUpdater {
     public:
        RootReferenceUpdater(ObjectId id, ObjectStore& store)
            : m_id(id),
              m_store(store)
            { }
        virtual void updateDirectoryReference(const String_t& /*name*/, const ObjectId& newId)
            {
                m_store.unlinkObject(ObjectStore::TreeObject, m_id);
                m_id = newId;
            }
     private:
        ObjectId m_id;
        ObjectStore& m_store;
    };

    size_t countObjects(server::file::InternalDirectoryHandler::Directory& dir)
    {
        size_t count = 0;
        for (size_t i = 0, n = dir.subdirectories.size(); i < n; ++i) {
            count += dir.subdirectories[i]->files.size();
        }
        return count;
    }
}

/** Simple tests. */
void
TestServerFileCaDirectoryHandler::testSimple()
{
    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    ObjectStore store(rootHandler);

    // Testee
    server::file::ca::DirectoryHandler testee(store, ObjectId::nil, "testSimple", *new NullReferenceUpdater());

    // Store and retrieve a file
    afl::base::ConstBytes_t content = afl::string::toBytes("content");
    DirectoryHandler::Info i = testee.createFile("foo", content);
    TS_ASSERT_EQUALS(i.name, "foo");
    afl::base::Ref<afl::io::FileMapping> map1 = testee.getFile(i);
    afl::base::Ref<afl::io::FileMapping> map2 = testee.getFileByName("foo");
    TS_ASSERT(map1->get().equalContent(content));
    TS_ASSERT(map2->get().equalContent(content));
}

/** Test directory handling. */
void
TestServerFileCaDirectoryHandler::testDir()
{
    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    ObjectStore store(rootHandler);

    // Testee
    server::file::ca::DirectoryHandler testee(store, ObjectId::nil, "testSimple", *new NullReferenceUpdater());

    // Create two directories
    DirectoryHandler::Info dirInfo1 = testee.createDirectory("one");
    DirectoryHandler::Info dirInfo2 = testee.createDirectory("two");
    std::auto_ptr<DirectoryHandler> dir1(testee.getDirectory(dirInfo1));
    std::auto_ptr<DirectoryHandler> dir2(testee.getDirectory(dirInfo2));
    TS_ASSERT_EQUALS(dirInfo1.name, "one");
    TS_ASSERT_EQUALS(dirInfo1.type, DirectoryHandler::IsDirectory);
    TS_ASSERT_EQUALS(dirInfo2.name, "two");
    TS_ASSERT_EQUALS(dirInfo2.type, DirectoryHandler::IsDirectory);

    // Create a file "a" in both
    afl::base::ConstBytes_t content = afl::string::toBytes("text a");
    dir1->createFile("a", content);
    dir2->createFile("a", content);

    // Repeatedly update directory two.
    // This will juggle the reference counts a little and make the ObjectId of one and two repeatedly be equal or not.
    for (int i = 0; i < 10; ++i) {
        uint8_t varyingContent[] = {uint8_t(i)};
        dir2->createFile("b", varyingContent);
        dir2->createFile("c", varyingContent);
        dir2->removeFile("a");
        dir2->removeFile("c");
        dir2->removeFile("b");
        dir2->createFile("a", content);
    }

    // Verify content
    TS_ASSERT(dir1->getFileByName("a")->get().equalContent(content));
    TS_ASSERT(dir2->getFileByName("a")->get().equalContent(content));
    TS_ASSERT_THROWS(dir1->getFileByName("b"), std::exception);
    TS_ASSERT_THROWS(dir2->getFileByName("b"), std::exception);
}

/** Test with a predefined tree. */
void
TestServerFileCaDirectoryHandler::testTree()
{
    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    ObjectStore store(rootHandler);

    // Create files
    // - directory "/dir"
    static const uint8_t TREE39[] = {
        '1','0','0','6','4','4',' ','f','i','l','e',0,
        0x40, 0x14, 0x2d, 0x09, 0xc7, 0x2b, 0x2c, 0x25, 0x57, 0x0b, 0x98, 0x30, 0x0c, 0x27, 0xd8, 0x9c, 0x57, 0xed, 0x13, 0x2d
    };
    TS_ASSERT_EQUALS(store.addObject(ObjectStore::TreeObject, TREE39).toHex(), "397bbf059739cbfa73aad2f8bf404d04f478b38a");

    // - blob "/dir/file"
    TS_ASSERT_EQUALS(store.addObject(ObjectStore::DataObject, afl::string::toBytes("blub\n")).toHex(), "40142d09c72b2c25570b98300c27d89c57ed132d");

    // - blob "/file"
    TS_ASSERT_EQUALS(store.addObject(ObjectStore::DataObject, afl::string::toBytes("bla\n")).toHex(), "a7f8d9e5dcf3a68fdd2bfb727cde12029875260b");

    // - directory "/"
    static const uint8_t TREE9A[] = {
        0x34, 0x30, 0x30, 0x30, 0x30, 0x20, 0x64, 0x69, 0x72, 0x00, 0x39, 0x7b,
        0xbf, 0x05, 0x97, 0x39, 0xcb, 0xfa, 0x73, 0xaa, 0xd2, 0xf8, 0xbf, 0x40,
        0x4d, 0x04, 0xf4, 0x78, 0xb3, 0x8a, 0x31, 0x30, 0x30, 0x36, 0x34, 0x34,
        0x20, 0x66, 0x69, 0x6c, 0x65, 0x00, 0xa7, 0xf8, 0xd9, 0xe5, 0xdc, 0xf3,
        0xa6, 0x8f, 0xdd, 0x2b, 0xfb, 0x72, 0x7c, 0xde, 0x12, 0x02, 0x98, 0x75,
        0x26, 0x0b
    };
    TS_ASSERT_EQUALS(store.addObject(ObjectStore::TreeObject, TREE9A).toHex(), "9aa7c49a27dd00dd2bdb9ce354f9a68cf04396b9");

    // Test
    server::file::ca::DirectoryHandler testee(store, ObjectId::fromHex("9aa7c49a27dd00dd2bdb9ce354f9a68cf04396b9"), "root", *new NullReferenceUpdater());

    // Read the root directory
    class Callback : public DirectoryHandler::Callback, public CallReceiver {
     public:
        virtual void addItem(const DirectoryHandler::Info& info)
            {
                checkCall(afl::string::Format("addItem(%s,%d,%d,%s)",
                                              info.name,
                                              info.type == DirectoryHandler::IsFile ? "file"
                                              : info.type == DirectoryHandler::IsDirectory ? "dir" : "?",
                                              info.size.orElse(-1),
                                              info.contentId.orElse("-")));
            }
    };
    Callback cb;
    cb.expectCall("addItem(dir,dir,-1,-)");
    cb.expectCall("addItem(file,file,4,a7f8d9e5dcf3a68fdd2bfb727cde12029875260b)");
    testee.readContent(cb);
    cb.checkFinish();

    // Read a file
    TS_ASSERT(testee.getFileByName("file")->get().equalContent(afl::string::toBytes("bla\n")));
    TS_ASSERT_THROWS(testee.getFileByName("file1"), afl::except::FileProblemException);

    // Create a file
    DirectoryHandler::Info i = testee.createFile("z", afl::string::toBytes("zz"));
    TS_ASSERT_EQUALS(i.name, "z");
    TS_ASSERT_EQUALS(i.size.orElse(-1), 2);
    TS_ASSERT_EQUALS(i.type, DirectoryHandler::IsFile);
    TS_ASSERT_EQUALS(i.contentId.orElse("-"), "03ab48c18c76ccda62f0435e8b38ef9bf4680b98");

    // Read directory again
    cb.expectCall("addItem(dir,dir,-1,-)");
    cb.expectCall("addItem(file,file,4,a7f8d9e5dcf3a68fdd2bfb727cde12029875260b)");
    cb.expectCall("addItem(z,file,2,03ab48c18c76ccda62f0435e8b38ef9bf4680b98)");
    testee.readContent(cb);
    cb.checkFinish();

    // Creation failure
    TS_ASSERT_THROWS(testee.createFile("dir", afl::base::Nothing), afl::except::FileProblemException);
    TS_ASSERT_THROWS(testee.createDirectory("z"), afl::except::FileProblemException);
}

/** Test ordering.
    Git sorts directories as if they had a trailing slash. */
void
TestServerFileCaDirectoryHandler::testOrder()
{
    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    ObjectStore store(rootHandler);
    afl::base::Ref<NullReferenceUpdater> ref(*new NullReferenceUpdater());

    // Testee
    server::file::ca::DirectoryHandler testee(store, ObjectId::nil, "root", ref);

    // Create files.
    // git wants these files ordered as
    //    a.b
    //    a/
    //    a0
    // although the directory goes before a.b, asciibetically.
    testee.createFile("a.b", afl::string::toBytes("xyz"));
    testee.createFile("a0",  afl::string::toBytes("pqr"));
    std::auto_ptr<server::file::DirectoryHandler> sub(testee.getDirectory(testee.createDirectory("a")));
    sub->createFile("f", afl::string::toBytes("abc"));

    // Get the directory
    afl::base::Ref<afl::io::FileMapping> dirMap(store.getObject(ref->getId(), ObjectStore::TreeObject));
    afl::base::ConstBytes_t dirContent(dirMap->get());

    server::file::ca::DirectoryEntry e;
    TS_ASSERT(e.parse(dirContent));
    TS_ASSERT_EQUALS(e.getName(), "a.b");
    TS_ASSERT_EQUALS(e.getType(), server::file::DirectoryHandler::IsFile);

    TS_ASSERT(e.parse(dirContent));
    TS_ASSERT_EQUALS(e.getName(), "a");
    TS_ASSERT_EQUALS(e.getType(), server::file::DirectoryHandler::IsDirectory);

    TS_ASSERT(e.parse(dirContent));
    TS_ASSERT_EQUALS(e.getName(), "a0");
    TS_ASSERT_EQUALS(e.getType(), server::file::DirectoryHandler::IsFile);

    TS_ASSERT(dirContent.empty());
    TS_ASSERT(!e.parse(dirContent));
}

/** Test operation using reference counts.
    This sequence used to fail. */
void
TestServerFileCaDirectoryHandler::testRefCount()
{
    using server::file::DirectoryHandler;

    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    ObjectStore store(rootHandler);
    afl::base::Ref<RootReferenceUpdater> ref(*new RootReferenceUpdater(ObjectId::nil, store));

    // Testee
    server::file::ca::DirectoryHandler testee(store, ObjectId::nil, "root", ref);

    // Test setup
    static const uint8_t CONTENT[] = {'a'};
    testee.createFile("a", CONTENT);
    testee.removeFile("a");
    testee.createFile("b", CONTENT); // If this fails to increase the reference counter...
    testee.createFile("c", CONTENT);
    testee.removeFile("b");          // ...this will remove the object, causing the file 'c' to become lost.

    TS_ASSERT(testee.getFileByName("c")->get().equalContent(CONTENT));
}

/** Test subdirectories. */
void
TestServerFileCaDirectoryHandler::testSubdir()
{
    using server::file::DirectoryHandler;

    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    ObjectStore store(rootHandler);
    afl::base::Ref<RootReferenceUpdater> ref(*new RootReferenceUpdater(ObjectId::nil, store));

    // Testee
    server::file::ca::DirectoryHandler testee(store, ObjectId::nil, "root", ref);

    // Create subdirectory
    std::auto_ptr<DirectoryHandler> sub(testee.getDirectory(testee.createDirectory("sub")));

    // Create a file
    static const uint8_t CONTENT[] = {'a'};
    sub->createFile("a", CONTENT);

    // Three objects: 2 directories, 1 file
    TS_ASSERT_EQUALS(countObjects(rootDir), 3U);

    // Remove
    sub->removeFile("a");
    testee.removeDirectory("sub");

    // One object remains (root)
    TS_ASSERT_EQUALS(countObjects(rootDir), 1U);
}

/** Test copy. */
void
TestServerFileCaDirectoryHandler::testCopy()
{
    // Create test setup
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    ObjectStore store(rootHandler);

    // Testee
    server::file::ca::DirectoryHandler testee(store, ObjectId::nil, "root", *new NullReferenceUpdater());

    // Create a file
    static const uint8_t CONTENT[] = {'a'};
    DirectoryHandler::Info a = testee.createFile("a", CONTENT);
    TS_ASSERT_EQUALS(a.name, "a");
    TS_ASSERT(a.contentId.isValid());
    TS_ASSERT(a.size.isValid());

    // Copy the file
    afl::base::Optional<DirectoryHandler::Info> b = testee.copyFile(testee, a, "b");
    TS_ASSERT(b.isValid());
    TS_ASSERT(b.get()->contentId.isValid());
    TS_ASSERT(b.get()->size.isValid());
    TS_ASSERT_EQUALS(b.get()->name, "b");
    TS_ASSERT_EQUALS(*b.get()->contentId.get(), *a.contentId.get());
    TS_ASSERT_EQUALS(*b.get()->size.get(), *a.size.get());

    // Test with different target
    {
        server::file::InternalDirectoryHandler::Directory otherDir("");
        server::file::InternalDirectoryHandler otherHandler("root", otherDir);
        TS_ASSERT(!otherHandler.copyFile(testee, a, "x").isValid());
    }
    {
        server::file::InternalDirectoryHandler::Directory otherDir("");
        server::file::InternalDirectoryHandler otherHandler("root", otherDir);
        ObjectStore otherStore(otherHandler);
        server::file::ca::DirectoryHandler other(otherStore, ObjectId::nil, "root", *new NullReferenceUpdater());
        TS_ASSERT(!other.copyFile(testee, a, "x").isValid());
    }
}
