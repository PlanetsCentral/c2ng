/**
  *  \file game/interface/missionfunction.cpp
  */

#include "game/interface/missionfunction.hpp"
#include "interpreter/arguments.hpp"
#include "game/root.hpp"

game::interface::MissionFunction::MissionFunction(Session& session)
    : m_session(session)
{ }

// IndexableValue:
game::interface::MissionContext*
game::interface::MissionFunction::get(interpreter::Arguments& args)
{
    /* @q Mission(number:Int, [player:Int]):Obj (Function, Context)
       Access ship mission properties.
       Use as
       | ForEach Mission Do ...
       or
       | With Mission(shipMission, shipOwner) Do ...

       @see int:index:group:missionproperty|Mission Properties
       @since PCC2NG 2.40.1 */

    args.checkArgumentCount(1, 2);

    // Mission number argument
    int32_t number;
    if (!interpreter::checkIntegerArg(number, args.getNext(), 0, 0x7FFF)) {
        return 0;
    }

    // Player argument
    int player = 0;
    bool hasPlayer = interpreter::checkIntegerArg(player, args.getNext(), 1, MAX_PLAYERS);

    // Verify environment
    Root* root = m_session.getRoot().get();
    game::spec::ShipList* shipList = m_session.getShipList().get();
    if (root == 0 || shipList == 0) {
        return 0;
    }

    // Create race mask. Note that missions operate on races, but we have a player number!
    PlayerSet_t playerSet;
    if (hasPlayer) {
        playerSet = PlayerSet_t(root->hostConfiguration().getPlayerMissionNumber(player));
    } else {
        playerSet = PlayerSet_t::allUpTo(MAX_RACES);
    }

    // Create result
    size_t slot;
    if (!shipList->missions().getIndexByNumber(number, playerSet, slot)) {
        return 0;
    }
    return new MissionContext(slot, *shipList);
}

void
game::interface::MissionFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw interpreter::Error::notAssignable();
}

// CallableValue:
int32_t
game::interface::MissionFunction::getDimension(int32_t /*which*/) const
{
    return 0;
}

game::interface::MissionContext*
game::interface::MissionFunction::makeFirstContext()
{
    game::spec::ShipList* shipList = m_session.getShipList().get();
    if (shipList != 0 && shipList->missions().at(0) != 0) {
        return new MissionContext(0, *shipList);
    } else {
        return 0;
    }
}

game::interface::MissionFunction*
game::interface::MissionFunction::clone() const
{
    return new MissionFunction(m_session);
}

// BaseValue:
String_t
game::interface::MissionFunction::toString(bool /*readable*/) const
{
    return "#<array:Mission>";
}

void
game::interface::MissionFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
