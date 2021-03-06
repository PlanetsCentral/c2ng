/**
  *  \file game/map/planet.hpp
  *  \brief Class game::map::Planet
  */
#ifndef C2NG_GAME_MAP_PLANET_HPP
#define C2NG_GAME_MAP_PLANET_HPP

#include "afl/base/optional.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/element.hpp"
#include "game/hostversion.hpp"
#include "game/map/basedata.hpp"
#include "game/map/object.hpp"
#include "game/map/planetdata.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/playerset.hpp"
#include "game/spec/hullassignmentlist.hpp"
#include "game/unitscorelist.hpp"

namespace game { namespace map {

    class Configuration;
    class Universe;

    /** Planet.
        This stores data of a planet and possibly a starbase.
        It is used for played and scanned planets.

        - We always know name and Id of all planets
        - A planet can exist or not:
          - if it exists, we know its position, but not necessarily its other data (!= NoPlanet)
          - if it does not exist, we do not know its position. We may still know some other data (from scanning) (== NoPlanet)
        - A planet can have three levels of data:
          - if we're playing it, we have full data (hasFullPlanetData() == CurrentPlanet); we may have full data for some other planets as well
          - if we've seen it somehow, we have partial data (hasAnyPlanetData() == KnownPlanet).
          - otherwise, we don't know anything about it == UnknownPlanet, NoPlanet
        - A starbase can have two levels of data:
          - if we're playing the planet, the base is either present or not, and if it's present (haveBaseData()), we have full data
          - if we're not playing the planet, we only know whether there is a base or not.

        Changes from PCC2:
        - we no longer distinguish dat/dis by using two instances; instead, we store before and after in the same object.
          This has the advantage that we don't have to duplicate and maintain information like X/Y, name, and status.
        - removed orbit flags
        - removed cargo arbiter */
    class Planet : public Object {
     public:
        enum BaseKind {
            UnknownBase,            ///< We do not know whether there is a base.
            NoBase,                 ///< We know that there is no base.
            ExistingBase,           ///< We know that there is a base.
            KnownBase,              ///< We know that there is a base, and have (partial or full) data.
            CurrentBase             ///< We have a BDATA.DAT entry for this base.
        };

        enum PlanetKind {
            NoPlanet,               ///< The planet does not exist. We do not know its position.
            HiddenPlanet,           ///< We do not know this planet's position, but we still have some data from sensor scans.
            UnknownPlanet,          ///< This planet exists but we do not know anything about it but its position.
            KnownPlanet,            ///< This planet exists and we have some information about it.
            CurrentPlanet           ///< We have a PDATAx.DAT entry for this planet.
        };

        enum Timestamp {
            MineralTime,            ///< Mined/ground/density
            ColonistTime,           ///< Population/owner/industry
            NativeTime,             ///< Native gov/pop/race
            CashTime                ///< Cash/supplies
        };

        struct AutobuildSettings {
            IntegerProperty_t goal[NUM_PLANETARY_BUILDING_TYPES];
            IntegerProperty_t speed[NUM_PLANETARY_BUILDING_TYPES];
        };


        /** Construct new planet.
            \param id Id */
        explicit Planet(Id_t id);

        /** Copy a planet.
            \param other Other planet */
        Planet(const Planet& other);

        /** Destructor. */
        ~Planet();


        /*
         *  Load and Save
         */

        /** Add planet .dat file entry.
            \param data Parsed data file
            \param source Source flag to use for this entry */
        void addCurrentPlanetData(const PlanetData& data, PlayerSet_t source);

        /** Add starbase .dat file entry.
            \param data Parsed data file
            \param source Source flag to use for this entry */
        void addCurrentBaseData(const BaseData& data, PlayerSet_t source);

        /** Add message information.
            Processes information received from messages, history, or util.dat.
            \param info Information */
        void addMessageInformation(const game::parser::MessageInformation& info);

