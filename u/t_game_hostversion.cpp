/**
  *  \file u/t_game_hostversion.cpp
  *  \brief Test for game::HostVersion
  */

#include "game/hostversion.hpp"

#include "t_game.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"

using game::HostVersion;

/** Test formatting. */
void
TestGameHostVersion::testFormat()
{
    // Unknown
    TS_ASSERT_EQUALS(HostVersion().toString(), "unknown");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,0,0)).toString(), "unknown");

    // Tim-Host
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, 0).toString(), "Host");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,0,0)).toString(), "Host 3.0");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,16,1)).toString(), "Host 3.16.001");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,20,0)).toString(), "Host 3.20");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,22,27)).toString(), "Host 3.22.027");

    // PHost
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, 0).toString(), "PHost");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)).toString(), "PHost 3.0");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,16,1)).toString(), "PHost 3.16a");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,20,0)).toString(), "PHost 3.20");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,4,5)).toString(), "PHost 3.4e");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,22,27)).toString(), "PHost 3.22.027");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,4,13)).toString(), "PHost 3.4m");

    // SRace (Tim-Host variant)
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace, 0).toString(), "SRace");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace, MKVERSION(3,0,0)).toString(), "SRace 3.0");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace, MKVERSION(3,16,1)).toString(), "SRace 3.16.001");

    // NuHost
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost, 0).toString(), "NuHost");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost, MKVERSION(3,0,0)).toString(), "NuHost 3.0");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost, MKVERSION(3,16,1)).toString(), "NuHost 3.16.001");
}

/** Test accessors. */
void
TestGameHostVersion::testAccessor()
{
    HostVersion t;
    TS_ASSERT_EQUALS(t.getKind(), HostVersion::Unknown);
    TS_ASSERT_EQUALS(t.getVersion(), 0);

    t.set(HostVersion::PHost, MKVERSION(4,1,0));
    TS_ASSERT_EQUALS(t.getKind(), HostVersion::PHost);
    TS_ASSERT_EQUALS(t.getVersion(), MKVERSION(4,1,0));

    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,22,0)).getKind(), HostVersion::Host);
}

/** Test MKVERSION.
    These values are given to scripts and therefore should be verified against known values. */
void
TestGameHostVersion::testVersion()
{
    TS_ASSERT_EQUALS(MKVERSION(0,0,0),   0);
    TS_ASSERT_EQUALS(MKVERSION(3,22,46), 322046);
    TS_ASSERT_EQUALS(MKVERSION(4,1,5),   401005);
}

