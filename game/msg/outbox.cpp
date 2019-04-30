/**
  *  \file game/msg/outbox.cpp
  */

#include "game/msg/outbox.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"

namespace {
    /* Header line for a universal message.
       Used/recognized by other programs, don't translate. */
    const char UNIVERSAL_TEXT[] = "  <<< Universal Message >>>";

    /* Header line for a message to ourselves and others.
       Starts with a '<' to avoid PHost recognizing it as a command message.
       We need to filter it out upon reception. */
    const char CC_SELF_PREFIX[] = "<CC: ";

    /* Header line for a message to ourselves and others. */
    const char CC_PREFIX[] = "CC: ";


    /* Check whether receiver indicates a universal message.
       We consider a universal message to be one that goes to all real players (i.e. 1-11). */
    bool isUniversalReceiver(game::PlayerSet_t receivers, const game::PlayerList& players)
    {
        return receivers.contains(players.getAllPlayers());
    }

    /* Get list of all allowed receivers.
       We allow all real players plus player 0 (=host). */
    game::PlayerSet_t getAllReceivers(const game::PlayerList& players)
    {
        return players.getAllPlayers() + 0;
    }

    /* Get "TO:" line for a receiver bitfield. */
    String_t getReceiverText(game::PlayerSet_t bits, afl::string::Translator& tx, const game::PlayerList& players)
    {
        /* Note: do not translate "Host" here, because this function is
           also used to generate title lines for sent messages */
        bits &= getAllReceivers(players);
        if (bits.empty()) {
            // Message will not be sent, so we can translate this
            return tx.translateString("Nobody");
        } else if (bits.isUnitSet()) {
            // One receiver
            for (int i = 0; i <= game::MAX_PLAYERS; ++i) {
                if (bits.contains(i)) {
                    if (i == 0) {
                        return "Host";
                    } else {
                        return players.getPlayerName(i, game::Player::LongName);
                    }
                }
            }
            return "Huh?";
        } else {
            // Many receivers
            String_t rv;
            for (int i = 0; i <= game::MAX_PLAYERS; ++i) {
                if (bits.contains(i)) {
                    if (rv.length()) {
                        rv += " ";
                    }
                    if (i == 0) {
                        rv += "Host";
                    } else {
                        rv += afl::string::Format("%d", i);
                    }
                }
            }
            return rv;
        }
    }

    String_t maybeStripHeaders(String_t msg, game::PlayerSet_t rec)
    {
        /* strip headers only from unicast messages */
        if (!rec.isUnitSet()) {
            return msg;
        }

        const String_t firstLine = afl::string::strFirst(msg, "\n");
        if (firstLine == UNIVERSAL_TEXT
            || firstLine.compare(0, sizeof(CC_PREFIX)-1,      CC_PREFIX,      sizeof(CC_PREFIX)-1) == 0
            || firstLine.compare(0, sizeof(CC_SELF_PREFIX)-1, CC_SELF_PREFIX, sizeof(CC_SELF_PREFIX)-1) == 0)
        {
            if (afl::string::strRemove(msg, "\n")) {
                return msg;
            } else {
                return String_t();  // silly people sending empty messages
            }
        } else {
            return msg;
        }
    }
}


struct game::msg::Outbox::Message {
    int         sender;
    String_t    text;
    PlayerSet_t receivers;

    Message(int sender, const String_t& text, PlayerSet_t receivers)
        : sender(sender), text(text), receivers(receivers)
        { }
};

// /** Construct outbox.
//     \param sender owner of this outbox: the player
//                   whom all messages contained in this
//                   messagebox belong to. */
game::msg::Outbox::Outbox()
    : Mailbox(),
      m_messages()
      // format(fWinplan)
{ }

game::msg::Outbox::~Outbox()
{ }

// Mailbox:
size_t
game::msg::Outbox::getNumMessages()
{
    // ex GOutbox::getCount
    return m_messages.size();
}

// /** Get text of message /index/ (0-based). The return value will
//     include headers. */
String_t
game::msg::Outbox::getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players)
{
    // ex GOutbox::getText
    if (index < m_messages.size()) {
        return getHeadersForDisplay(m_messages[index]->sender, m_messages[index]->receivers, tx, players) + m_messages[index]->text;
    } else {
        return String_t();
    }
}

String_t
game::msg::Outbox::getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players)
{
    // ex GOutbox::getHeading
    if (index < m_messages.size()) {
        if (isUniversalReceiver(m_messages[index]->receivers, players)) {
            return "Universal Message";
        } else {
            return afl::string::Format(tx.translateString("To: %s").c_str(),
                                       getReceiverText(m_messages[index]->receivers, tx, players));
        }
    } else {
        return String_t();
    }
}

