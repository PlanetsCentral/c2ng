/**
  *  \file client/si/outputstate.cpp
  */

#include "client/si/outputstate.hpp"

client::si::OutputState::OutputState()
    : m_process(),
      m_target(NoChange)
{ }

void
client::si::OutputState::set(RequestLink2 p, Target t)
{
    m_process = p;
    m_target = t;
}

client::si::RequestLink2
client::si::OutputState::getProcess() const
{
    return m_process;
}

client::si::OutputState::Target
client::si::OutputState::getTarget() const
{
    return m_target;
}

String_t
client::si::OutputState::toString(OutputState::Target target)
{
    const char* result = "?";
    switch (target) {
     case NoChange:     result = "NoChange";     break;
     case ExitProgram:  result = "ExitProgram";  break;
     case ExitGame:     result = "ExitGame";     break;
     case PlayerScreen: result = "PlayerScreen"; break;
     case ShipScreen:   result = "ShipScreen";   break;
     case PlanetScreen: result = "PlanetScreen"; break;
     case BaseScreen:   result = "BaseScreen";   break;
     case Starchart:    result = "Starchart";    break;
    }
    return result;
}