/** Test host properties. */
void
TestGameHostVersion::testProperties()
{
    game::config::HostConfiguration config;

    // getCommandArgumentLimit
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getCommandArgumentLimit(), 999);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getCommandArgumentLimit(), 999);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getCommandArgumentLimit(), 999);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(3, 2,0)).getCommandArgumentLimit(), 500);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).getCommandArgumentLimit(), 10000);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getCommandArgumentLimit(), 10000);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getCommandArgumentLimit(), 999);

    // hasDeathRays: PHost 4.0+
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasDeathRays());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasDeathRays());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasDeathRays());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasDeathRays());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasDeathRays());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasDeathRays());

    // hasExperienceLevels: PHost 4.0+
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasExperienceLevels());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasExperienceLevels());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasExperienceLevels());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasExperienceLevels());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasExperienceLevels());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasExperienceLevels());

    // hasShipSpecificFunctions: PHost 4.0+
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasShipSpecificFunctions());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasShipSpecificFunctions());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasShipSpecificFunctions());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasShipSpecificFunctions());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasShipSpecificFunctions());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasShipSpecificFunctions());

    // hasCumulativeHullfunc: PHost 4.0i+, 3.4k+
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasCumulativeHullfunc());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasCumulativeHullfunc());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasCumulativeHullfunc());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3,4,10)).hasCumulativeHullfunc());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3,4,11)).hasCumulativeHullfunc());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0,8)).hasCumulativeHullfunc());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,9)).hasCumulativeHullfunc());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasCumulativeHullfunc());

    // hasImmuneAssaultShip: all but PHost 4.0i+
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasImmuneAssaultShip());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasImmuneAssaultShip());
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasImmuneAssaultShip());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 5,0)).hasImmuneAssaultShip());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,8)).hasImmuneAssaultShip());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0,9)).hasImmuneAssaultShip());
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasImmuneAssaultShip());

    // hasHighTechTorpedoBug: Host 3.22.31+
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,31)).hasHighTechTorpedoBug());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,30)).hasHighTechTorpedoBug());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,31)).hasHighTechTorpedoBug());
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,31)).hasHighTechTorpedoBug());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 5,0)).hasHighTechTorpedoBug());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0,8)).hasHighTechTorpedoBug());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0,9)).hasHighTechTorpedoBug());
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3,22,31)).hasHighTechTorpedoBug());

    // hasSiliconoidDesertAdvantage: Tim and PHost 3.3+
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasSiliconoidDesertAdvantage());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasSiliconoidDesertAdvantage());
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasSiliconoidDesertAdvantage());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasSiliconoidDesertAdvantage());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 3,3)).hasSiliconoidDesertAdvantage());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasSiliconoidDesertAdvantage());
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasSiliconoidDesertAdvantage());

    // hasLargeCargoTransfer: PHost and Tim up to 3.22.30
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasLargeCargoTransfer());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasLargeCargoTransfer());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,30)).hasLargeCargoTransfer());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,31)).hasLargeCargoTransfer());
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasLargeCargoTransfer());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasLargeCargoTransfer());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasLargeCargoTransfer());
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasLargeCargoTransfer());

    // hasAutomaticMineIdentity: PHost 3.4c and newer
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasAutomaticMineIdentity());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasAutomaticMineIdentity());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,29)).hasAutomaticMineIdentity());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasAutomaticMineIdentity());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,2)).hasAutomaticMineIdentity());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,3)).hasAutomaticMineIdentity());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasAutomaticMineIdentity());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasAutomaticMineIdentity());

    // getPostTaxationHappinessLimit
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getPostTaxationHappinessLimit(), 31);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getPostTaxationHappinessLimit(), 31);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getPostTaxationHappinessLimit(), 31);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(3, 2,0)).getPostTaxationHappinessLimit(), 30);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).getPostTaxationHappinessLimit(), 30);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getPostTaxationHappinessLimit(), 30);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getPostTaxationHappinessLimit(), 31);

    // hasNegativeFCodes:
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasNegativeFCodes());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasNegativeFCodes());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasNegativeFCodes());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(2, 8,9)).hasNegativeFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(2, 9,0)).hasNegativeFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasNegativeFCodes());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasNegativeFCodes());

    // hasSpacePaddedFCodes: PHost 4.0h+, 3.4j+
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasSpacePaddedFCodes());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasSpacePaddedFCodes());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasSpacePaddedFCodes());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,9)).hasSpacePaddedFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3,4,10)).hasSpacePaddedFCodes());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0,7)).hasSpacePaddedFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,8)).hasSpacePaddedFCodes());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasSpacePaddedFCodes());

    // hasCaseInsensitiveUniversalMinefieldFCodes:
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 2,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasCaseInsensitiveUniversalMinefieldFCodes());

    // getNativeTaxRateLimit
    config[config.PlayerRace].set("1,2,3,4,5,6,7,8,9,10,11");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getNativeTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 100);

    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 75);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 75);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 75);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getNativeTaxRateLimit(2, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 75);

    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(6, config), 20);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getNativeTaxRateLimit(6, config), 20);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getNativeTaxRateLimit(6, config), 20);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getNativeTaxRateLimit(6, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getNativeTaxRateLimit(6, config), 20);

    config[config.PlayerRace].set("6,1,2,1");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 20);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(3, config), 75);

    // getColonistTaxRateLimit:
    config[config.PlayerRace].set("1,2,3,4,5,6,7,8,9,10,11");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getColonistTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);

    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 75);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 75);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 75);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getColonistTaxRateLimit(2, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 75);

    config[config.PlayerRace].set("6,1,2,1");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 100);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getColonistTaxRateLimit(3, config), 75);

    // isPHostRoundingMiningResults: PHost 4.1/3.5; does not apply to other-than-PHost (FIXME?)
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3,4,99)).isPHostRoundingMiningResults());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 5,0)).isPHostRoundingMiningResults());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4,0,99)).isPHostRoundingMiningResults());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 1,0)).isPHostRoundingMiningResults());

    // isExactHyperjumpDistance2: 340/360 is inclusive in PHost, but not in THost.
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,20,0)).isExactHyperjumpDistance2(340*340));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));

    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,20,0)).isExactHyperjumpDistance2(340*340+1));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));

    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,20,0)).isExactHyperjumpDistance2(360*360));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));

    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,20,0)).isExactHyperjumpDistance2(360*360-1));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));

    // getMinimumHyperjumpDistance2
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getMinimumHyperjumpDistance2(), 401);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getMinimumHyperjumpDistance2(), 401);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getMinimumHyperjumpDistance2(), 401);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getMinimumHyperjumpDistance2(), 1);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getMinimumHyperjumpDistance2(), 401);

    // isMissionAllowed: SRace cannot have mission 1
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isMissionAllowed(1));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isMissionAllowed(1));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isMissionAllowed(1));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isMissionAllowed(1));
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isMissionAllowed(1));

    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isMissionAllowed(2));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isMissionAllowed(2));
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isMissionAllowed(2));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isMissionAllowed(2));
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isMissionAllowed(2));

    // hasMinefieldCenterBug: all TimHost
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasMinefieldCenterBug());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasMinefieldCenterBug());
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasMinefieldCenterBug());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasMinefieldCenterBug());
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasMinefieldCenterBug());

    // isMineLayingAfterMineDecay: all PHost
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isMineLayingAfterMineDecay());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isMineLayingAfterMineDecay());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isMineLayingAfterMineDecay());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isMineLayingAfterMineDecay());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isMineLayingAfterMineDecay());

    // isRoundingMineDecay: all TimHost
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isRoundingMineDecay());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isRoundingMineDecay());
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isRoundingMineDecay());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isRoundingMineDecay());
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isRoundingMineDecay());

    // isBeamRequiredForMineScooping: all PHost
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isBeamRequiredForMineScooping());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isBeamRequiredForMineScooping());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isBeamRequiredForMineScooping());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isBeamRequiredForMineScooping());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isBeamRequiredForMineScooping());

    // isPBPGame: all TimHost, and PHost if configured
    config[config.BuildQueue].set("PAL");
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isPBPGame(config));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isPBPGame(config));
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isPBPGame(config));
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isPBPGame(config));
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isPBPGame(config));

    config[config.BuildQueue].set("PBP");
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isPBPGame(config));

    // isEugeneGame:
    config[config.FuelUsagePerFightFor100KT].set(0);
    config[config.FuelUsagePerTurnFor100KT].set(0);
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isEugeneGame(config));

    config[config.FuelUsagePerFightFor100KT].set("0,1,0,0,0");
    config[config.FuelUsagePerTurnFor100KT].set(0);
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isEugeneGame(config));

    config[config.FuelUsagePerFightFor100KT].set(0);
    config[config.FuelUsagePerTurnFor100KT].set("0,1,0,0,0");
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isEugeneGame(config));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isEugeneGame(config));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isEugeneGame(config));

    // hasDoubleTorpedoPower: everything with non-AlternativeCombat
    config[config.AllowAlternativeCombat].set(false);
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasDoubleTorpedoPower(config));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasDoubleTorpedoPower(config));
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasDoubleTorpedoPower(config));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasDoubleTorpedoPower(config));
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasDoubleTorpedoPower(config));

    config[config.AllowAlternativeCombat].set(true);
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasDoubleTorpedoPower(config));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasDoubleTorpedoPower(config));
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasDoubleTorpedoPower(config));
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasDoubleTorpedoPower(config));
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasDoubleTorpedoPower(config));

    // hasParallelShipTransfers: all but NuHost
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasParallelShipTransfers());
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasParallelShipTransfers());
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasParallelShipTransfers());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasParallelShipTransfers());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasParallelShipTransfers());

    // hasExtendedMissions: PHost, if enabled
    config[config.AllowExtendedMissions].set(false);
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasExtendedMissions(config));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasExtendedMissions(config));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasExtendedMissions(config));
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasExtendedMissions(config));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasExtendedMissions(config));

    config[config.AllowExtendedMissions].set(true);
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasExtendedMissions(config));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasExtendedMissions(config));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasExtendedMissions(config));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasExtendedMissions(config));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasExtendedMissions(config));

    // hasAccurateFuelModelBug: PHost <3.4h, 4.0e
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasAccurateFuelModelBug());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasAccurateFuelModelBug());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasAccurateFuelModelBug());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasAccurateFuelModelBug());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasAccurateFuelModelBug());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,8)).hasAccurateFuelModelBug());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 5,0)).hasAccurateFuelModelBug());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0,5)).hasAccurateFuelModelBug());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 1,0)).hasAccurateFuelModelBug());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasAccurateFuelModelBug());

    // hasAlchemyCombinations: PHost >= 4.0i, 3.4k
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22, 0)).hasAlchemyCombinations());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22, 0)).hasAlchemyCombinations());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22, 0)).hasAlchemyCombinations());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4, 0)).hasAlchemyCombinations());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0, 0)).hasAlchemyCombinations());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,11)).hasAlchemyCombinations());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 5, 0)).hasAlchemyCombinations());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0, 9)).hasAlchemyCombinations());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 1, 0)).hasAlchemyCombinations());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0, 0)).hasAlchemyCombinations());

    // hasRefineryFCodes: PHost >= 3.4m, 4.0k
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22, 0)).hasRefineryFCodes());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22, 0)).hasRefineryFCodes());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22, 0)).hasRefineryFCodes());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4, 0)).hasRefineryFCodes());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(4, 0, 0)).hasRefineryFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,13)).hasRefineryFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 5, 0)).hasRefineryFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 0,11)).hasRefineryFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(4, 1, 0)).hasRefineryFCodes());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0, 0)).hasRefineryFCodes());

    // hasAlchemyExclusionFCodes: PHost only
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasAlchemyExclusionFCodes());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasAlchemyExclusionFCodes());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasAlchemyExclusionFCodes());
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasAlchemyExclusionFCodes());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasAlchemyExclusionFCodes());

    // isAlchemyRounding
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isAlchemyRounding());
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isAlchemyRounding());
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isAlchemyRounding());
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isAlchemyRounding());
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isAlchemyRounding());

    // isValidChunnelDistance2
    // - 10000 (=100 ly) is ok for everyone
    config[config.MinimumChunnelDistance].set(100);
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isValidChunnelDistance2(10000, config));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isValidChunnelDistance2(10000, config));
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isValidChunnelDistance2(10000, config));
    TS_ASSERT( HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isValidChunnelDistance2(10000, config));
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isValidChunnelDistance2(10000, config));

    // - 9901 (=99.5 ly) is ok for Host
    TS_ASSERT( HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isValidChunnelDistance2(9901, config));
    TS_ASSERT( HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isValidChunnelDistance2(9901, config));
    TS_ASSERT( HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isValidChunnelDistance2(9901, config));
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isValidChunnelDistance2(9901, config));
    TS_ASSERT( HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isValidChunnelDistance2(9901, config));

    // - 100 (=10 ly) is not ok for anyone
    TS_ASSERT(!HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isValidChunnelDistance2(10, config));
    TS_ASSERT(!HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isValidChunnelDistance2(10, config));
    TS_ASSERT(!HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isValidChunnelDistance2(10, config));
    TS_ASSERT(!HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isValidChunnelDistance2(10, config));
    TS_ASSERT(!HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isValidChunnelDistance2(10, config));

    // getMinimumFuelToInitiateChunnel
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getMinimumFuelToInitiateChunnel(), 50);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getMinimumFuelToInitiateChunnel(), 50);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getMinimumFuelToInitiateChunnel(), 50);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).getMinimumFuelToInitiateChunnel(), 51);
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).getMinimumFuelToInitiateChunnel(), 50);
}

