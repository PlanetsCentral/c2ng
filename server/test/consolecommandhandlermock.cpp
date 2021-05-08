/**
  *  \file server/test/consolecommandhandlermock.cpp
  *  \brief Class server::test::CallReceiver
  */

#include "server/test/consolecommandhandlermock.hpp"
#include "server/types.hpp"
#include "interpreter/error.hpp"

server::test::ConsoleCommandHandlerMock::ConsoleCommandHandlerMock(afl::test::Assert a)
    : CallReceiver(a)
{ }

bool
server::test::ConsoleCommandHandlerMock::call(const String_t& cmd, interpreter::Arguments args, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& result)
{
    // Verify that this is the correct call
    String_t text = cmd;
    while (args.getNumArgs() > 0) {
        text += "|";
        text += server::toString(args.getNext());
    }
    checkCall(text);

    // Check
    Mode mode = consumeReturnValue<Mode>();
    std::auto_ptr<afl::data::Value> p(consumeReturnValue<afl::data::Value*>());

    // This is intentionally conditional to mirror that most commands that produce a null return value do not touch result.
    if (p.get() != 0) {
        result = p;
    }

    switch (mode) {
     case Success:
        return true;
     case Unrecognized:
        return false;
     case Failure:
        throw interpreter::Error("Failure");
    }
    return false;
}

void
server::test::ConsoleCommandHandlerMock::provideReturnValue(Mode mode, afl::data::Value* p)
{
    CallReceiver::provideReturnValue(mode);
    CallReceiver::provideReturnValue(p);
}
