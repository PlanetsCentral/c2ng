/**
  *  \file game/vcr/battle.hpp
  *  \brief Base class game::vcr::Battle
  */
#ifndef C2NG_GAME_VCR_BATTLE_HPP
#define C2NG_GAME_VCR_BATTLE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/point.hpp"
#include "game/playerlist.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace vcr {

    class Object;

    /** Entry in a VCR database. */
    class Battle : public afl::base::Deletable {
     public:
        static const int NeedQuickOutcome = 1;   ///< Caller needs the result of getOutcome(x).
        static const int NeedCompleteResult = 2; ///< Caller needs the result of getObject(x, true).

        enum Playability {
            IsPlayable,                        ///< Fight is playable.
            IsNotSupported,                    ///< We cannot play it and know why.
            IsDamaged                          ///< We cannot play it and don't know why. Might be host error.
        };

        /** Type of auxiliary information.
            See getAuxiliaryInformation(). */
        enum AuxInfo {
            aiSeed,                 ///< Seed. Internal clients will use a stronger-typed derived class functions. */
            aiMagic,                ///< Magic. Relevant for classic combat.
            aiType,                 ///< Type. Relevant for classic combat.
            aiFlags,                ///< Flags. Relevant for classic combat.
            aiAmbient               ///< Ambient. Relevant for FLAK.
        };


        /** Get number of objects. */
        virtual size_t getNumObjects() const = 0;

        /** Get an object participating in the fight.
            If the result is requested but not yet known, needs not compute it; use prepareResult() to reliably obtain results.
            If the fight cannot be played, but after=true is requested, treat that as after=false.
            \param slot Slot, [0,getNumObjects())
            \param after false to return beginning of fight, true to return after. */
        virtual const Object* getObject(size_t slot, bool after) const = 0;

        /** Get outcome for an object.
            Can be one of:
            - -1 = unit got destroyed
            - 0 = unit survived or fight not playable
            - positive = unit got captured by specified player
            \param slot Slot, [0,getNumObjects()) */
        virtual int getOutcome(const game::config::HostConfiguration& config,
                               const game::spec::ShipList& shipList,
                               size_t slot) = 0;

        /** Check whether this fight is playable.
            Should operate quickly. */
        virtual Playability getPlayability(const game::config::HostConfiguration& config,
                                           const game::spec::ShipList& shipList) = 0;

        /** Prepare the result.
            Compute this fight's result.
            If the result is already computed, just return.
            \param resultLevel Requested result level (NeedQuickOutcome, NeedCompleteResult, or combination thereof).
            \param pm Progress monitor */
        virtual void prepareResult(const game::config::HostConfiguration& config,
                                   const game::spec::ShipList& shipList,
                                   int resultLevel) = 0;

        /** Get name of algorithm used to play this fight.
            \param tx Translator. Pass user translator to obtain localized name, NullTranslator to obtain raw name. */
        virtual String_t getAlgorithmName(afl::string::Translator& tx) const = 0;

        /** Check whether Engine/Shield Bonus is active in this fight. */
        virtual bool isESBActive(const game::config::HostConfiguration& config) const = 0;

        /** Get position of this battle on the map.
            \param [out] result Position
            \return true: position was known, result set; false: result not set */
        virtual bool getPosition(game::map::Point& result) const = 0;

        /** Get auxiliary information.
            This information is intended for external clients (=scripts, c2play-server).
            \param info Value to query
            \return Value if known */
        virtual afl::base::Optional<int32_t> getAuxiliaryInformation(AuxInfo info) const = 0;

        /** Get result summary in human-readable form.
            For classic fights, this produces a line such as "We won (100 EP)".
            \param viewpointPlayer Viewpoint player; to be able to use "we" in the result
            \param config          Host configuration (for score computation)
            \param shipList        Ship list (for score computation)
            \param fmt             Number formatter
            \param tx              Translator
            \return text */
        virtual String_t getResultSummary(int viewpointPlayer,
                                          const game::config::HostConfiguration& config, const game::spec::ShipList& shipList,
                                          util::NumberFormatter fmt, afl::string::Translator& tx) const = 0;


        /*
         *  Useful methods
         */

        /** Describe a battle.
            The idea is to say "<name> vs <name>" in 1:1 fights, and "<race> vs <race>" in fleet battles with two participating races.
            \param players Player list
            \param tx Translator
            \return description */
        String_t getDescription(const game::PlayerList& players, afl::string::Translator& tx) const;
    };

} }

#endif
