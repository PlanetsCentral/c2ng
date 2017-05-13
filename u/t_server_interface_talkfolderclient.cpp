/**
  *  \file u/t_server_interface_talkfolderclient.cpp
  *  \brief Test for server::interface::TalkFolderClient
  */

#include <memory>
#include "server/interface/talkfolderclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple test. */
void
TestServerInterfaceTalkFolderClient::testIt()
{
    CommandHandlerMock mock;
    server::interface::TalkFolderClient testee(mock);

    // getFolders
    {
        afl::data::IntegerList_t result;
        mock.expectCall("FOLDERLS");
        mock.provideReturnValue(0);
        testee.getFolders(result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("FOLDERLS");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(2).pushBackInteger(100))));
        testee.getFolders(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 2);
        TS_ASSERT_EQUALS(result[2], 100);
    }

    // getInfo
    {
        mock.expectCall("FOLDERSTAT|103");
        mock.provideReturnValue(0);
        server::interface::TalkFolder::Info i = testee.getInfo(103);
        TS_ASSERT_EQUALS(i.name, "");
        TS_ASSERT_EQUALS(i.description, "");
        TS_ASSERT_EQUALS(i.numMessages, 0);
        TS_ASSERT_EQUALS(i.isFixedFolder, false);
        TS_ASSERT_EQUALS(i.hasUnreadMessages, false);
    }
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("name", server::makeStringValue("The Name"));
        in->setNew("description", server::makeStringValue("Description..."));
        in->setNew("messages", server::makeIntegerValue(42));
        in->setNew("fixed", server::makeIntegerValue(true));
        in->setNew("unread", server::makeIntegerValue(true));
        mock.expectCall("FOLDERSTAT|104");
        mock.provideReturnValue(new HashValue(in));

        server::interface::TalkFolder::Info out = testee.getInfo(104);
        TS_ASSERT_EQUALS(out.name, "The Name");
        TS_ASSERT_EQUALS(out.description, "Description...");
        TS_ASSERT_EQUALS(out.numMessages, 42);
        TS_ASSERT_EQUALS(out.isFixedFolder, true);
        TS_ASSERT_EQUALS(out.hasUnreadMessages, true);
    }

    // getInfos
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("name", server::makeStringValue("N"));
        in->setNew("description", server::makeStringValue("D"));
        in->setNew("messages", server::makeIntegerValue(23));
        in->setNew("fixed", server::makeIntegerValue(true));
        in->setNew("unread", server::makeIntegerValue(false));

        mock.expectCall("FOLDERMSTAT|50|105");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().pushBackNew(0).pushBackNew(new HashValue(in)))));

        static const int32_t ufids[] = { 50, 105 };
        afl::container::PtrVector<server::interface::TalkFolder::Info> out;
        testee.getInfo(ufids, out);

        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT(out[0] == 0);
        TS_ASSERT(out[1] != 0);
        TS_ASSERT_EQUALS(out[1]->name, "N");
        TS_ASSERT_EQUALS(out[1]->description, "D");
        TS_ASSERT_EQUALS(out[1]->numMessages, 23);
        TS_ASSERT_EQUALS(out[1]->isFixedFolder, true);
        TS_ASSERT_EQUALS(out[1]->hasUnreadMessages, false);
    }

    // create
    mock.expectCall("FOLDERNEW|N");
    mock.provideReturnValue(server::makeIntegerValue(108));
    TS_ASSERT_EQUALS(testee.create("N", afl::base::Nothing), 108);
    {
        mock.expectCall("FOLDERNEW|Nam.|description|Desc.");
        mock.provideReturnValue(server::makeIntegerValue(109));

        const String_t args[] = { "description", "Desc." };
        TS_ASSERT_EQUALS(testee.create("Nam.", args), 109);
    }

    // remove
    mock.expectCall("FOLDERRM|105");
    mock.provideReturnValue(0);
    testee.remove(105);

    // configure
    mock.expectCall("FOLDERSET|109");
    mock.provideReturnValue(0);
    testee.configure(109, afl::base::Nothing);
    {
        mock.expectCall("FOLDERSET|109|description|Desc2.");
        mock.provideReturnValue(0);

        const String_t args[] = { "description", "Desc2." };
        testee.configure(109, args);
    }

    // getPMs
    {
        mock.expectCall("FOLDERLSPM|109");
        mock.provideReturnValue(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(testee.getPMs(109, server::interface::TalkFolder::ListParameters()));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }
    {
        mock.expectCall("FOLDERLSPM|109|LIMIT|5|3|SORT|subject");
        server::interface::TalkFolder::ListParameters ps;
        ps.mode = ps.WantRange;
        ps.start = 5;
        ps.count = 3;
        ps.sortKey = "subject";
        mock.provideReturnValue(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(testee.getPMs(109, ps));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }
}
