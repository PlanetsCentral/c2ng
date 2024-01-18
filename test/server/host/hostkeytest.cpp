/**
  *  \file test/server/host/hostkeytest.cpp
  *  \brief Test for server::host::HostKey
  */

#include "server/host/hostkey.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/configuration.hpp"
#include "server/host/keystore.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"
#include <stdexcept>

using server::host::Session;
using server::host::HostKey;

namespace {
    /*
     *  Some keys for testing
     */

    const uint8_t KEY1[] = {
        0x5E, 0x04, 0x00, 0x00, 0x36, 0x07, 0x00, 0x00, 0xE7, 0x09, 0x00, 0x00,
        0x80, 0x06, 0x00, 0x00, 0x50, 0x14, 0x00, 0x00, 0xE8, 0x20, 0x00, 0x00,
        0x7B, 0x22, 0x00, 0x00, 0xB0, 0x2C, 0x00, 0x00, 0x29, 0x2E, 0x00, 0x00,
        0xE8, 0x3A, 0x00, 0x00, 0x3D, 0x40, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00,
        0xEB, 0x4B, 0x00, 0x00, 0xF0, 0x49, 0x00, 0x00, 0xE3, 0x49, 0x00, 0x00,
        0xA0, 0x5C, 0x00, 0x00, 0x31, 0x57, 0x00, 0x00, 0xC6, 0x6C, 0x00, 0x00,
        0x97, 0x5D, 0x00, 0x00, 0xC8, 0x73, 0x00, 0x00, 0xB5, 0x6B, 0x00, 0x00,
        0xC0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,
        0xA0, 0x28, 0x00, 0x00, 0xCF, 0x03, 0x00, 0x00, 0x42, 0x0A, 0x00, 0x00,
        0x6F, 0x12, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x71, 0x0C, 0x00, 0x00,
        0xC0, 0x09, 0x00, 0x00, 0x60, 0x0B, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00,
        0xA0, 0x0E, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xE0, 0x11, 0x00, 0x00,
        0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xC0, 0x16, 0x00, 0x00,
        0x60, 0x18, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0xA0, 0x1B, 0x00, 0x00,
        0x40, 0x1D, 0x00, 0x00, 0xE0, 0x1E, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00,
        0x20, 0x22, 0x00, 0x00, 0xC0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00,
        0x00, 0x27, 0x00, 0x00, 0xA0, 0x28, 0x00, 0x00, 0x22, 0x61, 0x07, 0x00
    };

    const uint8_t KEY2[] = {
        0x5E, 0x04, 0x00, 0x00, 0x36, 0x07, 0x00, 0x00, 0xE7, 0x09, 0x00, 0x00,
        0x80, 0x06, 0x00, 0x00, 0x50, 0x14, 0x00, 0x00, 0xE8, 0x20, 0x00, 0x00,
        0x7B, 0x22, 0x00, 0x00, 0xB0, 0x2C, 0x00, 0x00, 0x29, 0x2E, 0x00, 0x00,
        0xE8, 0x3A, 0x00, 0x00, 0x3D, 0x40, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00,
        0xEB, 0x4B, 0x00, 0x00, 0xF0, 0x49, 0x00, 0x00, 0xE3, 0x49, 0x00, 0x00,
        0xA0, 0x5C, 0x00, 0x00, 0x31, 0x57, 0x00, 0x00, 0xC6, 0x6C, 0x00, 0x00,
        0x97, 0x5D, 0x00, 0x00, 0xC8, 0x73, 0x00, 0x00, 0xB5, 0x6B, 0x00, 0x00,
        0xC0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,
        0xA0, 0x28, 0x00, 0x00, 0xCF, 0x03, 0x00, 0x00, 0x42, 0x0A, 0x00, 0x00,
        0x6F, 0x12, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0xB2, 0x0C, 0x00, 0x00,
        0xC0, 0x09, 0x00, 0x00, 0x60, 0x0B, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00,
        0xA0, 0x0E, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xE0, 0x11, 0x00, 0x00,
        0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xC0, 0x16, 0x00, 0x00,
        0x60, 0x18, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0xA0, 0x1B, 0x00, 0x00,
        0x40, 0x1D, 0x00, 0x00, 0xE0, 0x1E, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00,
        0x20, 0x22, 0x00, 0x00, 0xC0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00,
        0x00, 0x27, 0x00, 0x00, 0xA0, 0x28, 0x00, 0x00, 0x63, 0x61, 0x07, 0x00
    };