/** Test setImpliedHostConfiguration(). */
void
TestGameHostVersion::testSetImpliedHostConfiguration()
{
    using game::config::HostConfiguration;

    // Base case
    {
        HostConfiguration c;
        c.setDefaultValues();
        TS_ASSERT_EQUALS(c[HostConfiguration::CPEnableShow](), true);
    }

    // Host
    {
        HostConfiguration c;
        c.setDefaultValues();
        HostVersion(HostVersion::Host, MKVERSION(3,22,0)).setImpliedHostConfiguration(c);
        TS_ASSERT_EQUALS(c[HostConfiguration::CPEnableShow](), false);
    }

    // Old PHost
    {
        HostConfiguration c;
        c.setDefaultValues();
        HostVersion(HostVersion::PHost, MKVERSION(3,2,5)).setImpliedHostConfiguration(c);
        TS_ASSERT_EQUALS(c[HostConfiguration::CPEnableShow](), false);
    }

    // New PHost
    {
        HostConfiguration c;
        c.setDefaultValues();
        HostVersion(HostVersion::PHost, MKVERSION(4,1,5)).setImpliedHostConfiguration(c);
        TS_ASSERT_EQUALS(c[HostConfiguration::CPEnableShow](), true);
    }

    // ...but it's not unconditionally enabled
    {
        HostConfiguration c;
        c.setDefaultValues();
        c[HostConfiguration::CPEnableShow].set(false);
        HostVersion(HostVersion::PHost, MKVERSION(4,1,5)).setImpliedHostConfiguration(c);
        TS_ASSERT_EQUALS(c[HostConfiguration::CPEnableShow](), false);
    }
}

