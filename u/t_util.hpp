/**
  *  \file u/t_util.hpp
  *  \brief Tests for util
  */
#ifndef C2NG_U_T_UTIL_HPP
#define C2NG_U_T_UTIL_HPP

#include <cxxtest/TestSuite.h>

class TestUtilApplication : public CxxTest::TestSuite {
 public:
    void testInit();
    void testExit();
    void testWriteError();
};

class TestUtilAtomTable : public CxxTest::TestSuite {
 public:
    void testAtom();
    void testManyAtoms();
};

class TestUtilBackupFile : public CxxTest::TestSuite {
 public:
    void testExpand();
    void testFileOperations();
    void testFileOperationsEmpty();
};

class TestUtilCharsetFactory : public CxxTest::TestSuite {
 public:
    void testIteration();
    void testNames();
    void testCodes();
    void testErrors();
};

class TestUtilConfigurationFile : public CxxTest::TestSuite {
 public:
    void testLoad();
    void testSave();
    void testFind();
    void testMergePreserve();
    void testMergeNamespaced();
    void testRemove();
    void testAdd();
    void testSet();
};

class TestUtilConfigurationFileParser : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUtilConsoleLogger : public CxxTest::TestSuite {
 public:
    void testDefault();
    void testConfig();
};

class TestUtilDataTable : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testNames();
    void testIteration();
    void testStack();
    void testAppend();
    void testAdd();
    void testSort();
};

class TestUtilDigest : public CxxTest::TestSuite {
 public:
    void testIt();
    void testStaticInstance();
    void testDynamicType();
};

class TestUtilDirectoryBrowser : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCreateDirectory();
    void testSelect();
    void testWildcard();
    void testRoot();
};

class TestUtilExpressionList : public CxxTest::TestSuite {
 public:
    void testAccess();
    void testLRU();
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
    void testIt();
    void testCharset();
    void testTrimComments();
    void testParseOptional();
};

class TestUtilHelpIndex : public CxxTest::TestSuite {
 public:
    void testMulti();
    void testMissing();
};

class TestUtilInstructionList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testReadInsnOnly();
    void testAppend();
};

class TestUtilIo : public CxxTest::TestSuite {
 public:
    void testStorePascalString();
    void testStorePascalStringTruncate();
    void testLoadPascalString();
    void testAppendExt();
    void testCreateDirectoryTree();
    void testGetFileNameExtension();
    void testMakeSearchDirectory();
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
    void testDescribe();
    void testDescribeMI();
};

class TestUtilKeymapInformation : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilKeymapTable : public CxxTest::TestSuite {
 public:
    void testKeymapTable();
};

class TestUtilLayout : public CxxTest::TestSuite {
 public:
    void testLabelPos();
};

class TestUtilMath : public CxxTest::TestSuite {
 public:
    void testDivideAndRound();
    void testDivideAndRoundUp();
    void testDivideAndRoundToEven();
    void testGetHeading();
    void testSquareInteger();
    void testRound();
    void testDistance();
    void testSquareFloat();
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

class TestUtilNumberFormatter : public CxxTest::TestSuite {
 public:
    void testFormat();
};

class TestUtilPrefixArgument : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSequences();
    void testCancel();
};

class TestUtilProcessRunner : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilProfileDirectory : public CxxTest::TestSuite {
 public:
    void testOpen();
    void testOpenDir();
};

class TestUtilRandomNumberGenerator : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRange();
    void testFullRange();
    void testReset();
};

class TestUtilRange : public CxxTest::TestSuite {
 public:
    void testInit();
    void testInclude();
    void testIntersect();
    void testOp();
    void testFormat();
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

class TestUtilRequestSender : public CxxTest::TestSuite {
 public:
    void testConvert();
    void testMakeTemporary();
    void testPostRequest();
    void testConvertFail();
    void testMakeTemporaryFail();
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

class TestUtilSimpleRequestDispatcher : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSkinColor : public CxxTest::TestSuite {
 public:
    void testIt();
    void testParse();
};

class TestUtilStopSignal : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilString : public CxxTest::TestSuite {
 public:
    void testStringMatch();
    void testParseRange();
    void testParsePlayer();
    void testFormatOptions();
    void testEncodeMimeHeader();
    void testParseBoolean();
    void testFormatName();
    void testEncodeHtml();
    void testTrailing();
    void testCollate();
    void testFormatAge();
    void testStrStartsWith();
    void testParseZoomLevel();
    void testFormatZoomLevel();
};

class TestUtilStringInstructionList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testReadWrong();
    void testSwap();
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

class TestUtilSystemInformation : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilTranslation : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilTreeList : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testAddPath();
    void testSwap();
};

class TestUtilUnicodeChars : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilUpdater : public CxxTest::TestSuite {
 public:
    void testTrue();
    void testFalse();
};

class TestUtilVector : public CxxTest::TestSuite {
 public:
    void testIt();
    void test1Based();
};

#endif
