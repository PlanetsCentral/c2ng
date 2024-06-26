/**
  *  \file game/sim/transfer.cpp
  *  \brief Class game::sim::Transfer
  */

#include "game/sim/transfer.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/planetformula.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/mission.hpp"
#include "game/vcr/objectinfo.hpp"

using game::Element;
using game::spec::BasicHullFunction;
using game::spec::Mission;

const char*const NULL_FCODE = "?""?""?";

game::sim::BaseTransfer::BaseTransfer(const game::spec::ShipList& shipList,
                                      const game::config::HostConfiguration& config,
                                      afl::string::Translator& tx)
    : m_shipList(shipList),
      m_config(config),
      m_translator(tx)
{ }

void
game::sim::BaseTransfer::setName(Ship& out, String_t name) const
{
    if (!name.empty()) {
        out.setName(name);
    }
    if (out.getName().empty()) {
        out.setDefaultName(m_translator);
    }
}

game::sim::Transfer::Transfer(const UnitScoreDefinitionList& shipScores,
                              const UnitScoreDefinitionList& planetScores,
                              const game::spec::ShipList& shipList,
                              const game::config::HostConfiguration& config,
                              afl::string::Translator& tx)
    : BaseTransfer(shipList, config, tx),
      m_shipScores(shipScores),
      m_planetScores(planetScores)
{ }

bool
game::sim::Transfer::copyShipFromGame(Ship& out, const game::map::Ship& in) const
{
    // ex GSimulatorRealGameInterface::copyShip (totally reworked)
    // Must know at least owner and hull
    int owner, hullNr;
    if (!in.getOwner().get(owner) || !in.getHull().get(hullNr)) {
        return false;
    }
    const game::spec::Hull* pHull = m_shipList.hulls().get(hullNr);
    if (pHull == 0) {
        return false;
    }

    // Id
    out.setId(in.getId());

    // Name
    setName(out, in.getName());

    // FCode
    out.setFriendlyCode(in.getFriendlyCode().orElse(NULL_FCODE));

    // Damage
    out.setDamage(std::max(0, in.getDamage().orElse(0) - in.getCargo(Element::Supplies).orElse(0) / 5));

    // Shield
    out.setShield(std::max(0, 100 - out.getDamage()));

    // Owner
    out.setOwner(in.getRealOwner().orElse(owner));

    // Experience
    out.setExperienceLevel(in.unitScores().getScoreById(ScoreId_ExpLevel, m_shipScores).orElse(0));

    // Crew
    out.setCrew(in.getCrew().orElse(pHull->getMaxCrew()));

    // Hull Type
    out.setHullTypeOnly(hullNr);

    // Mass
    out.setMass(pHull->getMass());

    // Beams
    {
        int numBeams = in.getNumBeams().orElse(pHull->getMaxBeams());
        int beamType = in.getBeamType().orElse(m_shipList.beams().size());
        if (numBeams > 0 && beamType > 0) {
            out.setNumBeams(numBeams);
            out.setBeamType(beamType);
        } else {
            out.setNumBeams(0);
            out.setBeamType(0);
        }
    }

    // Launchers
    {
        int numLaunchers = in.getNumLaunchers().orElse(pHull->getMaxLaunchers());
        int torpedoType = in.getTorpedoType().orElse(m_shipList.launchers().size());
        if (numLaunchers > 0 && torpedoType > 0) {
            out.setNumLaunchers(numLaunchers);
            out.setTorpedoType(torpedoType);
        } else {
            out.setNumLaunchers(0);
            out.setTorpedoType(0);
        }
    }

    // Bays
    out.setNumBays(in.getNumBays().orElse(pHull->getNumBays()));

    // Ammo
    if (out.getNumLaunchers() != 0 || out.getNumBays() != 0) {
        out.setAmmo(in.getAmmo().orElse(pHull->getMaxCargo()));
    } else {
        out.setAmmo(0);
    }

    // Engine
    out.setEngineType(in.getEngineType().orElse(m_shipList.engines().size()));

    // Aggressiveness
    const int fuel = in.getCargo(Element::Neutronium).orElse(-1);
    const int mission = in.getMission().orElse(Mission::msn_Kill);
    const int pe = in.getPrimaryEnemy().orElse(0);
    if (fuel == 0) {
        out.setAggressiveness(Ship::agg_NoFuel);
    } else if (mission == Mission::msn_Kill) {
        out.setAggressiveness(Ship::agg_Kill);
    } else {
        out.setAggressiveness(pe);
    }

    // Intercept
    const bool cloakable = in.hasSpecialFunction(BasicHullFunction::Cloak, m_shipScores, m_shipList, m_config);
    if (mission == Mission::msn_Intercept && cloakable) {
        out.setInterceptId(in.getMissionParameter(InterceptParameter).orElse(0));
    } else {
        out.setInterceptId(0);
    }

    // Flags
    int32_t flags = 0;
    if (fuel > 0 && cloakable && m_shipList.missions().isMissionCloaking(mission, out.getOwner(), m_config)) {
        flags |= Ship::fl_Cloaked;
    }
    setHullFunction(flags, out, in, FullWeaponryAbility,   BasicHullFunction::FullWeaponry);
    setHullFunction(flags, out, in, PlanetImmunityAbility, BasicHullFunction::PlanetImmunity);
    setHullFunction(flags, out, in, CommanderAbility,      BasicHullFunction::Commander);
    out.setFlags(flags);
    return true;
}

