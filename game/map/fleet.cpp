/**
  *  \file game/map/fleet.cpp
  *  \brief Class game::map::Fleet
  */

#include "game/map/fleet.hpp"
#include "afl/string/format.hpp"
#include "game/map/configuration.hpp"
#include "game/map/ship.hpp"
#include "game/map/shiputils.hpp"
#include "game/map/universe.hpp"
#include "game/playerset.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/mission.hpp"
#include "game/spec/missionlist.hpp"

using afl::string::Format;
using game::spec::Mission;

namespace {
    /** Manipulating: check whether fleet member is being towed. */
    bool isFleetMemberTowed(game::map::Universe& univ, const game::map::Ship& ship)
    {
        const game::Id_t fid = ship.getFleetNumber();
        if (fid != 0) {
            game::map::AnyShipType& ships(univ.allShips());
            for (game::Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
                if (const game::map::Ship* other = ships.getObjectByIndex(i)) {
                    if (i != ship.getId() && other->getFleetNumber() == fid) {
                        if (other->getMission().isSame(Mission::msn_Tow)
                            && other->getMissionParameter(game::TowParameter).isSame(ship.getId()))
                        {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    int getShipMaxEfficientWarp(const game::map::Ship& sh, const game::spec::ShipList& shipList)
    {
        if (const game::spec::Engine* e = shipList.engines().get(sh.getEngineType().orElse(0))) {
            return e->getMaxEfficientWarp();
        } else {
            return game::spec::Engine::MAX_WARP;
        }
    }
}

game::map::Fleet::Fleet(Universe& univ, Ship& ship)
    : m_universe(univ),
      m_ship(ship)
{ }

void
game::map::Fleet::markDirty()
{
    // ex game/fleet.h:markFleetDirty
    AnyShipType& ships(m_universe.allShips());
    const int fleetNumber = m_ship.getId();
    for (Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
        if (Ship* sh = ships.getObjectByIndex(i)) {
            if (sh->getFleetNumber() == fleetNumber) {
                sh->markDirty();
            }
        }
    }
}

void
game::map::Fleet::synchronize(const game::config::HostConfiguration& config,
                              const game::spec::ShipList& shipList,
                              const Configuration& mapConfig)
{
    // ex game/fleet.h:synchronizeFleet, fleet.pas:SynchFleet
    AnyShipType& ships(m_universe.allShips());
    const int fleetNumber = m_ship.getId();
    for (Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
        if (Ship* sh = ships.getObjectByIndex(i)) {
            if (sh->getFleetNumber() == fleetNumber) {
                synchronizeFleetMember(m_universe, i, mapConfig, config, shipList);
            }
        }
    }
}

bool
game::map::Fleet::hasSpecialFunction(int basicFunction,
                                     const UnitScoreDefinitionList& scoreDefinitions,
                                     const game::spec::ShipList& shipList,
                                     const game::config::HostConfiguration& config) const
{
    // ex game/fleet.h:canFleetDoSpecial
    if (m_ship.getFleetNumber() == 0) {
        // Lone ship: just check it
        return m_ship.hasSpecialFunction(basicFunction, scoreDefinitions, shipList, config);
    } else {
        // Fleet: check all members
        AnyShipType& ships(m_universe.allShips());
        const int fleetNumber = m_ship.getId();
        for (Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
            if (Ship* sh = ships.getObjectByIndex(i)) {
                if (sh->getFleetNumber() == fleetNumber) {
                    if (!sh->hasSpecialFunction(basicFunction, scoreDefinitions, shipList, config)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
}

String_t
game::map::Fleet::getTitle(afl::string::Translator& tx) const
{
    return getTitle(m_ship, tx);
}

int
game::map::Fleet::getMaxEfficientWarp(const game::spec::ShipList& shipList) const
{
    // ex getFleetEngineSpeed(GUniverse& univ, int fid)
    if (m_ship.getFleetNumber() == 0) {
        // Lone ship: just check it
        return getShipMaxEfficientWarp(m_ship, shipList);
    } else {
        // Fleet: check all members
        int result = game::spec::Engine::MAX_WARP;
        AnyShipType& ships(m_universe.allShips());
        const int fleetNumber = m_ship.getId();
        for (Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
            if (const Ship* sh = ships.getObjectByIndex(i)) {
                if (sh->getFleetNumber() == fleetNumber) {
                    result = std::min(result, getShipMaxEfficientWarp(*sh, shipList));
                }
            }
        }
        return result;
    }
}

int
game::map::Fleet::countFleetMembers() const
{
    AnyShipType& ships(m_universe.allShips());
    const int fleetNumber = m_ship.getId();
    int result = 0;
    for (Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
        if (const Ship* sh = ships.getObjectByIndex(i)) {
            if (sh->getFleetNumber() == fleetNumber) {
                ++result;
            }
        }
    }
    return result;
}

// FIXME: I'm not entirely happy with the placement of this function.
// It would make sense to put this into FleetMember, but then we'd have a call chain FleetMember -> Fleet -> FleetMember.
void
game::map::Fleet::synchronizeFleetMember(Universe& univ, Id_t sid,
                                         const Configuration& mapConfig,
                                         const game::config::HostConfiguration& config,
                                         const game::spec::ShipList& shipList)
{
    // ex game/fleet.h:synchronizeFleetMember, fleet.pas:SynchWaypoint
    const game::spec::MissionList& missions = shipList.missions();

    // Fetch ship
    Ship* sh = univ.ships().get(sid);
    if (sh == 0) {
        // Ship does not exist? Error, ignore.
    } else if (sh->getFleetNumber() == 0) {
        // Ship is not member of a fleet, ignore.
    } else if (sh->getFleetNumber() == sid) {
        // I am the leader, so nothing to do.
    } else {
        // I am a member. Fetch the leader.
        if (const Ship* leader = univ.ships().get(sh->getFleetNumber())) {
            // Check for intercept mission
            const Mission* msn = getShipMission(*leader, config, missions);
            const bool xcept = (msn != 0 && msn->getFlags().contains(Mission::WaypointMission));

            if (xcept) {
                // We're intercepting
                if (leader->getMissionParameter(game::InterceptParameter).isSame(sid)) {
                    // Fleet is intercepting this very ship.
                    sh->setMission(Mission::msn_Explore, 0, 0);
                    sh->clearWaypoint();
                    sh->setWarpFactor(getShipMaxEfficientWarp(*sh, shipList));
                } else {
                    // Fleet is intercepting someone else.
                    sh->setMission(leader->getMission(),
                                   leader->getMissionParameter(InterceptParameter),
                                   leader->getMissionParameter(TowParameter));
                    setInterceptWaypoint(univ, *sh, mapConfig);
                    sh->setWarpFactor(leader->getWarpFactor());
                }
            } else {
                // We're moving normally
                if (isFleetMemberTowed(univ, *sh)) {
                    // Member is under tow, so avoid that it escapes.
                    sh->setWarpFactor(0);
                    sh->clearWaypoint();
                } else {
                    // Normal move
                    sh->setWarpFactor(leader->getWarpFactor());

                    Point shipPos, leaderPos;
                    if (sh->getPosition().get(shipPos) && leader->getWaypoint().get(leaderPos)) {
                        sh->setWaypoint(mapConfig.getSimpleNearestAlias(leaderPos, shipPos));
                    }
                }

                // Cancel any intercept
                const Mission* shipMission = getShipMission(*sh, config, missions);
                if (shipMission != 0 && shipMission->getFlags().contains(Mission::WaypointMission)) {
                    sh->setMission(Mission::msn_Explore, 0, 0);
                }
            }
        } else {
            // Leader does not exist? Error, ignore.
        }
    }
}

String_t
game::map::Fleet::getTitle(const Ship& ship, afl::string::Translator& tx)
{
    // ex getFleetTitle
    if (ship.getFleetName().empty()) {
        return Format(tx.translateString("Fleet %d: led by %s").c_str(), ship.getId(), ship.getName());
    } else {
        return Format(tx.translateString("Fleet %d: %s").c_str(), ship.getId(), ship.getFleetName());
    }
}
