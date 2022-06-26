/**
  *  \file game/test/files.hpp
  *  \brief File images for testing
  */
#ifndef C2NG_GAME_TEST_FILES_HPP
#define C2NG_GAME_TEST_FILES_HPP

#include "afl/base/memory.hpp"

namespace game { namespace test {

    /*
     *  Result Files
     */

    /** Get simple v3.0 result file. */
    afl::base::ConstBytes_t getResultFile30();

    /** Get simple v3.5 result file. */
    afl::base::ConstBytes_t getResultFile35();

    /** Get complex v3.0 result file.
        This file contains many ships, planets, targets, some messages. */
    afl::base::ConstBytes_t getComplexResultFile();

    /*
     *  Simulation Setups
     *
     *  These files have all been created by then-current CCBSim versions.
     */

    /** Get v0 simulation setup. */
    afl::base::ConstBytes_t getSimFileV0();

    /** Get v1 simulation setup. */
    afl::base::ConstBytes_t getSimFileV1();

    /** Get v2 simulation setup. */
    afl::base::ConstBytes_t getSimFileV2();

    /** Get v3 simulation setup. */
    afl::base::ConstBytes_t getSimFileV3();

    /** Get v4 simulation setup. */
    afl::base::ConstBytes_t getSimFileV4();

    /** Get v5 simulation setup. */
    afl::base::ConstBytes_t getSimFileV5();

    /*
     *  Specification/defaults
     */

    /** Get default registration key file (fizz.bin). */
    afl::base::ConstBytes_t getDefaultRegKey();

    /** Get default racename file (race.nm). */
    afl::base::ConstBytes_t getDefaultRaceNames();

    /** Get default planet coordinates file (xyplan.dat). */
    afl::base::ConstBytes_t getDefaultPlanetCoordinates();

    /** Get default planet name file (planet.nm). */
    afl::base::ConstBytes_t getDefaultPlanetNames();

    /** Get default beams (beamspec.dat). */
    afl::base::ConstBytes_t getDefaultBeams();

    /** Get default torpedoes (torpspec.dat). */
    afl::base::ConstBytes_t getDefaultTorpedoes();

    /** Get default hulls (hullspec.dat). */
    afl::base::ConstBytes_t getDefaultHulls();

    /** Get default engines (engspec.dat). */
    afl::base::ConstBytes_t getDefaultEngines();

    /** Get default hull assignments (truehull.dat). */
    afl::base::ConstBytes_t getDefaultHullAssignments();

} }

#endif
