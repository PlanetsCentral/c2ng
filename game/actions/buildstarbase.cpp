/**
  *  \file game/actions/buildstarbase.cpp
  *  \brief Class game::actions::BuildStarbase
  */

#include "game/actions/buildstarbase.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"

// Constructor.
game::actions::BuildStarbase::BuildStarbase(game::map::Planet& planet,
                                            CargoContainer& container,
                                            bool wantBase,
                                            afl::string::Translator& tx,
                                            game::config::HostConfiguration& config)
    : m_planet(planet),
      m_wantBase(wantBase),
      m_hostConfiguration(config),
      m_costAction(container),
      m_planetChangeConnection(planet.sig_change.add(this, &BuildStarbase::update)),
      m_actionChangeConnection(m_costAction.sig_change.add(this, &BuildStarbase::update)),
      m_configChangeConnection(config.sig_change.add(this, &BuildStarbase::update))
{
    // ex GPlanetBuildStarbaseAction::GPlanetBuildStarbaseAction
    // ex planint.pas:BuildBase (sort-of)
    mustBePlayed(planet, tx);

    if (wantBase == m_planet.isBuildingBase()) {
        // Null operation. It is an error to start with a null action,
        // but it's ok if a later change makes this action pointless.
        if (wantBase) {
            throw Exception(Exception::eDone, tx.translateString("Starbase building already in progress"));
        } else {
            throw Exception(Exception::eDone, tx.translateString("No starbase being built"));
        }
    } else if (m_planet.hasBase()) {
        // base present
        throw Exception(Exception::ePerm, tx.translateString("Starbase already present"));
    } else {
        // no base; build action is ok.
    }

    update();
}

// Destructor.
game::actions::BuildStarbase::~BuildStarbase()
{ }

void
game::actions::BuildStarbase::commit()
{
    update();
    if (!m_costAction.isValid()) {
        throw Exception(Exception::eNoResource);
    }
    m_planet.setBuildBaseFlag(m_wantBase);
    m_costAction.commit();
}

bool
game::actions::BuildStarbase::isValid() const
{
    return m_costAction.isValid();
}

const game::actions::CargoCostAction&
game::actions::BuildStarbase::costAction() const
{
    return m_costAction;
}

void
game::actions::BuildStarbase::update()
{
    int factor = int(m_wantBase) - int(m_planet.isBuildingBase());
    int owner;
    if (m_planet.getOwner(owner)) {
        m_costAction.setCost(m_hostConfiguration[game::config::HostConfiguration::StarbaseCost](owner) * factor);
    }
}
