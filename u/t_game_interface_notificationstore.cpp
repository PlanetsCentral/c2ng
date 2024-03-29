/**
  *  \file u/t_game_interface_notificationstore.cpp
  *  \brief Test for game::interface::NotificationStore
  */

#include "game/interface/notificationstore.hpp"

#include "t_game_interface.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/interface/processlisteditor.hpp"
#include "game/parser/informationconsumer.hpp"
#include "game/playerlist.hpp"
#include "game/teamsettings.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

using game::Reference;

namespace {
    class NullInformationConsumer : public game::parser::InformationConsumer {
     public:
        virtual void addMessageInformation(const game::parser::MessageInformation& /*info*/)
            { }
    };
}


/** Simple sequence test.
    This test is mostly taken from PCC2 that had more complex interaction with processes.

    A: Create two messages; one not associated with a process.
    E: Messages can be correctly retrieved, removeOrphanedMessages() works correctly. */
void
TestGameInterfaceNotificationStore::testIt()
{
    // ex IntNotifyTestSuite::testNotify
    // Environment
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::PlayerList list;
    interpreter::World world(log, tx, fs);

    // Create empty store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);
    TS_ASSERT_EQUALS(store.getNumMessages(), 0U);

    // Out-of-bounds access correctly rejected
    TS_ASSERT(store.getMessageByIndex(0) == 0);
    TS_ASSERT_EQUALS(store.getMessageHeaderText(0, tx, list), "");
    TS_ASSERT_EQUALS(store.getMessageBodyText(0, tx, list), "");
    TS_ASSERT_EQUALS(store.getMessageDisplayText(0, tx, list).getText(), "");
    {
        game::TeamSettings teams;
        afl::charset::Utf8Charset cs;
        NullInformationConsumer consumer;
        TS_ASSERT_THROWS_NOTHING(store.receiveMessageData(0, consumer, teams, true, cs));
    }

    // Add a message
    game::interface::NotificationStore::Message* msg = store.addMessage(77777, "foo\n", "bar", Reference(Reference::Ship, 77));
    TS_ASSERT(msg != 0);
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageByIndex(0), msg);
    TS_ASSERT_EQUALS(store.getMessageHeading(0, tx, list), "foo");
    TS_ASSERT_EQUALS(store.getMessageText(0, tx, list), "foo\nbar");
    TS_ASSERT_EQUALS(store.getMessageBody(msg), "bar");
    TS_ASSERT_EQUALS(store.getMessageBody(0), "");             // 0 is actually NULL
    TS_ASSERT_EQUALS(store.getMessageMetadata(0, tx, list).primaryLink, Reference(Reference::Ship, 77));
    TS_ASSERT_EQUALS(store.getMessageReplyText(0, tx, list), "> foo\n> bar\n");
    TS_ASSERT_EQUALS(store.getMessageForwardText(0, tx, list), "--- Forwarded Message ---\nfoo\nbar\n--- End Forwarded Message ---");
    TS_ASSERT_EQUALS(store.getMessageDisplayText(0, tx, list).getText(), "foo\nbar");

    // Add another message, associate that with a process
    interpreter::Process& proc = procList.create(world, "name");
    game::interface::NotificationStore::Message* msg2 = store.addMessage(proc.getProcessId(), "foo2\n", "bar2", Reference());
    TS_ASSERT(msg2 != 0);
    TS_ASSERT(msg2 != msg);
    TS_ASSERT_EQUALS(store.getNumMessages(), 2U);
    TS_ASSERT_EQUALS(store.getMessageByIndex(1), msg2);
    TS_ASSERT_EQUALS(store.getMessageHeading(1, tx, list), "foo2");
    TS_ASSERT_EQUALS(store.getMessageText(1, tx, list), "foo2\nbar2");
    TS_ASSERT_EQUALS(store.getMessageBody(msg2), "bar2");
    TS_ASSERT_EQUALS(store.getMessageDisplayText(1, tx, list).getText().substr(0, 9), "foo2\nbar2");
    TS_ASSERT(store.getMessageDisplayText(1, tx, list).getText().find("has been stopped") != String_t::npos);

    TS_ASSERT_EQUALS(store.findMessageByProcessId(proc.getProcessId()), msg2);
    TS_ASSERT(!store.findMessageByProcessId(88888));

    TS_ASSERT_EQUALS(store.findIndexByProcessId(proc.getProcessId()).orElse(9999), 1U);
    TS_ASSERT(!store.findIndexByProcessId(8888).isValid());

    // Delete first message; it has no associated process
    store.removeOrphanedMessages();
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageByIndex(0), msg2);
}