bool
game::sim::Transfer::copyShipToGame(game::map::Ship& out, const Ship& in, game::map::Universe& univ, const game::map::Configuration& mapConfig) const
{
    // ex GSimulatorRealGameInterface::updateToGame, ccsim.pas:DoWriteback
    int owner, hullNr;
    if (!out.getOwner().get(owner) || !out.getHull().get(hullNr)) {
        return false;
    }
    const int realOwner = out.getRealOwner().orElse(owner);
    if (!out.isPlayable(game::map::Object::Playable) || realOwner != in.getOwner() || hullNr != in.getHullType()) {
        return false;
    }

    // Mission
    // Do not touch fleet leaders/members here for now, but use FleetMember for implicit intercept waypoint propagation
    if (out.getFleetNumber() == 0) {
        game::map::FleetMember mem(univ, out, mapConfig);
        if (in.getAggressiveness() == Ship::agg_Kill) {
            // Aggressiveness Kill -> set Kill mission
            mem.setMission(Mission::msn_Kill, 0, 0, m_config, m_shipList);
        } else {
            const int oldMission = out.getMission().orElse(0);
            const bool isCloaking = m_shipList.missions().isMissionCloaking(oldMission, realOwner, m_config);
            if (out.hasSpecialFunction(BasicHullFunction::Cloak, m_shipScores, m_shipList, m_config) && (in.getFlags() & Ship::fl_Cloaked) != 0) {
                // Ship can cloak -> set a cloak mission unless it already has one
                if (!isCloaking) {
                    mem.setMission(Mission::msn_Cloak, 0, 0, m_config, m_shipList);
                }
            } else {
                // Ship shall not cloak -> reset Cloak mission if any
                // Ship shall not kill -> reset Kill mission if any
                if (isCloaking || oldMission == Mission::msn_Kill) {
                    mem.setMission(0, 0, 0, m_config, m_shipList);
                }

                const int intId = in.getInterceptId();
                if (intId != 0) {
                    // Take over Intercept mission if allowed
                    const game::map::Ship* intShip = univ.ships().get(intId);
                    if (intShip != 0 && intShip->isReliablyVisible(0)) {
                        mem.setMission(Mission::msn_Intercept, intId, 0, m_config, m_shipList);
                    }
                }
            }
        }
    }

    // PE
    if (Ship::isPrimaryEnemy(in.getAggressiveness())) {
        out.setPrimaryEnemy(in.getAggressiveness());
    } else {
        out.setPrimaryEnemy(0);
    }

    // FCode, Name
    out.setFriendlyCode(in.getFriendlyCode());
    out.setName(in.getName());

    // Ammo
    // FIXME: in case we handle mkt/lfm someday, we would have to revert it here
    int simAmmo = in.getAmmo();
    int shipAmmo = out.getAmmo().orElse(simAmmo);
    game::map::Point pt;
    game::map::Planet* planet = 0;
    if (simAmmo != shipAmmo
        && out.getPosition().get(pt)
        && (planet = univ.planets().get(univ.findPlanetAt(pt))) != 0
        && planet->isPlayable(game::map::Object::Playable))
    {
        // Preconditions for client-side transfer are fulfilled, set up transfer normally.
        // This checks additional preconditions.
        try {
            game::actions::CargoTransfer tr;
            game::actions::CargoTransferSetup::fromPlanetShip(univ, planet->getId(), out.getId())
                .buildDirect(tr, univ, m_config, m_shipList);

            if (out.getNumBays().orElse(0) != 0) {
                tr.move(Element::Fighters, simAmmo - shipAmmo, 0, 1, true, false);
            } else if (out.getNumLaunchers().orElse(0) != 0) {
                tr.move(Element::fromTorpedoType(out.getTorpedoType().orElse(1)), simAmmo - shipAmmo, 0, 1, true, false);
            } else {
                // should not happen
            }
            tr.commit();
        }
        catch (...) {
            // Ignore possible errors, e.g. planet doesn't have a starbase or we failed at the precondition check
        }
    }

    return true;
}