    const uint8_t KEY3[] = {
        0x5E, 0x04, 0x00, 0x00, 0x36, 0x07, 0x00, 0x00, 0xE7, 0x09, 0x00, 0x00,
        0x80, 0x06, 0x00, 0x00, 0x50, 0x14, 0x00, 0x00, 0xE8, 0x20, 0x00, 0x00,
        0x7B, 0x22, 0x00, 0x00, 0xB0, 0x2C, 0x00, 0x00, 0x29, 0x2E, 0x00, 0x00,
        0xE8, 0x3A, 0x00, 0x00, 0x3D, 0x40, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00,
        0xEB, 0x4B, 0x00, 0x00, 0xF0, 0x49, 0x00, 0x00, 0xE3, 0x49, 0x00, 0x00,
        0xA0, 0x5C, 0x00, 0x00, 0x31, 0x57, 0x00, 0x00, 0xC6, 0x6C, 0x00, 0x00,
        0x97, 0x5D, 0x00, 0x00, 0xC8, 0x73, 0x00, 0x00, 0xB5, 0x6B, 0x00, 0x00,
        0xC0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,
        0xA0, 0x28, 0x00, 0x00, 0xCF, 0x03, 0x00, 0x00, 0x42, 0x0A, 0x00, 0x00,
        0x6F, 0x12, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0xF3, 0x0C, 0x00, 0x00,
        0xC0, 0x09, 0x00, 0x00, 0x60, 0x0B, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00,
        0xA0, 0x0E, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xE0, 0x11, 0x00, 0x00,
        0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xC0, 0x16, 0x00, 0x00,
        0x60, 0x18, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0xA0, 0x1B, 0x00, 0x00,
        0x40, 0x1D, 0x00, 0x00, 0xE0, 0x1E, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00,
        0x20, 0x22, 0x00, 0x00, 0xC0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00,
        0x00, 0x27, 0x00, 0x00, 0xA0, 0x28, 0x00, 0x00, 0xA4, 0x61, 0x07, 0x00
    };

    const uint8_t KEY4[] = {
        0x2A, 0x04, 0x00, 0x00, 0x42, 0x0A, 0x00, 0x00, 0xB1, 0x0F, 0x00, 0x00,
        0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xC0, 0x09, 0x00, 0x00,
        0x60, 0x0B, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0xA0, 0x0E, 0x00, 0x00,
        0x40, 0x10, 0x00, 0x00, 0xE0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00,
        0x20, 0x15, 0x00, 0x00, 0xC0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00,
        0x00, 0x1A, 0x00, 0x00, 0xA0, 0x1B, 0x00, 0x00, 0x40, 0x1D, 0x00, 0x00,
        0xE0, 0x1E, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00,
        0xC0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,
        0xA0, 0x28, 0x00, 0x00, 0xCF, 0x03, 0x00, 0x00, 0x42, 0x0A, 0x00, 0x00,
        0x6F, 0x12, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00,
        0xC0, 0x09, 0x00, 0x00, 0x60, 0x0B, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00,
        0xA0, 0x0E, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xE0, 0x11, 0x00, 0x00,
        0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xC0, 0x16, 0x00, 0x00,
        0x60, 0x18, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0xA0, 0x1B, 0x00, 0x00,
        0x40, 0x1D, 0x00, 0x00, 0xE0, 0x1E, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00,
        0x20, 0x22, 0x00, 0x00, 0xC0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00,
        0x00, 0x27, 0x00, 0x00, 0xA0, 0x28, 0x00, 0x00, 0xF9, 0x4D, 0x04, 0x00
    };

    String_t toKeyFile(afl::base::ConstBytes_t content)
    {
        return String_t(136, '\0') + afl::string::fromBytes(content);
    }


    /*
     *  Test Setup
     */

    struct TestHarness {
        // Database
        afl::net::redis::InternalDatabase db;

        // Host filer
        server::file::InternalFileServer hostFile;

        // User filer
        server::file::InternalFileServer userFile;

        // Mail queue
        afl::net::NullCommandHandler mailSink;
        server::interface::MailQueueClient mailQueue;

        // Checkturn
        util::ProcessRunner checkturnRunner;

        // File system
        afl::io::NullFileSystem fs;

        // Configuration
        server::host::Configuration config;

        // Root to tie it all together
        server::host::Root root;

        TestHarness()
            : db(), hostFile(), userFile(), mailSink(), mailQueue(mailSink),
              checkturnRunner(), fs(), config(), root(db, hostFile, userFile, mailQueue, checkturnRunner, fs, config)
            { }
    };

