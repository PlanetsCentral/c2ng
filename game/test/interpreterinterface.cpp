/**
  *  \file game/test/interpreterinterface.cpp
  *  \brief Class game::test::InterpreterInterface
  */

#include "game/test/interpreterinterface.hpp"

afl::data::Value*
game::test::InterpreterInterface::evaluate(Scope /*scope*/, int /*id*/, String_t /*expr*/)
{
    return 0;
}

String_t
game::test::InterpreterInterface::getComment(Scope /*scope*/, int /*id*/)
{
    return String_t();
}

bool
game::test::InterpreterInterface::hasTask(Scope /*scope*/, int /*id*/)
{
    return false;
}

bool
game::test::InterpreterInterface::getHullShortName(int /*nr*/, String_t& /*out*/)
{
    return false;
}

bool
game::test::InterpreterInterface::getPlayerAdjective(int /*nr*/, String_t& /*out*/)
{
    return false;
}