/** Test message header handling.

    A: Create a message that has a header in typical format.
    E: Check that header is correctly simplified */
void
TestGameInterfaceNotificationStore::testHeader()
{
    // environment
    afl::string::NullTranslator tx;
    game::PlayerList list;

    // create empty store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);
    TS_ASSERT_EQUALS(store.getNumMessages(), 0U);

    // add a message
    game::interface::NotificationStore::Message* msg = store.addMessage(77777, "(-s0123)<<< Ship Message >>>\nFROM: USS Kelvin\n\n", "Hi mom.", Reference(Reference::Ship, 123));
    TS_ASSERT(msg != 0);
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageByIndex(0), msg);
    TS_ASSERT_EQUALS(store.getMessageHeading(0, tx, list), "(-s) Ship Message");
    TS_ASSERT_EQUALS(store.getMessageMetadata(0, tx, list).primaryLink, Reference(Reference::Ship, 123));
}

/** Test resumeConfirmedProcesses().

    A: Create two processes with a message each. Resume one message.
    E: One process resumed, one unchanged */
void
TestGameInterfaceNotificationStore::testResume()
{
    // Environment
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::PlayerList list;
    interpreter::World world(log, tx, fs);

    // Message store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);

    // Two processes
    interpreter::Process& p1 = procList.create(world, "p1");
    interpreter::Process& p2 = procList.create(world, "p2");
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Suspended);

    // Messages for each
    store.addMessage(p1.getProcessId(), "m1", "b", Reference());
    store.addMessage(p2.getProcessId(), "m2", "b", Reference());
    TS_ASSERT_EQUALS(store.getNumMessages(), 2U);

    // Confirm m2
    game::interface::NotificationStore::Message* msg = store.findMessageByProcessId(p2.getProcessId());
    store.confirmMessage(msg, true);
    TS_ASSERT_EQUALS(store.isMessageConfirmed(msg), true);

    // Resume
    game::interface::ProcessListEditor editor(procList);
    store.resumeConfirmedProcesses(editor);
    editor.commit(procList.allocateProcessGroup());

    // Verify
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Runnable);
}

/** Test resumeConfirmedProcesses(), use general API.

    A: Create two processes with a message each. Resume one message.
    E: One process resumed, one unchanged */
void
TestGameInterfaceNotificationStore::testResume2()
{
    // Environment
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::PlayerList list;
    interpreter::World world(log, tx, fs);

    // Message store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);

    // Two processes
    interpreter::Process& p1 = procList.create(world, "p1");
    interpreter::Process& p2 = procList.create(world, "p2");
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Suspended);

    // Messages for each
    store.addMessage(p1.getProcessId(), "m1", "b", Reference());
    store.addMessage(p2.getProcessId(), "m2", "b", Reference());
    TS_ASSERT_EQUALS(store.getNumMessages(), 2U);

    TS_ASSERT(!store.getMessageMetadata(1, tx, list).flags.contains(game::msg::Mailbox::Confirmed));
    TS_ASSERT(store.getMessageActions(1).contains(game::msg::Mailbox::ToggleConfirmed));

    // Confirm m2 using general API
    size_t index = store.findIndexByProcessId(p2.getProcessId()).orElse(9999);
    TS_ASSERT_EQUALS(index, 1U);
    store.performMessageAction(index, game::msg::Mailbox::ToggleConfirmed);
    TS_ASSERT(store.getMessageMetadata(1, tx, list).flags.contains(game::msg::Mailbox::Confirmed));
    TS_ASSERT(!store.getMessageActions(1).contains(game::msg::Mailbox::ToggleConfirmed));

    // Resume
    game::interface::ProcessListEditor editor(procList);
    store.resumeConfirmedProcesses(editor);
    editor.commit(procList.allocateProcessGroup());

    // Verify
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Runnable);
}

/** Test message replacement.

    A: Create two messages with same process Id.
    E: Only one message survives. */
void
TestGameInterfaceNotificationStore::testReplace()
{
    // Environment
    afl::string::NullTranslator tx;
    game::PlayerList list;

    // Create empty store
    interpreter::ProcessList procList;
    game::interface::NotificationStore store(procList);
    TS_ASSERT_EQUALS(store.getNumMessages(), 0U);

    // Add a message
    store.addMessage(77777, "h1", "b1", Reference());
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageHeading(0, tx, list), "h1");

    // Add another message with the same Id
    store.addMessage(77777, "h2", "b2", Reference());
    TS_ASSERT_EQUALS(store.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(store.getMessageHeading(0, tx, list), "h2");
}

