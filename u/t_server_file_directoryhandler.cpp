/**
  *  \file u/t_server_file_directoryhandler.cpp
  *  \brief Test for server::file::DirectoryHandler
  */

#include "server/file/directoryhandler.hpp"

#include <stdexcept>
#include "t_server_file.hpp"

/** Interface test. */
void
TestServerFileDirectoryHandler::testInterface()
{
    class Tester : public server::file::DirectoryHandler {
     public:
        virtual String_t getName()
            { return String_t(); }
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& /*info*/)
            { throw std::runtime_error("no ref"); }
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t /*name*/)
            { throw std::runtime_error("no ref"); }
        virtual Info createFile(String_t /*name*/, afl::base::ConstBytes_t /*content*/)
            { return Info(); }
        virtual void removeFile(String_t /*name*/)
            { }
        virtual afl::base::Optional<Info> copyFile(ReadOnlyDirectoryHandler& /*source*/, const Info& /*sourceInfo*/, String_t /*name*/)
            { return afl::base::Nothing; }
        virtual void readContent(Callback& /*callback*/)
            { }
        virtual DirectoryHandler* getDirectory(const Info& /*info*/)
            { return 0; }
        virtual Info createDirectory(String_t /*name*/)
            { return Info(); }
        virtual void removeDirectory(String_t /*name*/)
            { }
    };
    Tester t;
}
