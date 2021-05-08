/**
  *  \file u/t_ui.hpp
  *  \brief Tests for ui
  */
#ifndef C2NG_U_T_UI_HPP
#define C2NG_U_T_UI_HPP

#include <cxxtest/TestSuite.h>

class TestUiColorScheme : public CxxTest::TestSuite {
 public:
    void testBackground();
    void testColor();
};

class TestUiEventLoop : public CxxTest::TestSuite {
 public:
    void testStop();
    void testTask();
};

class TestUiGroup : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiInvisibleWidget : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiLayoutableGroup : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiPrefixArgument : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiRoot : public CxxTest::TestSuite {
 public:
    void testKeys();
};

class TestUiWidget : public CxxTest::TestSuite {
 public:
    void testDeathFocus();
    void testRemoveFocus();
};

#endif