        /** Set position.
            \param pos Position */
        void setPosition(Point pos);

        /** Set planet name.
            \param name Name */
        void setName(const String_t& name);

        /** Get name.
            \param tx Translator */
        String_t getName(afl::string::Translator& tx) const;

        /** Set whether non-existance of this planet is known.
            There is no way to explicitly specify that a planet does not exist.
            To build maps with fewer than 500 planets, people move planets to far-away positions.
            Recent PHosts send a util.dat message whenever they consider a planet to be non-existant,
            to make sure that the clients' idea of which planets do exist agrees with PHost's.
            \param value flag (true: planet does not exist) */
        void setKnownToNotExist(bool value);

        /** Get current planet data for storage.
            \param [out] data */
        void getCurrentPlanetData(PlanetData& data) const;

        /** Get current starbase data for storage.
            \param [out] data */
        void getCurrentBaseData(BaseData& data) const;

        /** Do internal checks for this planet.
            Internal checks do not require a partner to interact with.
            This will determine the planet kind,
            fix possible the problems, and log appropriate messages.
            \param config Map configuration
            \param tx Translator
            \param log Logger */
        void internalCheck(const Configuration& config, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Combined checks, phase 2.
            This will do all post-processing which needs a partner to interact with.
            It requires the playability to be filled in.
            \param univ Universe
            \param availablePlayers Players for which we have current data
            \param turnNumber Turn number */
        void combinedCheck2(const Universe& univ, PlayerSet_t availablePlayers, int turnNumber);


        /*
         *  Object interface:
         */

        virtual String_t getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;
        virtual bool getPosition(Point& result) const;


        /*
         *  Planet status accessors:
         */

        bool        isVisible() const;
        PlayerSet_t getPlanetSource() const;
        void        addPlanetSource(PlayerSet_t p);
        bool        hasAnyPlanetData() const;
        bool        hasFullPlanetData() const;
        int         getHistoryTimestamp(Timestamp kind) const;


        /*
         *  Base status accessors:
         */

        PlayerSet_t getBaseSource() const;
        void        addBaseSource(PlayerSet_t p);
        bool        hasBase() const;
        bool        hasFullBaseData() const;

        /*
         *  Owner accessors:
         */
        void               setOwner(IntegerProperty_t owner);

        // Structure accessors:
        IntegerProperty_t  getNumBuildings(PlanetaryBuilding kind) const;
        void               setNumBuildings(PlanetaryBuilding kind, IntegerProperty_t n);

        IntegerProperty_t  getIndustryLevel(const HostVersion& host) const;
        static int         getIndustryLevel(int mifa, const HostVersion& host);
        void               setIndustryLevel(IntegerProperty_t level, const HostVersion& host);

        // Colonist accessors:
        NegativeProperty_t getColonistHappiness() const;
        void               setColonistHappiness(NegativeProperty_t happiness);
        IntegerProperty_t  getColonistTax() const;
        void               setColonistTax(IntegerProperty_t tax);

        // Native accessors:
        IntegerProperty_t  getNativeGovernment() const;
        void               setNativeGovernment(IntegerProperty_t gov);
        NegativeProperty_t getNativeHappiness() const;
        void               setNativeHappiness(NegativeProperty_t happiness);
        IntegerProperty_t  getNativeRace() const;
        void               setNativeRace(IntegerProperty_t race);
        IntegerProperty_t  getNativeTax() const;
        void               setNativeTax(IntegerProperty_t tax);
        LongProperty_t     getNatives() const;
        void               setNatives(LongProperty_t natives);

        bool               isKnownToHaveNatives() const;
        void               setKnownToHaveNatives(bool known);

        // FCode accessors:
        StringProperty_t   getFriendlyCode() const;
        void               setFriendlyCode(StringProperty_t fc);

        // Starbase building accessors:
        bool               isBuildingBase() const;
        void               setBuildBaseFlag(bool b);

        // Environment accessors:
        IntegerProperty_t  getOreDensity(Element::Type type) const;
        void               setOreDensity(Element::Type type, IntegerProperty_t amount);
        LongProperty_t     getOreGround(Element::Type type) const;
        void               setOreGround(Element::Type type, LongProperty_t amount);
        IntegerProperty_t  getTemperature() const;
        void               setTemperature(IntegerProperty_t value);

        // Cargo accessors:
        LongProperty_t     getCargo(Element::Type type) const;
        void               setCargo(Element::Type type, LongProperty_t amount);

        // Simple base accessors:
        IntegerProperty_t  getBaseDamage() const;
        void               setBaseDamage(IntegerProperty_t n);
        IntegerProperty_t  getBaseMission() const;
        void               setBaseMission(IntegerProperty_t mission);
        IntegerProperty_t  getBaseTechLevel(TechLevel level) const;
        void               setBaseTechLevel(TechLevel level, IntegerProperty_t value);

        // Shipyard accessors:
        IntegerProperty_t  getBaseShipyardAction() const;
        IntegerProperty_t  getBaseShipyardId() const;
        void               setBaseShipyardOrder(IntegerProperty_t action, IntegerProperty_t id);

        // Component storage accessors:
        // for hulls, this is a TRUEHULL SLOT, not a HULL NUMBER.
        IntegerProperty_t  getBaseStorage(TechLevel area, int slot) const;
        void               setBaseStorage(TechLevel area, int slot, IntegerProperty_t amount);

        // Build order accessors:
        IntegerProperty_t  getBaseBuildHull(const game::config::HostConfiguration& config, const game::spec::HullAssignmentList& map) const;
        ShipBuildOrder     getBaseBuildOrder() const;
        void               setBaseBuildOrder(const ShipBuildOrder& order);
        IntegerProperty_t  getBaseBuildOrderHullIndex() const;

        // Build queue accessors:
        IntegerProperty_t  getBaseQueuePosition() const;
        void               setBaseQueuePosition(IntegerProperty_t pos);
        LongProperty_t     getBaseQueuePriority() const;
        void               setBaseQueuePriority(LongProperty_t pri);

        // Auto build accessors:
        int         getAutobuildGoal(PlanetaryBuilding ps) const;
        void        setAutobuildGoal(PlanetaryBuilding ps, int value);
        int         getAutobuildSpeed(PlanetaryBuilding ps) const;
        void        setAutobuildSpeed(PlanetaryBuilding ps, int value);
        void        applyAutobuildSettings(const AutobuildSettings& settings);

        UnitScoreList& unitScores();
        const UnitScoreList& unitScores() const;

     private:
        Id_t m_id;              // ID, always known
        String_t m_name;        // Name, always known

        afl::base::Optional<Point> m_position;    // Position
        bool m_knownToNotExist; // Override saying this planet does not exist

        PlanetData m_currentPlanetData;

        BaseData m_currentBaseData;

        BaseKind m_baseKind;
        PlanetKind m_planetKind;

        // Source flags. These specify which players' .dat files contained
        // the PDATA/BDATA records that make up this planet. They do NOT
        // necessarily mean that we (a) play this unit and (b) know
        // everything about it. We will, however, assume that we know
        // everything about the units we play.
        PlayerSet_t m_planetSource;
        PlayerSet_t m_baseSource;

        // Planet extra info
        int m_historyTimestamps[4];
        bool m_isPlanetKnownToHaveNatives;
        IntegerProperty_t m_industryLevel;

        int m_autobuildGoals[NUM_PLANETARY_BUILDING_TYPES];
        int m_autobuildSpeeds[NUM_PLANETARY_BUILDING_TYPES];

        // Base extra info
        IntegerProperty_t m_queuePosition;
        LongProperty_t m_queuePriority;

        UnitScoreList m_unitScores;
    };

} }

#endif
