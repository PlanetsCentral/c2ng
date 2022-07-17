/**
  *  \file u/t_game_db.hpp
  *  \brief Tests for game::db
  */
#ifndef C2NG_U_T_GAME_DB_HPP
#define C2NG_U_T_GAME_DB_HPP

#include <cxxtest/TestSuite.h>

class TestGameDbDrawingAtomMap : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSave();
    void testLoad();
};

class TestGameDbFleetLoader : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testBroken();
    void testBrokenBadSig();
    void testBrokenBadVersion();
    void testBrokenTruncated();
    void testSimple();
    void testMoved();
    void testMovedRange();
    void testMovedName();
    void testDeleted();
    void testComments();
    void testConflict();
    void testSave();
    void testSaveEmpty();
    void testSaveBig();
};

class TestGameDbLoader : public CxxTest::TestSuite {
 public:
    void testLoad();
};

class TestGameDbPacker : public CxxTest::TestSuite {
 public:
    void testUfo();
    void testPlanet();
    void testFullPlanet();
    void testShip();
    void testFullShip();
};

class TestGameDbStructures : public CxxTest::TestSuite {
 public:
    void testHeader();
};

#endif