    void createFiles(afl::net::CommandHandler& file)
    {
        server::interface::FileBaseClient c(file);

        // Create home directory for user 'first'
        c.createDirectory("u");
        c.createDirectoryAsUser("u/first", "1001");

        // Create some key files
        //   key1 appears twice because why not
        c.createDirectory("u/first/k1");
        c.putFile("u/first/k1/fizz.bin", toKeyFile(KEY1));
        c.createDirectory("u/first/k1a");
        c.putFile("u/first/k1a/fizz.bin", toKeyFile(KEY1));
        c.createDirectory("u/first/k3");
        c.putFile("u/first/k3/fizz.bin", toKeyFile(KEY3));
    }

    void createDatabase(afl::net::CommandHandler& db)
    {
        using afl::net::redis::StringKey;
        using afl::net::redis::StringSetKey;
        using afl::net::redis::HashKey;
        using afl::net::redis::Subtree;
        using game::v3::RegistrationKey;

        // Create a user
        StringSetKey(db, "user:all").add("1001");
        StringKey(db, "user:1001:name").set("first");

        // Give him some keys
        {
            server::host::KeyStore store(Subtree(db, "user:1001:key:"), server::host::Configuration());
            RegistrationKey k(std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()));

            // - key 1 used on game 3
            k.unpackFromBytes(KEY1);
            store.addKey(k, 9999, 3);

            // - key 2 used on no game
            k.unpackFromBytes(KEY2);
            store.addKey(k, 9998, 0);
        }

        // Create another user and give them one key
        StringSetKey(db, "user:all").add("1002");
        StringKey(db, "user:1002:name").set("second");
        {
            server::host::KeyStore store(Subtree(db, "user:1002:key:"), server::host::Configuration());
            RegistrationKey k(std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()));
            k.unpackFromBytes(KEY2);
            store.addKey(k, 9998, 3);
        }

        // Create the mentioned game
        StringKey(db, "game:3:name").set("Third Game");
    }
}

/** Test use in admin context.
    A: perform getKey, listKey operations.
    E: operation fails */
AFL_TEST("server.host.HostKey:admin", a)
{
    TestHarness h;
    Session s;
    HostKey testee(s, h.root);

    // Cannot get
    AFL_CHECK_THROWS(a("01. getKey"), testee.getKey("x"), std::exception);

    // Cannot list
    HostKey::Infos_t list;
    AFL_CHECK_THROWS(a("11. listKeys"), testee.listKeys(list), std::exception);
}

/** Test use in user context, normal case.
    A: create complete test setup. Perform getKey, listKey operations.
    E: list produces expected values; every listed key can be retrieved. */
AFL_TEST("server.host.HostKey:normal", a)
{
    TestHarness h;
    createFiles(h.userFile);
    createDatabase(h.db);
    Session s;
    s.setUser("1001");
    HostKey testee(s, h.root);

    HostKey::Infos_t list;
    AFL_CHECK_SUCCEEDS(a("01. listKeys"), testee.listKeys(list));
    a.checkEqual("02. size", list.size(), 3U);

    // Verify listed keys
    //   every key listed once
    //   every key retrievable
    // We don't hardcode key Ids here.
    bool seen1 = false, seen2 = false, seen3 = false;
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i].label2 == "Key 1") {
            // Key 1: seen in game, twice in FS
            a.check("11. seen1", !seen1);
            a.checkEqual("12. getKey", testee.getKey(list[i].keyId), toKeyFile(KEY1));
            a.checkEqual("13. filePathName", list[i].filePathName.isValid(), true);
            a.checkEqual("14. fileUseCount", list[i].fileUseCount.orElse(0), 2);
            a.checkEqual("15. lastGame",     list[i].lastGame.orElse(0), 3);
            a.checkEqual("16. lastGameName", list[i].lastGameName.orElse(""), "Third Game");
        } else if (list[i].label2 == "Key 2") {
            // Key 2: seen in DB but not in game, not in FS
            a.check("17. seen2", !seen2);
            a.checkEqual("18. getKey", testee.getKey(list[i].keyId), toKeyFile(KEY2));
            a.checkEqual("19. filePathName", list[i].filePathName.isValid(), false);
            a.checkEqual("20. lastGameName", list[i].lastGameName.isValid(), false);
        } else if (list[i].label2 == "Key 3") {
            // Key 3: seen in FS
            a.check("21. seen3", !seen3);
            a.checkEqual("22. getKey", testee.getKey(list[i].keyId), toKeyFile(KEY3));
            a.checkEqual("23. filePathName", list[i].filePathName.orElse(""), "u/first/k3");
            a.checkEqual("24. lastGameName", list[i].lastGameName.isValid(), false);
        } else {
            a.checkEqual("25. label2", list[i].label2, "Key 1");
        }
    }

    // Invalid ID
    AFL_CHECK_THROWS(a("31. getKey"), testee.getKey("x"), std::exception);
}

/** Test use in user context, error case.
    A: create test setup where listKeyInfo on the filer fails. Perform, listKey operation.
    E: list produces expected values, failure of filer is not given to user. */