bool
game::sim::Transfer::copyPlanetFromGame(Planet& out, const game::map::Planet& in) const
{
    // ex GSimulatorRealGameInterface::copyPlanet
    // We cannot do anything sensible if we don't know the owner
    int owner = 0;
    if (!in.getOwner().get(owner) || owner == 0) {
        return false;
    }

    // Id, Name
    out.setId(in.getId());
    out.setName(in.getName(m_translator));

    // Friendly Code
    out.setFriendlyCode(in.getFriendlyCode().orElse(NULL_FCODE));

    // Damage/Shield
    // FIXME: can we do better?
    out.setDamage(0);
    out.setShield(100);

    // Owner
    out.setOwner(owner);

    // Experience
    out.setExperienceLevel(in.unitScores().getScoreById(ScoreId_ExpLevel, m_planetScores).orElse(0));

    // Flags: there are no flags relevant for planets so far
    out.setFlags(0);

    // Defense
    {
        int defense;
        int32_t maxDefense;
        if (in.getNumBuildings(DefenseBuilding).get(defense)) {
            out.setDefense(defense);
        } else if (getMaxBuildings(in, DefenseBuilding, m_config).get(maxDefense)) {
            out.setDefense(maxDefense);
        } else {
            out.setDefense(10);
        }
    }

    // Starbase
    int beamTech;
    if (in.hasBase() && in.getBaseTechLevel(BeamTech).get(beamTech) && beamTech != 0) {
        // Base present
        out.setBaseBeamTech(beamTech);
        out.setBaseDefense(in.getNumBuildings(BaseDefenseBuilding).orElse(0));
        out.setBaseTorpedoTech(in.getBaseTechLevel(TorpedoTech).orElse(1));
        out.setNumBaseFighters(in.getCargo(Element::Fighters).orElse(0));
        for (int i = 1; i <= Planet::NUM_TORPEDO_TYPES; ++i) {
            out.setNumBaseTorpedoes(i, in.getCargo(Element::fromTorpedoType(i)).orElse(0));
        }
        out.setBaseDamage(in.getBaseDamage().orElse(0));
    } else {
        // No base
        out.setBaseBeamTech(0);
        out.setBaseDefense(0);
        out.setBaseTorpedoTech(0);
        out.setNumBaseFighters(0);
        for (int i = 1; i <= Planet::NUM_TORPEDO_TYPES; ++i) {
            out.setNumBaseTorpedoes(i, 0);
        }
        out.setBaseDamage(0);
    }

    return true;
}

bool
game::sim::Transfer::copyPlanetToGame(game::map::Planet& out, const Planet& in) const
{
    // Check applicability
    int owner;
    if (!out.getOwner().get(owner)) {
        return false;
    }
    if (!out.isPlayable(game::map::Object::Playable) || owner != in.getOwner()) {
        return false;
    }

    // Copy
    // The only thing we can safely copy is the friendly code for now
    out.setFriendlyCode(in.getFriendlyCode());
    return true;
}

