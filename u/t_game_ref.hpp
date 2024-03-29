/**
  *  \file u/t_game_ref.hpp
  *  \brief Tests for game::ref
  */
#ifndef C2NG_U_T_GAME_REF_HPP
#define C2NG_U_T_GAME_REF_HPP

#include <cxxtest/TestSuite.h>

class TestGameRefConfiguration : public CxxTest::TestSuite {
 public:
    void testPreferences();
    void testCreatePredicateSortById();
    void testCreatePredicateSortByOwner();
    void testCreatePredicateSortByHull();
    void testCreatePredicateSortByMass();
    void testCreatePredicateSortByFleet();
    void testCreatePredicateSortByTowGroup();
    void testCreatePredicateSortByBattleOrder();
    void testCreatePredicateSortByPosition();
    void testCreatePredicateSortByHullMass();
    void testCreatePredicateSortByDamage();
    void testCreatePredicateSortByName();
    void testCreatePredicateSortByNextPosition();
    void testCreatePredicateSortByTransferTarget();
    void testCreatePredicateFromConfig();
    void testBlank();
};

class TestGameRefFleetList : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testSort1();
    void testSort2();
    void testAdd();
};

class TestGameRefFleetMemberList : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testSort1();
    void testSort2();
    void testSet();
    void testSet2();
};

class TestGameRefHistoryShipList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSort1();
    void testSort2();
};

class TestGameRefHistoryShipSelection : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testModeSet();
    void testBuildList();
    void testBuildListHist();
    void testBuildListHist2();
};

class TestGameRefList : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testAddObjectsAt();
};

class TestGameRefListObserver : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameRefNullPredicate : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameRefSortBy : public CxxTest::TestSuite {
 public:
    void testId();
    void testName();
    void testOwner();
    void testPosition();
    void testNextPosition();
    void testDamage();
    void testMass();
    void testHullMass();
    void testHullType();
    void testBattleOrder();
    void testFleet();
    void testTowGroup();
    void testTransferTarget();
};

class TestGameRefSortPredicate : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testThen();
};

class TestGameRefTypeAdaptor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameRefUserList : public CxxTest::TestSuite {
 public:
    void testMakeReferenceItem();
    void testAdd();
    void testAddList();
};

#endif