AFL_TEST("server.host.HostKey:listKey:filer-error", a)
{
    TestHarness h;
    createFiles(h.userFile);
    createDatabase(h.db);
    Session s;
    s.setUser("1002");
    HostKey testee(s, h.root);

    // Fetch key list; must retrieve one key.
    HostKey::Infos_t list;
    AFL_CHECK_SUCCEEDS(a("01. listKeys"), testee.listKeys(list));
    a.checkEqual("02. size", list.size(), 1U);

    // Verify listed keys
    a.checkEqual("11. label2", list[0].label2, "Key 2");
    a.checkEqual("12. getKey", testee.getKey(list[0].keyId), toKeyFile(KEY2));
}

/** Test use of a registered key. */
AFL_TEST("server.host.HostKey:reg", a)
{
    TestHarness h;

    using afl::net::redis::StringKey;
    using afl::net::redis::StringSetKey;
    using afl::net::redis::Subtree;
    using game::v3::RegistrationKey;

    // Create a user
    StringSetKey(h.db, "user:all").add("1001");
    StringKey(h.db, "user:1001:name").set("first");

    // Give him a key
    {
        server::host::KeyStore store(Subtree(h.db, "user:1001:key:"), server::host::Configuration());
        RegistrationKey k(std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()));
        k.unpackFromBytes(KEY4);
        store.addKey(k, 9999, 0);
    }

    // Retrieve the key
    Session s;
    s.setUser("1001");
    HostKey testee(s, h.root);

    // Fetch key list; must retrieve one key.
    HostKey::Infos_t list;
    AFL_CHECK_SUCCEEDS(a("01. listKeys"), testee.listKeys(list));
    a.checkEqual("02. size", list.size(), 1U);
    a.checkEqual("03. isRegistered", list[0].isRegistered, true);
    a.checkEqual("04. label1",       list[0].label1, "Reg");
    a.checkEqual("05. label2",       list[0].label2, "Key");
}

/** Test behaviour with empty database. */
AFL_TEST("server.host.HostKey:empty", a)
{
    // Empty environment
    TestHarness h;
    Session s;
    s.setUser("1001");
    HostKey testee(s, h.root);

    // Must retrieve empty list
    HostKey::Infos_t list;
    AFL_CHECK_SUCCEEDS(a("01. listKeys"), testee.listKeys(list));
    a.checkEqual("02. size", list.size(), 0U);
}

/** Test behaviour with empty database, but enabled key-generation. */
AFL_TEST("server.host.HostKey:generate", a)
{
    // Empty environment
    TestHarness h;

    // Configure it. We need an extra root to pass our configuration in
    h.config.keyTitle = "Test";
    server::host::Root root(h.db, h.hostFile, h.userFile, h.mailQueue, h.checkturnRunner, h.fs, h.config);
    Session s;
    s.setUser("1001");

    // Must retrieve one key tagged as server key
    HostKey::Infos_t list;
    AFL_CHECK_SUCCEEDS(a("01. listKeys"), HostKey(s, root).listKeys(list));
    a.checkEqual("02. size", list.size(), 1U);
    a.checkEqual("03. isServerKey",  list[0].isServerKey, true);
    a.checkEqual("04. isRegistered", list[0].isRegistered, true);
    a.checkEqual("05. label1",       list[0].label1, "Test");

    // Key must be retrievable
    String_t key;
    AFL_CHECK_SUCCEEDS(a("11. getKey"), key = HostKey(s, root).getKey(list[0].keyId));

    // Different user must receive a different key
    s.setUser("1002");
    HostKey::Infos_t list2;
    AFL_CHECK_SUCCEEDS(a("21. listKeys"), HostKey(s, root).listKeys(list2));
    a.checkEqual("22. size", list2.size(), 1U);
    a.checkEqual("23. isServerKey",  list2[0].isServerKey, true);
    a.checkEqual("24. isRegistered", list2[0].isRegistered, true);
    a.checkEqual("25. label1",       list2[0].label1, "Test");

    a.checkDifferent("31. keyId",  list[0].keyId, list2[0].keyId);
    a.checkDifferent("32. label2", list[0].label2, list2[0].label2);

    // Must not be able to retrieve previous user's key...
    String_t key2;
    AFL_CHECK_THROWS(a("41. getKey"), HostKey(s, root).getKey(list[0].keyId), std::exception);

    // ...but must be able to retrieve our key, which must differ from previous user's.
    AFL_CHECK_SUCCEEDS(a("51. getKey"), key2 = HostKey(s, root).getKey(list2[0].keyId));
    a.checkDifferent("52. different keys", key2, key);
}