int
game::msg::Outbox::getMessageTurnNumber(size_t /*index*/)
{
    return 0;
}

// /** Get prefix for message when physically sent. The prefix can be
//     carelessly concatenated to the message text.
//     \param index    message number
//     \param to       addressee of this incarnation of the message. */
String_t
game::msg::Outbox::getMessageSendPrefix(size_t index, int receiver,
                                        afl::string::Translator& tx,
                                        const PlayerList& players) const
{
    if (index < m_messages.size()) {
        PlayerSet_t receivers = m_messages[index]->receivers & getAllReceivers(players);

        // Universal message? (all or all+host)
        if (isUniversalReceiver(receivers, players)) {
            return String_t(UNIVERSAL_TEXT) + '\n';
        }

        // More than one receiver?
        receivers -= receiver;
        if (!receivers.empty()) {
            const char* pfx = (m_messages[index]->sender == receiver ? CC_SELF_PREFIX : CC_PREFIX);
            return pfx + getReceiverText(receivers, tx, players) + '\n';
        }
    }
    return String_t();
}

// /** Get raw text of a message. */
String_t
game::msg::Outbox::getMessageRawText(size_t index) const
{
    // ex GOutbox::getRawText
    if (index < m_messages.size()) {
        return m_messages[index]->text;
    } else {
        return String_t();
    }
}

// /** Get bitmask of receivers. */
game::PlayerSet_t
game::msg::Outbox::getMessageReceiverMask(size_t index) const
{
    // ex GOutbox::getReceiverMask
    if (index < m_messages.size()) {
        return m_messages[index]->receivers;
    } else {
        return PlayerSet_t();
    }
}

int
game::msg::Outbox::getMessageSender(size_t index) const
{
    // ex GOutbox::getSender (sort-of)
    if (index < m_messages.size()) {
        return m_messages[index]->sender;
    } else {
        return 0;
    }
}

void
game::msg::Outbox::deleteMessagesAfter(size_t index)
{
    // ex GOutbox::deleteMessagesAfter
    if (index < m_messages.size()) {
        m_messages.resize(index);
    }
}

// /** Add message (send).
//     \param text       message body without headers
//     \param receivers  receiver set */
void
game::msg::Outbox::addMessage(int sender, String_t text, PlayerSet_t receivers)
{
    // ex GOutbox::addMessage
    m_messages.pushBackNew(new Message(sender, afl::string::strRTrim(text), receivers));
}

// /** Add message (from file). Unlike addMessage, this one attempts to
//     merge multicast messages. */
void
game::msg::Outbox::addMessageFromFile(int sender, String_t text, PlayerSet_t receivers)
{
    // ex GOutbox::addMessageFromFile
    /* attempt to merge messages. Preconditions:
       - message box contains at least one message
       - receivers don't overlap
       - message bodies are identical, sans headers */
    text = afl::string::strRTrim(text);
    const String_t rawText = maybeStripHeaders(text, receivers);

    if (!m_messages.empty()
        && (m_messages.back()->sender == sender)
        && (m_messages.back()->receivers & receivers).empty()
        && (maybeStripHeaders(m_messages.back()->text, m_messages.back()->receivers) == rawText))
    {
        /* merge */
        m_messages.back()->receivers |= receivers;
        m_messages.back()->text = rawText;
    } else {
        /* don't merge */
        addMessage(sender, text, receivers);
    }
}

void
game::msg::Outbox::clear()
{
    m_messages.clear();
}

// /** Get headers to use for a message to /receivers/.
//     These headers are displayed, but not part of the stored text. */
String_t
game::msg::Outbox::getHeadersForDisplay(int sender,
                                        PlayerSet_t receivers,
                                        afl::string::Translator& tx,
                                        const PlayerList& players)
{
    // ex GOutbox::getHeadersForDisplay
    receivers &= getAllReceivers(players);

    String_t senderName = players.getPlayerName(sender, Player::LongName);
    String_t receiverText = getReceiverText(receivers, tx, players);
    String_t text = afl::string::Format(tx.translateString("<<< Sub Space Message >>>\nFROM: %s\nTO: %s\n").c_str(),
                                        senderName, receiverText);
    if (isUniversalReceiver(receivers, players)) {
        text += UNIVERSAL_TEXT;
        text += '\n';
    } else if (!receivers.isUnitSet()) {
        text += CC_PREFIX; // FIXME: i18n?
        text += receiverText;
        text += '\n';
    } else {
        // no additional header
    }
    return text;
}
