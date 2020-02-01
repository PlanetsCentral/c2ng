/**
  *  \file game/actions/changebuildqueue.cpp
  *  \brief Class game::actions::ChangeBuildQueue
  *
  *  Build rules:
  *
  *  Host:
  *    - PBx friendly codes are not case-sensitive
  *    - identical PBx friendly codes ok, will build both
  *    - can priority-build more than 10 ships
  *
  *  PHost PAL:
  *    - PBx friendly codes are case-sensitive
  *    - only one PBx per slot, reorders builds
  *    - clones in build queue
  *
  *  PHost PBP:
  *    - PBx friendly codes are case-sensitive
  *    - only one PBx per slot
  *    - clones in build queue
  */

#include "game/actions/changebuildqueue.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/hostversion.hpp"
#include "game/map/reverter.hpp"

using afl::string::Format;
using game::map::Planet;
using game::map::Ship;
using game::config::HostConfiguration;

namespace {
    const int MAX_PRIORITY_VALUE = 9;

    /* Check priority friendly code. */
    int checkPriorityCode(const String_t& str, const game::HostVersion& host)
    {
        if (str.size() == 3
            && (str[0] == 'P' || (!host.isPHost() && str[0] == 'p'))
            && (str[1] == 'B' || (!host.isPHost() && str[1] == 'b'))
            && (str[2] >= '1' && str[2] <= '9'))
        {
            return (str[2] - '0');
        } else {
            return 0;
        }
    }

    String_t makePriorityCode(int pri)
    {
        return String_t("PB") + char('0' + pri);
    }


    game::Id_t findCloningShipAt(game::map::Universe& univ, game::map::Point pt)
    {
        game::map::PlayedShipType& ships = univ.playedShips();
        for (game::Id_t i = ships.findFirstObjectAt(pt); i != 0; i = ships.findNextObjectAt(pt, i)) {
            String_t shipFC;
            const Ship* pShip = ships.getObjectByIndex(i);
            if (pShip != 0 && pShip->getFriendlyCode().get(shipFC) && shipFC == "cln") {
                return i;
            }
        }
        return 0;
    }

    game::Id_t findPreviouslyCloningShipAt(game::map::Universe& univ, game::map::Point pt)
    {
        game::map::PlayedShipType& ships = univ.playedShips();
        for (game::Id_t i = ships.findFirstObjectAt(pt); i != 0; i = ships.findNextObjectAt(pt, i)) {
            String_t shipFC;
            game::map::Reverter* rev = univ.getReverter();
            if (rev != 0 && rev->getPreviousShipFriendlyCode(i).get(shipFC) && shipFC == "cln") {
                return i;
            }
        }
        return 0;
    }

    bool isEqual(game::ShipBuildOrder a, game::ShipBuildOrder b)
    {
        a.canonicalize();
        b.canonicalize();
        return a == b;
    }
}

/*
 *  Sort Predicate
 *
 *  This tries to sort the build orders into the order in which they will be executed.
 */
class game::actions::ChangeBuildQueue::Sorter {
 public:
    Sorter(const ChangeBuildQueue& parent)
        : m_parent(parent)
        { }
    bool operator()(const LocalInfo& a, const LocalInfo& b) const
        {
            // Parse friendly codes
            int valA = checkPriorityCode(a.friendlyCode, m_parent.m_host);
            int valB = checkPriorityCode(b.friendlyCode, m_parent.m_host);

            // Prioritized goes before unprioritized
            bool priA = (valA != 0);
            bool priB = (valB != 0);
            if (priA != priB) {
                return priA > priB;
            }

            if (priA) {
                // Two priorized orders: lower values go first
                if (valA != valB) {
                    return valA < valB;
                }
            } else {
                // Unpriorized: use queue order if known
                int knownA = a.queuePosition != 0;
                int knownB = b.queuePosition != 0;
                if (knownA != knownB) {
                    return knownA > knownB;
                }
                if (a.queuePosition != b.queuePosition) {
                    return a.queuePosition < b.queuePosition;
                }
            }

            // Use Id as tie-breaker
            return a.planetId < b.planetId;
        }
 private:
    const ChangeBuildQueue& m_parent;
};

// Constructor.
game::actions::ChangeBuildQueue::ChangeBuildQueue(game::map::Universe& univ,
                                                  const game::spec::ShipList& shipList,
                                                  game::HostVersion host,
                                                  const game::config::HostConfiguration& config,
                                                  util::RandomNumberGenerator& rng,
                                                  int viewpointPlayer)
    : m_universe(univ),
      m_shipList(shipList),
      m_host(host),
      m_config(config),
      m_availablePoints()
{
    init(rng, viewpointPlayer);
}

