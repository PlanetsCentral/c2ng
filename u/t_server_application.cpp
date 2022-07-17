/**
  *  \file u/t_server_application.cpp
  *  \brief Test for server::Application
  */

#include "server/application.hpp"

#include "t_server.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullnetworkstack.hpp"
#include "afl/sys/internalenvironment.hpp"

/** Test simple application. */
void
TestServerApplication::testSimple()
{
    // The application:
    class Tester : public server::Application {
     public:
        Tester(const String_t& logName, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
            : Application(logName, env, fs, net)
            { }
        virtual void serverMain()
            { exit(99); }
        virtual bool handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
            { return false; }
        virtual bool handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
            { return false; }
        virtual String_t getApplicationName() const
            { return String_t(); }
        virtual String_t getCommandLineOptionHelp() const
            { return String_t(); }
    };

    // Environment and instantiation:
    afl::sys::InternalEnvironment env;
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    Tester t("test", env, fs, net);

    // Verify accessors
    TS_ASSERT_EQUALS(&t.fileSystem(), &fs);
    TS_ASSERT_EQUALS(&t.networkStack(), &net);

    // Run and verify result
    TS_ASSERT_EQUALS(t.run(), 99);
}