/** Test setImpliedHostConfiguration(). */
void
TestGameHostVersion::testSetImpliedHostConfigurationMine()
{
    using game::config::HostConfiguration;

    {
        HostConfiguration c;
        c[HostConfiguration::UnitsPerTorpRate].set("1,2,3,4,5,6,7,8,9,10");
        HostVersion(HostVersion::Host, MKVERSION(3,22,40)).setImpliedHostConfiguration(c);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerTorpRate](1), 100);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerTorpRate](6), 100);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerTorpRate](9), 400);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerWebRate](1), 100);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerWebRate](6), 100);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerWebRate](9), 400);
    }

    {
        HostConfiguration c;
        c[HostConfiguration::UnitsPerTorpRate].set("1,2,3,4,5,6,7,8,9,10");
        HostVersion(HostVersion::PHost, MKVERSION(3,2,5)).setImpliedHostConfiguration(c);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerTorpRate](1), 1);   // set above
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerTorpRate](6), 6);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerTorpRate](9), 9);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerWebRate](1), 100);  // default
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerWebRate](6), 100);
        TS_ASSERT_EQUALS(c[HostConfiguration::UnitsPerWebRate](9), 400);
    }
}

/** Test fromString(). */
void
TestGameHostVersion::testFromString()
{
    HostVersion v;

    // Unknown
    TS_ASSERT(!v.fromString("unknown"));

    // Tim-Host
    TS_ASSERT(!v.fromString("Host"));

    TS_ASSERT(v.fromString("Host 3.0"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::Host);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,0,0));

    TS_ASSERT(v.fromString("Host 3.16.001"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::Host);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,16,1));

    TS_ASSERT(v.fromString("Host 3.2"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::Host);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,20,0));

    TS_ASSERT(v.fromString("Host 3.20"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::Host);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,20,0));

    TS_ASSERT(v.fromString("Host 3.20a"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::Host);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,20,1));

    TS_ASSERT(v.fromString("Host 3.22.027"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::Host);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,22,27));

    // - 2-arg variant
    TS_ASSERT(v.fromString("host", "3.0"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::Host);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,0,0));

    // PHost
    TS_ASSERT(!v.fromString("PHost"));

    TS_ASSERT(v.fromString("PHost 3.0"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::PHost);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,0,0));

    TS_ASSERT(v.fromString("PHost 3.16a"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::PHost);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,16,1));

    TS_ASSERT(v.fromString("PHost 3.20"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::PHost);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,20,0));

    TS_ASSERT(v.fromString("PHost 3.4e"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::PHost);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,4,5));

    TS_ASSERT(v.fromString("PHost 3.22.027"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::PHost);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,22,27));

    TS_ASSERT(v.fromString("PHost 3.4m"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::PHost);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,4,13));

    // SRace (Tim-Host variant)
    TS_ASSERT(!v.fromString("SRace"));

    TS_ASSERT(v.fromString("SRace 3.0"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::SRace);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,0,0));

    TS_ASSERT(v.fromString("SRace 3.16.001"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::SRace);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,16,1));

    // NuHost
    TS_ASSERT(!v.fromString("NuHost"));

    TS_ASSERT(v.fromString("NuHost 3.0"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::NuHost);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,0,0));

    TS_ASSERT(v.fromString("NuHost 3.16.001"));
    TS_ASSERT_EQUALS(v.getKind(), HostVersion::NuHost);
    TS_ASSERT_EQUALS(v.getVersion(), MKVERSION(3,16,1));
}