// Destructor.
game::actions::ChangeBuildQueue::~ChangeBuildQueue()
{ }

// Set available build points.
void
game::actions::ChangeBuildQueue::setAvailableBuildPoints(LongProperty_t points)
{
    m_availablePoints = points;
}

// Prepare data into output format.
void
game::actions::ChangeBuildQueue::describe(Infos_t& result, afl::string::Translator& tx) const
{
    int32_t points = 0;
    bool pointsOK = m_availablePoints.get(points);
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        // Input
        const LocalInfo& in = m_info[i];
        const Planet& pl = mustExist(m_universe.planets().get(in.planetId));
        int player = 0;
        pl.getOwner(player);

        // Make empty slot
        result.push_back(Info());
        Info& out = result.back();

        // Simple content
        out.planetId = in.planetId;
        out.planetName = pl.getName(tx);
        out.friendlyCode = in.friendlyCode;
        out.queuePosition = in.queuePosition;
        out.conflict = (i > 0 && m_info[i-1].friendlyCode == out.friendlyCode && checkPriorityCode(out.friendlyCode, m_host) != 0);
        out.hasPriority = checkPriorityCode(in.friendlyCode, m_host);
        out.playable = in.playable;

        // Ship being built
        int hullMass = 0;
        bool isClone = false;
        if (in.cloningShipId == 0) {
            if (const game::spec::Hull* pHull = m_shipList.hulls().get(pl.getBaseBuildHull(m_config, m_shipList.hullAssignments()).orElse(0))) {
                out.actionName = Format(tx("Build %s"), pHull->getName(m_shipList.componentNamer()));
                hullMass = pHull->getMass();
            }
        } else {
            const Ship& sh = mustExist(m_universe.ships().get(in.cloningShipId));
            String_t shipName = sh.getName();
            if (shipName.empty()) {
                shipName = Format(tx("Ship #%d"), sh.getId());
            }
            out.actionName = Format(tx("Clone %s"), shipName);

            if (const game::spec::Hull* pHull = m_shipList.hulls().get(sh.getHull().orElse(0))) {
                hullMass = pHull->getMass();
            }
            isClone = true;
        }

        // Points
        if (m_host.isPBPGame(m_config) && hullMass != 0) {
            // FIXME: formula doubled in HullSpecificationProxy
            int32_t pointsRequired;
            if (m_host.isPHost()) {
                pointsRequired = std::max(hullMass * m_config[HostConfiguration::PBPCostPer100KT](player) / 100,
                                          m_config[HostConfiguration::PBPMinimumCost](player));
                if (isClone) {
                    pointsRequired = pointsRequired * m_config[HostConfiguration::PBPCloneCostRate](player) / 100;
                }
            } else {
                pointsRequired = (hullMass + 49) / 50;
            }
            out.pointsRequired = pointsRequired;

            if (pointsOK) {
                out.pointsAvailable = points;
                points = std::max(int32_t(0), points - pointsRequired);
            }
        }
    }
}

// Set priority of a build order.
void
game::actions::ChangeBuildQueue::setPriority(size_t slot, int pri)
{
    if (slot < m_info.size()) {
        LocalInfo& info = m_info[slot];
        if (info.playable) {
            if (pri >= 1 && pri <= MAX_PRIORITY_VALUE) {
                info.friendlyCode = makePriorityCode(pri);
            } else {
                info.friendlyCode = info.oldFriendlyCode;
            }
            sort();
        }
    }
}

// Increase a slot's priority (build earlier).
void
game::actions::ChangeBuildQueue::increasePriority(size_t slot)
{
    const size_t n = m_info.size();
    if (slot < n) {
        LocalInfo& info = m_info[slot];
        if (info.playable) {
            const int current = checkPriorityCode(info.friendlyCode, m_host);
            if (current == 0) {
                // Unpriorized to lowest priority: set to one-plus-highest used value
                // (need to check items before slot only)
                // If no priority codes are currently used, this conveniently sets the value to PB1.
                int maxValue = 0;
                for (size_t i = 0; i < slot; ++i) {
                    maxValue = std::max(maxValue, checkPriorityCode(m_info[i].friendlyCode, m_host));
                }
                info.friendlyCode = makePriorityCode(std::min(MAX_PRIORITY_VALUE, 1 + maxValue));
            } else if (current > 1) {
                // Higher priority (lower value).
                // Set priority value. If this causes a duplicate, try to move these out of the way.
                info.friendlyCode = makePriorityCode(current-1);
                avoid(current-1, current, slot);
            } else {
                // Cannot go higher
            }
            sort();
        }
    }
}

