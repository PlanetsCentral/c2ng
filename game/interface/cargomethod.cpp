/**
  *  \file game/interface/cargomethod.cpp
  *  \brief Foundations of cargo-related script commands
  */

#include "game/interface/cargomethod.hpp"
#include "afl/data/stringvalue.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"

using game::Exception;

namespace {
    const int flagOverload = 1;
    const int flagSup2MC   = 2;
    const int flagNoFail   = 4;
    const int flagJettison = 8;


    void doScriptTransfer(game::actions::CargoTransferSetup& setup,
                          interpreter::Process& process,
                          int32_t flags,
                          const String_t& cargospec,
                          game::Session& session,
                          const game::map::Configuration& mapConfig,
                          game::Turn& turn,
                          const game::Root& root)
    {
        // ex int/if/shipif.cc:doScriptTransfer
        // Parse cargospec
        game::CargoSpec cs;
        if (!cs.parse(cargospec, false)) {
            throw Exception("Invalid cargo specification");
        }

        // Build cargo transfer from template
        const game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);
        game::actions::CargoTransfer action;
        setup.build(action, turn, mapConfig, root.hostConfiguration(), shipList, root.hostVersion());

        // Configure
        if ((flags & flagOverload) != 0) {
            action.setOverload(true);
        }

        // Execute transfer
        action.move(cs, shipList, 0, 1, (flags & flagSup2MC) != 0);

        // Evaluate result
        if ((flags & flagNoFail) != 0) {
            action.commit();
            afl::data::StringValue sv(cs.toCargoSpecString());
            process.setVariable("CARGO.REMAINDER", &sv);
        } else {
            if (!cs.isZero()) {
                throw Exception("Cargo transfer failed");
            } else {
                action.commit();
            }
        }
    }

    void setProxyIfRequired(game::actions::CargoTransferSetup& setup, const game::Turn& turn, int proxyId)
    {
        if (proxyId != 0) {
            // Error handling behaviour:
            // PCC 1.x validates the proxy to be correct position, owner, status, and produces ERANGE/EPOS/ENOTOWNER.
            // However, it ignores the valid proxy if it is not needed.
            // We only validate the proxy when needed.
            if (setup.getStatus() == game::actions::CargoTransferSetup::NeedProxy) {
                if (!setup.setProxy(turn.universe(), proxyId)) {
                    throw Exception(Exception::ePos);
                }
            }
        }
    }
}


void
game::interface::doCargoTransfer(game::map::Planet& pl, interpreter::Process& process, interpreter::Arguments& args, Session& session, const game::map::Configuration& mapConfig, Turn& turn, const Root& root)
{
    // ex int/if/planetif.cc:IFPlanetCargoTransfer
    // ex planint.pas:Planet_CargoTransfer
    // Args are cargospec, ship Id, OSNJ flags + proxy ID
    // @change PCC1 accepts 'J' flags, but that makes no sense for planets
    args.checkArgumentCount(2, 3);

    String_t cargospec;
    int32_t target_sid;
    int32_t flags = 0;
    int32_t proxyId = 0;
    if (!interpreter::checkStringArg(cargospec, args.getNext())) {
        return;
    }
    if (!interpreter::checkIntegerArg(target_sid, args.getNext(), 1, MAX_NUMBER)) {
        // This used to limit to turn.universe().ships().size(). We now limit to MAX_NUMBER instead
        // so the error message will be the same no matter whether the given Id is higher or lower
        // than the maximum existing ship Id (game::Exception). Only numbers that are clearly out
        // of range still generate a interpreter::Error.
        return;
    }
    interpreter::checkFlagArg(flags, &proxyId, args.getNext(), "OSN");

    // Validate
    game::actions::CargoTransferSetup setup = game::actions::CargoTransferSetup::fromPlanetShip(turn.universe(), pl.getId(), target_sid);
    setProxyIfRequired(setup, turn, proxyId);

    // Do it
    doScriptTransfer(setup, process, flags, cargospec, session, mapConfig, turn, root);
}

void
game::interface::doCargoTransfer(game::map::Ship& sh, interpreter::Process& process, interpreter::Arguments& args, Session& session, const game::map::Configuration& mapConfig, Turn& turn, const Root& root)
{
    // ex int/if/shipif.cc:IFShipCargoTransfer
    // ex shipint.pas:Ship_CargoTransfer
    // Args are cargospec, target ship Id, flags O/S/N (overload, supply sale, no-fail)
    String_t cargospec;
    int32_t target_sid;
    int32_t flags = 0;
    args.checkArgumentCount(2, 3);
    if (!interpreter::checkStringArg(cargospec, args.getNext())) {
        return;
    }
    if (!interpreter::checkIntegerArg(target_sid, args.getNext(), 1, MAX_NUMBER)) {
        return;
    }
    interpreter::checkFlagArg(flags, 0, args.getNext(), "OSN");

    // Validate
    game::actions::CargoTransferSetup setup = game::actions::CargoTransferSetup::fromShipShip(turn.universe(), sh.getId(), target_sid);

    // Do it
    doScriptTransfer(setup, process, flags, cargospec, session, mapConfig, turn, root);
}

void
game::interface::doCargoUnload(game::map::Ship& sh, bool reverse, interpreter::Process& process, interpreter::Arguments& args, Session& session, const game::map::Configuration& mapConfig, Turn& turn, const Root& root)
{
    // ex int/if/shipif.cc:doShipCargoUnUpload
    // Args are cargospec, flags O/S/N/J (overload, supply sale, no-fail, jettison permission) + proxy Id
    String_t cargospec;
    int32_t flags = 0;
    int32_t proxyId = 0;
    args.checkArgumentCount(1, 2);
    if (!interpreter::checkStringArg(cargospec, args.getNext())) {
        return;
    }
    interpreter::checkFlagArg(flags, &proxyId, args.getNext(), "OSNJ");

    // Validate position
    game::map::Point shipPos;
    if (!sh.getPosition().get(shipPos)) {
        throw Exception("Ship does not exist");
    }

    // Build action depending on whether this is going to be jettison or not
    game::actions::CargoTransferSetup setup;
    if (const Id_t planetId = turn.universe().findPlanetAt(shipPos)) {
        // Ship/Planet transfer. Note that this produces the wrong order.
        setup = game::actions::CargoTransferSetup::fromPlanetShip(turn.universe(), planetId, sh.getId());
        setup.swapSides();
    } else {
        // Jettison. Reject if not requested. The order is already correct.
        if ((flags & flagJettison) == 0) {
            throw Exception("Not at planet");
        }
        setup = game::actions::CargoTransferSetup::fromShipJettison(turn.universe(), sh.getId());
    }

    // We now have a transfer with the ship on the left side.
    // This is correct for "unload". If "upload" is requested, we must swap sides.
    if (reverse) {
        setup.swapSides();
    }

    // Proxy
    setProxyIfRequired(setup, turn, proxyId);

    // Do it
    doScriptTransfer(setup, process, flags, cargospec, session, mapConfig, turn, root);
}
