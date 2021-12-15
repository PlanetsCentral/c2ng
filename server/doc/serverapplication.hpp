/**
  *  \file server/doc/serverapplication.hpp
  *  \brief Class server::doc::ServerApplication
  */
#ifndef C2NG_SERVER_DOC_SERVERAPPLICATION_HPP
#define C2NG_SERVER_DOC_SERVERAPPLICATION_HPP

#include "afl/io/filesystem.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/application.hpp"
#include "afl/async/interrupt.hpp"

namespace server { namespace doc {

    /** c2doc server application.
        c2doc-server's main function consists of an instantiation of this object. */
    class ServerApplication : public server::Application {
     public:
        /** Constructor.
            \param env  Environment
            \param fs   File system
            \param net  Network stack
            \param intr Operating system interrupts */
        ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr);

        /** Destructor. */
        ~ServerApplication();

        // server::Application
        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser);
        virtual void serverMain();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual String_t getApplicationName() const;
        virtual String_t getCommandLineOptionHelp() const;

     private:
        afl::net::Name m_listenAddress;

        String_t m_directoryName;

        afl::async::Interrupt& m_interrupt;
    };

} }

#endif