// Decrease a slot's priority (build later).
void
game::actions::ChangeBuildQueue::decreasePriority(size_t slot)
{
    const size_t n = m_info.size();
    if (slot < n) {
        LocalInfo& info = m_info[slot];
        if (info.playable) {
            const int current = checkPriorityCode(info.friendlyCode, m_host);
            if (current == 0) {
                // Cannot go any lower
            } else if (current == 9 || (slot+1 < m_info.size() && checkPriorityCode(m_info[slot+1].friendlyCode, m_host) == 0)) {
                // Least priority to unprioritized
                info.friendlyCode = info.oldFriendlyCode;
            } else {
                // Lower priority (higher value)
                info.friendlyCode = makePriorityCode(current+1);
                avoid(current+1, current, slot);
            }
            sort();
        }
    }
}

// Write all changes back to universe.
void
game::actions::ChangeBuildQueue::commit()
{
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        if (Planet* pl = m_universe.planets().get(m_info[i].planetId)) {
            if (pl->isPlayable(game::map::Object::Playable)) {
                pl->setFriendlyCode(m_info[i].friendlyCode);
            }
        }
    }
}

void
game::actions::ChangeBuildQueue::init(util::RandomNumberGenerator& rng, int viewpointPlayer)
{
    game::map::PlayedBaseType& bases = m_universe.playedBases();
    for (Id_t i = bases.findNextIndex(0); i != 0; i = bases.findNextIndex(i)) {
        Planet* p = m_universe.planets().get(i);
        game::map::Point pt;
        int owner;
        if (p != 0 && p->getOwner(owner) && owner == viewpointPlayer && p->getPosition(pt)) {
            // Existing friendly code
            String_t friendlyCode = p->getFriendlyCode().orElse("");

            // Old (revert) friendly code
            String_t oldFriendlyCode = checkPriorityCode(friendlyCode, m_host) == 0
                ? friendlyCode
                : m_shipList.friendlyCodes().generateRandomCode(rng, m_host);

            // Find ship being cloned here. Only for PHost; cloning has entirely different queue in Host.
            Id_t cloningShipId = 0;
            Id_t previouslyCloningShipId = 0;
            if (m_host.isPHost() && m_config[HostConfiguration::AllowShipCloning]()) {
                cloningShipId = findCloningShipAt(m_universe, pt);
                previouslyCloningShipId = findPreviouslyCloningShipAt(m_universe, pt);
            }

            // Ship being built here
            int buildingHullId = p->getBaseBuildHull(m_config, m_shipList.hullAssignments()).orElse(0);
            if (cloningShipId != 0 || buildingHullId != 0) {
                // New build order
                bool isNew = false;
                if (m_host.isPHost()) {
                    // For PHost, check whether order was changed
                    if (cloningShipId != 0) {
                        // Cloning ship: ID must not have changed (build order not relevant)
                        if (previouslyCloningShipId != cloningShipId) {
                            isNew = true;
                        }
                    } else {
                        // Not cloning: build order must be the same, no clone
                        game::map::Reverter* rev = m_universe.getReverter();
                        ShipBuildOrder oldOrder;
                        if ((rev != 0 && rev->getPreviousShipBuildOrder(i, oldOrder) && !isEqual(p->getBaseBuildOrder(), oldOrder))
                            || previouslyCloningShipId != 0)
                        {
                            isNew = true;
                        }
                    }
                }

                // Queue position
                int queuePosition = isNew ? 0 : p->getBaseQueuePosition().orElse(0);

                // Playability
                bool playable = p->isPlayable(game::map::Object::Playable);

                // Remember it
                m_info.push_back(LocalInfo(i, cloningShipId, friendlyCode, oldFriendlyCode, queuePosition, playable));
            }
        }
    }
    sort();
}

void
game::actions::ChangeBuildQueue::sort()
{
    std::sort(m_info.begin(), m_info.end(), Sorter(*this));
}

void
game::actions::ChangeBuildQueue::avoid(int setThis, int toThis, size_t slot)
{
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        if (i != slot && m_info[i].playable && checkPriorityCode(m_info[i].friendlyCode, m_host) == setThis) {
            m_info[i].friendlyCode = makePriorityCode(toThis);
        }
    }
}
