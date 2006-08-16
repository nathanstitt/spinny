#ifndef UNITTEST_TESTRUNNER_H
#define UNITTEST_TESTRUNNER_H


namespace UnitTest {

class TestReporter;
class TestList;


int RunAllTests( const char *suiteName = 0 );
int RunAllTests(TestReporter& reporter, TestList const& list, char const* suiteName, int maxTestTimeInMs = 0);

}


#endif
