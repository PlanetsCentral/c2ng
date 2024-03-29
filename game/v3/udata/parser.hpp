/**
  *  \file game/v3/udata/parser.hpp
  *  \brief Class game::v3::udata::Parser
  */
#ifndef C2NG_GAME_V3_UDATA_PARSER_HPP
#define C2NG_GAME_V3_UDATA_PARSER_HPP

#include "afl/charset/charset.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/spec/shiplist.hpp"
#include "game/unitscorelist.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/udata/reader.hpp"
#include "game/vcr/object.hpp"
#include "util/atomtable.hpp"
#include "util/vector.hpp"

namespace game { namespace v3 { namespace udata {

    /** Util.dat parser.
        Contains logic to read a util.dat file and assimilate its content into a game. */
    class Parser : public Reader {
     public:
        /** Constructor.
            \param game       Game. Data will be stored here (mostly, its currentTurn(), but also scores and score definitions)
            \param playerNr   Player number.
            \param config     Host configuration. Could be updated by received data.
            \param host       Host version.
            \param shipList   Ship list. The modified hull function list could be updated by received data.
            \param atomTable  Atom table (for marker tags).
            \param cs         Character set. Used for decoding strings.
            \param tx         Translator
            \param log        Logger */
        Parser(Game& game,
               int playerNr,
               game::config::HostConfiguration& config,
               game::HostVersion host,
               game::spec::ShipList& shipList,
               util::AtomTable& atomTable,
               afl::charset::Charset& cs,
               afl::string::Translator& tx,
               afl::sys::LogListener& log);

        virtual ~Parser();

        /** Perform post-processing when we have no util.dat file.
            Essentially, pretends that there is an empty util.dat file and processes that.
            This will synthesize some data (mainly, information derived from VCRs).
            This responsibility isn't perfectly placed but will have to do for now. */
        void handleNoUtilData();

        // Reader:
        virtual bool handleRecord(uint16_t recordId, afl::base::ConstBytes_t data);
        virtual void handleError(afl::io::Stream& in);
        virtual void handleEnd();

     private:
        enum Scope {
            ShipScope,
            PlanetScope
        };

        // Fixed data
        Game& m_game;
        const int m_player;
        game::config::HostConfiguration& m_hostConfiguration;
        HostVersion m_hostVersion;
        game::spec::ShipList& m_shipList;
        util::AtomTable& m_atomTable;
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;

        // Dynamic data
        util::Vector<uint8_t,Id_t> m_destroyedShips;
        std::vector<game::v3::structures::Util7Battle> m_battleResults;

        int getTurnNumber() const;
        UnitScoreList* getUnitScoreList(Scope scope, Id_t id);

        void markShipKilled(Id_t id);
        void processAlliances(const game::v3::structures::Util22Alliance& allies);
        void processEnemies(uint16_t enemies);
        void processExperienceLevel(const game::vcr::Object& obj);
        void processScoreRecord(afl::base::ConstBytes_t data, Scope scope, UnitScoreDefinitionList& defs);
        void processMessageInformation(const game::parser::MessageInformation& info);
    };

} } }

#endif