bool
game::sim::BaseTransfer::copyShipFromBattle(Ship& out, const game::vcr::Object& in, int assumedHull, bool withESB) const
{
    // ex client/widgets/vcrinfomain.cc:addShipToSim (part)
    out.setId(in.getId());
    setName(out, in.getName());
    out.setFriendlyCode(NULL_FCODE);
    out.setDamage(in.getDamage());
    out.setShield(in.getShield());
    out.setOwner(in.getOwner());
    out.setExperienceLevel(in.getExperienceLevel());
    out.setFlags(0);
    out.setFlakRatingOverride(0);
    out.setFlakCompensationOverride(0);
    out.setHullType(assumedHull, m_shipList);    // Must be before crew, mass, and weapons; will set those to defaults
    out.setCrew(in.getCrew());
    out.setMass(in.getMass());
    out.setBeamType(in.getBeamType());
    out.setNumBeams(in.getNumBeams());
    out.setTorpedoType(in.getTorpedoType());
    out.setNumLaunchers(in.getNumLaunchers());
    out.setNumBays(in.getNumBays());
    out.setAmmo(in.getNumLaunchers() > 0
                ? in.getNumTorpedoes()
                : in.getNumFighters());

    int engineType = in.getGuessedEngine(m_shipList.engines(), m_shipList.hulls().get(assumedHull), withESB, m_config);
    if (engineType == 0) {
        engineType = 1;
    }
    out.setEngineType(engineType);
    out.setAggressiveness(Ship::agg_Kill);
    out.setInterceptId(0);
    return true;
}

bool
game::sim::BaseTransfer::copyPlanetFromBattle(Planet& out, const game::vcr::Object& in) const
{
    // ex client/widgets/vcrinfomain.cc:addPlanetToSim
    game::vcr::PlanetInfo pi;
    describePlanet(pi, in, m_config);
    if (!pi.isValid) {
        return false;
    }

    out.setId(in.getId());
    out.setName(in.getName());
    out.setFriendlyCode("ATT");
    out.setDamage(in.getDamage());
    out.setShield(in.getShield());
    out.setOwner(in.getOwner());
    out.setExperienceLevel(in.getExperienceLevel());
    out.setFlags(0);
    out.setFlakRatingOverride(0);
    out.setFlakCompensationOverride(0);

    // Clear all base torps
    for (int i = 1; i <= Planet::NUM_TORPEDO_TYPES; ++i) {
        out.setNumBaseTorpedoes(i, 0);
    }

    // We use the maximum possible planet defense, and the minimum
    // possible starbase defense. The rest follows from that.
    out.setDefense(pi.defense.empty() ? 0 : pi.defense.max());
    if (pi.hasBase) {
        // Defense, beams
        out.setBaseDefense(pi.baseDefense.empty() ? 0 : pi.baseDefense.min());
        out.setBaseBeamTech(pi.baseBeamTech.empty() ? 1 : pi.baseBeamTech.min());

        // FIXME: starbase torps!
        out.setBaseTorpedoTech(1);

        // Fighters
        out.setNumBaseFighters(pi.numBaseFighters.min());
    } else {
        out.setBaseDefense(0);
        out.setBaseBeamTech(0);
        out.setBaseTorpedoTech(1);
        out.setNumBaseFighters(0);
    }

    return true;
}

void
game::sim::Transfer::setHullFunction(int32_t& flags, const Ship& out, const game::map::Ship& in, Ability a, int basicHullFunction) const
{
    // ex game/sim-game.cc:setHullFunction
    // FIXME: we pass a blank Configuration() to hasImpliedFunction. For now, this configuration does not affect anything.
    // If it starts affecting things, we should pass one that matches version/config, because we probably want to sim
    // "this game's host" when we add "this game's ship".
    const bool shipCanDo = in.hasSpecialFunction(basicHullFunction, m_shipScores, m_shipList, m_config);
    const bool simCanDo = out.hasImpliedAbility(a, Configuration(), m_shipList, m_config);
    if (shipCanDo != simCanDo) {
        const Object::AbilityInfo info = Object::getAbilityInfo(a);
        flags |= info.setBit;
        if (shipCanDo) {
            flags |= info.activeBit;
        }
    }
}

