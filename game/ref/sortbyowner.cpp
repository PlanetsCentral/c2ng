/**
  *  \file game/ref/sortbyowner.cpp
  */

#include "game/ref/sortbyowner.hpp"

game::ref::SortByOwner::SortByOwner(const game::map::Universe& univ, const PlayerList& players)
    : m_universe(univ),
      m_players(players)
{ }

int
game::ref::SortByOwner::compare(const Reference& a, const Reference& b) const
{
    // ex sortByOwner
    return getOwner(a) - getOwner(b);
}

String_t
game::ref::SortByOwner::getClass(const Reference& a) const
{
    // ex diviOwner
    return m_players.getPlayerName(getOwner(a), Player::ShortName);
}

int
game::ref::SortByOwner::getOwner(const Reference& a) const
{
    int result;
    if (a.getType() == Reference::Player) {
        result = a.getId();
    } else if (const game::map::Object* obj = m_universe.getObject(a)) {
        if (!obj->getOwner(result)) {
            result = 0;
        }
    } else {
        result = 0;
    }
    return result;
}
