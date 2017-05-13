/**
  *  \file u/t_server_interface_talkgroup.cpp
  *  \brief Test for server::interface::TalkGroup
  */

#include "server/interface/talkgroup.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceTalkGroup::testIt()
{
    class Tester : public server::interface::TalkGroup {
     public:
        virtual void add(String_t /*groupId*/, const Description& /*info*/)
            { }
        virtual void set(String_t /*groupId*/, const Description& /*info*/)
            { }
        virtual String_t getField(String_t /*groupId*/, String_t /*fieldName*/)
            { return String_t(); }
        virtual void list(String_t /*groupId*/, afl::data::StringList_t& /*groups*/, afl::data::IntegerList_t& /*forums*/)
            { }
        virtual Description getDescription(String_t /*groupId*/)
            { return Description(); }
        virtual void getDescriptions(const afl::data::StringList_t& /*groups*/, afl::container::PtrVector<Description>& /*results*/)
            { }
    };
    Tester t;
}

