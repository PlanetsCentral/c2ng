/**
  *  \file game/vcr/classic/types.hpp
  *  \brief Types for Classic VCR
  */
#ifndef C2NG_GAME_VCR_CLASSIC_TYPES_HPP
#define C2NG_GAME_VCR_CLASSIC_TYPES_HPP

#include "afl/bits/smallset.hpp"
#include "afl/base/types.hpp"

namespace game { namespace vcr { namespace classic {

    /** Battle algorithm types. */
    enum Type {
        // xref game::vcr::classic::Battle::getAlgorithmName
        Unknown,            ///< Unknown.
        Host,               ///< Host.
        UnknownPHost,       ///< Unknown brand of PHost.
        PHost2,             ///< PHost 2.
        PHost3,             ///< PHost 3 or PHost 4 without extensions.
        PHost4,             ///< PHost 4 with extensions.
        NuHost              ///< NuHost.
    };

    /** Check for PHost-type battle.
        \param t Type to check
        \return true if PHost-style battle */
    bool isPHost(Type t);

    /** Status values (battle outcome). */
    enum BattleResult {
        LeftDestroyed,      ///< Left object has been destroyed.
        RightDestroyed,     ///< Right object has been destroyed.
        LeftCaptured,       ///< Left object has been captured. Only valid if left object is a ship; planets are destroyed.
        RightCaptured,      ///< Right object has been captured. Only valid if right object is a ship; planets are destroyed.
        Timeout,            ///< Battle timed out. Used alone.
        Stalemate,          ///< Stalemate (neither has ammo). Used alone.
        Invalid             ///< Battle cannot be played. Used alone.
    };

    /** Status bitset (battle outcome).
        Normally, this is a unit set.
        However, sometimes multiple "Destroyed" or "Captured" bits are set
        if both units reach that state in the same battle tick.

        The empty set means the status is not yet known. */
    typedef afl::bits::SmallSet<BattleResult> BattleResult_t;

    /** Fighter statuses.
        Those might still be hardcoded someplace, don't change! */
    enum FighterStatus {
        FighterIdle,
        FighterAttacks,
        FighterReturns
    };

    /** Side of a VCR. */
    enum Side {
        LeftSide,
        RightSide
    };

    /** Get other side.
        \param s Side
        \return other side */
    Side flipSide(Side s);

    /** Type for time into a battle.
        Battles start at time 0.
        Time is visualized as seconds since start in PCC. */
    typedef int32_t Time_t;

} } }

inline bool
game::vcr::classic::isPHost(Type t)
{
    switch (t) {
     case Unknown:      break;
     case Host:         break;
     case UnknownPHost: return true;
     case PHost2:       return true;
     case PHost3:       return true;
     case PHost4:       return true;
     case NuHost:       break;
    }
    return false;
}

inline game::vcr::classic::Side
game::vcr::classic::flipSide(Side s)
{
    // ex ccvcr.pas:Opp
    return s == LeftSide ? RightSide : LeftSide;
}

#endif
