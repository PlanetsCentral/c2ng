/**
  *  \file u/t_util.hpp
  *  \brief Tests for util
  */
#ifndef C2NG_U_T_UTIL_HPP
#define C2NG_U_T_UTIL_HPP

#include <cxxtest/TestSuite.h>

class TestUtilAnswerProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilApplication : public CxxTest::TestSuite {
 public:
    void testInit();
    void testExit();
};

class TestUtilAtomTable : public CxxTest::TestSuite {
 public:
    void testAtom();
};

class TestUtilBackupFile : public CxxTest::TestSuite {
 public:
    void testExpand();
};

class TestUtilBaseSlaveRequest : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilBaseSlaveRequestSender : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCall();
};

class TestUtilConfigurationFileParser : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUtilConstantAnswerProvider : public CxxTest::TestSuite {
 public:
    void testIt();
    void testYes();
    void testNo();
};

class TestUtilFileNamePattern : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFail();
    void testLiterals();
    void testCopy();
    void testPrepared();
};

class TestUtilFileParser : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUtilIo : public CxxTest::TestSuite {
 public:
    void testStorePascalString();
    void testStorePascalStringTruncate();
};

class TestUtilKey : public CxxTest::TestSuite {
 public:
    void testParse();
    void testFormat();
    void testUnique();
    void testClassify();
};

class TestUtilKeyString : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilKeymap : public CxxTest::TestSuite {
 public:
    void testKeymap();
    void testChange();
};

class TestUtilKeymapTable : public CxxTest::TestSuite {
 public:
    void testKeymapTable();
};

class TestUtilMath : public CxxTest::TestSuite {
 public:
    void testDivideAndRound();
    void testDivideAndRoundToEven();
    void testGetHeading();
    void testSquareInteger();
    void testRound();
    void testDistance();
};

class TestUtilMessageCollector : public CxxTest::TestSuite {
 public:
    void testForward();
    void testBackward();
    void testWrap();
};

class TestUtilMessageMatcher : public CxxTest::TestSuite {
 public:
    void testErrors();
    void testMatch();
};

class TestUtilMessageNotifier : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilPrefixArgument : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSequences();
    void testCancel();
};

class TestUtilRandomNumberGenerator : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRange();
    void testFullRange();
    void testReset();
};

class TestUtilRequest : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRequestDispatcher : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRequestReceiver : public CxxTest::TestSuite {
 public:
    void testIt();
    void testDie();
};

class TestUtilRequestThread : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRunLengthExpandTransform : public CxxTest::TestSuite {
 public:
    void testIt();
    void testBad();
};

class TestUtilSkinColor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSlaveObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSlaveRequest : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSlaveRequestSender : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCall();
};

class TestUtilString : public CxxTest::TestSuite {
 public:
    void testStringMatch();
    void testParseRange();
    void testParsePlayer();
    void testFormatOptions();
    void testEncodeMimeHeader();
};

class TestUtilStringList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSort();
    void testCopy();
};

class TestUtilStringParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilUnicodeChars : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilstring : public CxxTest::TestSuite {
 public:
    void testFormatName();
};

#endif
