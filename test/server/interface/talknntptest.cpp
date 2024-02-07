/**
  *  \file test/server/interface/talknntptest.cpp
  *  \brief Test for server::interface::TalkNNTP
  */

#include "server/interface/talknntp.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.TalkNNTP")
{
    class Tester : public server::interface::TalkNNTP {
     public:
        virtual void listNewsgroups(afl::container::PtrVector<Info>& /*result*/)
            { }
        virtual Info findNewsgroup(String_t /*newsgroupName*/)
            { return Info(); }
        virtual int32_t findMessage(String_t /*rfcMsgId*/)
            { return 0; }
        virtual void listMessages(int32_t /*forumId*/, afl::data::IntegerList_t& /*result*/)
            { }
        virtual afl::data::Hash::Ref_t getMessageHeader(int32_t /*messageId*/)
            { throw "no ref"; }
        virtual void getMessageHeader(afl::base::Memory<const int32_t> /*messageIds*/, afl::data::Segment& /*results*/)
            { }
        virtual void listNewsgroupsByGroup(String_t /*groupId*/, afl::data::StringList_t& /*result*/)
            { }
    };
    Tester t;
}
