/**
  *  \file u/t_server_interface.hpp
  *  \brief Tests for server::interface
  */
#ifndef C2NG_U_T_SERVER_INTERFACE_HPP
#define C2NG_U_T_SERVER_INTERFACE_HPP

#include <cxxtest/TestSuite.h>

class TestServerInterfaceBase : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceBaseClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceComposableCommandHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceFileBase : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testProperty();
};

class TestServerInterfaceFileBaseClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceFileBaseServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceFileGame : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceFileGameClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceFileGameServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceFormat : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceFormatClient : public CxxTest::TestSuite {
 public:
    void testChain();
};

class TestServerInterfaceFormatServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
};

class TestServerInterfaceHostCron : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceHostCronClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceMailQueue : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceMailQueueClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceMailQueueServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRoundtrip();
};

class TestServerInterfaceTalkFolder : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkFolderClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkFolderServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkForum : public CxxTest::TestSuite {
 public:
    void testIt();
    void testGetValue();
};

class TestServerInterfaceTalkForumClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkForumServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkGroup : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkGroupClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkGroupServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkNNTP : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkNNTPClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkNNTPServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkPM : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkPMClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkPMServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkPost : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkPostClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkPostServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkRender : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkRenderClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkRenderServer : public CxxTest::TestSuite {
 public:
    void testServer();
    void testRoundtrip();
};

class TestServerInterfaceTalkSyntax : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkSyntaxClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkSyntaxServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRoundtrip();
};

class TestServerInterfaceTalkThread : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkThreadClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkThreadServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkUser : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkUserClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkUserServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

#endif
