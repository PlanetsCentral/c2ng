/**
  *  \file server/file/clientapplication.cpp
  *  \brief Class server::file::ClientApplication
  */

#include "server/file/clientapplication.hpp"
#include "afl/net/http/pagedispatcher.hpp"
#include "afl/net/http/protocolhandler.hpp"
#include "afl/net/name.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/directoryhandlerfactory.hpp"
#include "server/file/directorypage.hpp"
#include "server/file/utils.hpp"
#include "version.hpp"

void
server::file::ClientApplication::appMain()
{
    // Parse args
    afl::string::Translator& tx = translator();
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    afl::base::Optional<String_t> arg_command;
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "h" || p == "help") {
                help();
            } else if (p == "proxy") {
                m_networkStack.add(commandLine.getRequiredParameter(p));
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            arg_command = p;
            break;
        }
    }

    // Now, the commandLine sits at the first argument for the command. Dispatch on command.
    const String_t* pCommand = arg_command.get();
    if (!pCommand) {
        errorExit(afl::string::Format(tx("no command specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }
    if (*pCommand == "help") {
        help();
    } else if (*pCommand == "ls") {
        doList(commandLine);
    } else if (*pCommand == "cp") {
        doCopy(commandLine);
    } else if (*pCommand == "clear") {
        doClear(commandLine);
    } else if (*pCommand == "sync") {
        doSync(commandLine);
    } else if (*pCommand == "serve") {
        doServe(commandLine);
    } else {
        errorExit(afl::string::Format(tx("invalid command '%s'. Use '%s -h' for help.").c_str(), *pCommand, environment().getInvocationName()));
    }
}

void
server::file::ClientApplication::doCopy(afl::sys::CommandLineParser& cmdl)
{
    afl::string::Translator& tx = translator();
    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    DirectoryHandler* in = 0;
    DirectoryHandler* out = 0;
    CopyFlags_t flags;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                flags += CopyRecursively;
            } else if (p == "x") {
                flags += CopyExpandTarballs;
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            if (in == 0) {
                in = &dhf.createDirectoryHandler(p);
            } else if (out == 0) {
                out = &dhf.createDirectoryHandler(p);
            } else {
                errorExit(afl::string::Format(tx("too many directory names specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }                
        }
    }

    if (out == 0) {
        errorExit(afl::string::Format(tx("need two directory names (source, destination). Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    copyDirectory(*out, *in, flags);
}

void
server::file::ClientApplication::doSync(afl::sys::CommandLineParser& cmdl)
{
    afl::string::Translator& tx = translator();
    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    DirectoryHandler* in = 0;
    DirectoryHandler* out = 0;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                // ignore for symmetry with 'cp'
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            if (in == 0) {
                in = &dhf.createDirectoryHandler(p);
            } else if (out == 0) {
                out = &dhf.createDirectoryHandler(p);
            } else {
                errorExit(afl::string::Format(tx("too many directory names specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }                
        }
    }

    if (out == 0) {
        errorExit(afl::string::Format(tx("need two directory names (source, destination). Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    synchronizeDirectories(*out, *in);
}

void
server::file::ClientApplication::doList(afl::sys::CommandLineParser& cmdl)
{
    afl::string::Translator& tx = translator();
    bool opt_recursive = false;
    bool opt_long = false;
    std::vector<String_t> args;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                opt_recursive = true;
            } else if (p == "l") {
                opt_long = true;
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            args.push_back(p);
        }
    }

    if (args.empty()) {
        errorExit(afl::string::Format(tx("missing directory name to list. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    bool withHeader = (args.size() > 1 || opt_recursive);
    for (size_t i = 0, n = args.size(); i < n; ++i) {
        doList(dhf.createDirectoryHandler(args[i]), args[i], opt_recursive, opt_long, withHeader);
    }
}

void
server::file::ClientApplication::doList(DirectoryHandler& in, String_t name, bool recursive, bool longFormat, bool withHeader)
{
    afl::io::TextWriter& out = standardOutput();
    if (withHeader) {
        out.writeLine(name + ":");
    }

    // Read content
    InfoVector_t children;
    listDirectory(children, in);

    // Display content
    for (size_t i = 0, n = children.size(); i < n; ++i) {
        const DirectoryHandler::Info& ch = children[i];
        if (longFormat) {
            String_t size = "-";
            if (const int32_t* p = ch.size.get()) {
                size = afl::string::Format("%d", *p);
            }
            String_t type = "?";
            switch (ch.type) {
             case DirectoryHandler::IsUnknown:   type = "UNK";  break;
             case DirectoryHandler::IsDirectory: type = "DIR";  break;
             case DirectoryHandler::IsFile:      type = "FILE"; break;
            }
            out.writeLine(afl::string::Format("%-4s %-40s %10s  %s", type, ch.contentId.orElse("-"), size, ch.name));
        } else {
            out.writeLine(ch.name);
        }
    }
    if (withHeader) {
        out.writeLine();
    }

    if (recursive) {
        for (size_t i = 0, n = children.size(); i < n; ++i) {
            const DirectoryHandler::Info& ch = children[i];
            if (ch.type == DirectoryHandler::IsDirectory) {
                std::auto_ptr<DirectoryHandler> sub(in.getDirectory(ch));
                doList(*sub, DirectoryHandlerFactory::makePathName(name, ch.name), recursive, longFormat, withHeader);
            }
        }
    }
}

void
server::file::ClientApplication::doClear(afl::sys::CommandLineParser& cmdl)
{
    afl::string::Translator& tx = translator();
    std::vector<String_t> args;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                // Ignore for consistency; we are always recursive
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            args.push_back(p);
        }
    }

    if (args.empty()) {
        errorExit(afl::string::Format(tx("missing directory name to clear. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    for (size_t i = 0, n = args.size(); i < n; ++i) {
        removeDirectoryContent(dhf.createDirectoryHandler(args[i]));
    }
}

void
server::file::ClientApplication::doServe(afl::sys::CommandLineParser& cmdl)
{
    // Parse parameters
    afl::string::Translator& tx = translator();
    afl::base::Optional<String_t> source;
    afl::base::Optional<String_t> address;

    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
        } else if (!source.isValid()) {
            source = p;
        } else if (!address.isValid()) {
            address = p;
        } else {
            errorExit(afl::string::Format(tx("too many parameters. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
        }
    }

    const String_t* pSource = source.get();
    const String_t* pAddress = address.get();
    if (pSource == 0 || pAddress == 0) {
        errorExit(afl::string::Format(tx("too few parameters. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    // ProtocolHandlerFactory
    class MyProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        MyProtocolHandlerFactory(afl::net::http::Dispatcher& disp)
            : m_dispatcher(disp)
            { }
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::http::ProtocolHandler(m_dispatcher); }
     private:
        afl::net::http::Dispatcher& m_dispatcher;
    };

    // Set up
    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    DirectoryHandler& dh(dhf.createDirectoryHandler(*pSource));
    afl::net::http::PageDispatcher disp;
    disp.addNewPage("", new DirectoryPage(dh));
    MyProtocolHandlerFactory phf(disp);
    afl::net::Server server(m_serverNetworkStack.listen(afl::net::Name::parse(*pAddress, "8080"), 10), phf);
    server.run();
}

void
server::file::ClientApplication::help()
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(afl::string::Format(tx("PCC2 File Client v%s - (c) 2017-2021 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(tx("Usage:\n"
                                         "  %s [-h]\n"
                                         "  %$0s [--proxy=URL] COMMAND...\n"
                                         "\n"
                                         "Commands:\n"
                                         "  %$0s cp [-r] [-x] SOURCE DEST\n"
                                         "                      Copy everything from SOURCE to DEST\n"
                                         "  %$0s ls [-r] [-l] DIR...\n"
                                         "                      List content of the DIRs\n"
                                         "  %$0s sync SOURCE DEST\n"
                                         "                      Make DEST contain the same content as SOURCE\n"
                                         "  %$0s clear DIR...\n"
                                         "                      Remove content of DIRs\n"
                                         "  %$0s serve SOURCE HOST:PORT\n"
                                         "                      Serve SOURCE via HTTP for testing\n"
                                         "\n"
                                         "Command Options:\n"
                                         "  -r                  Recursive\n"
                                         "  -l                  Long format\n"
                                         "  -x                  Expand *.tgz/*.tar.gz files\n"
                                         "\n"
                                         "File specifications:\n"
                                         "  PATH                Access files within unmanaged file system\n"
                                         "  [PATH@]ca:SPEC      Access files within unmanaged content-addressable file system\n"
                                         // "  [PATH@]c2file:SPEC  Access files within managed file system\n"
                                         // "  [PATH@]ro:SPEC      Prevent write access\n"
                                         "  [PATH@]int:[UNIQ]   Internal (RAM, not persistent) file space\n"
                                         "  c2file://[USER@]HOST:PORT/PATH\n"
                                         "                      Access in a remote managed file system (c2file server)\n"
                                         "\n"
                                         "Report bugs to <Streu@gmx.de>\n").c_str(),
                                      environment().getInvocationName()));
    out.flush();
    exit(0);
}
