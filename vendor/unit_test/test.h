#ifndef UNITTESTCPP_H
#define UNITTESTCPP_H

#include "Config.h"
#include "Test.h"
#include "TestList.h"
#include "TestSuite.h"
#include "TestResults.h"
#include "TestMacros.h"
#include "CheckMacros.h"
#include "TestRunner.h"

#include "TestDetails.h"

namespace UnitTest {

class TestResults;
class TestList;

class Test
{
public:
    Test(char const* testName, char const* suiteName = "DefaultSuite", char const* filename = "", int lineNumber = 0);
    virtual ~Test();
    void Run(TestResults& testResults) const;

    TestDetails const m_details;
    Test* next;
    mutable bool m_timeConstraintExempt;

    static TestList& GetTestList();

private:
    virtual void RunImpl(TestResults& testResults_) const;

    Test(Test const&);
    Test& operator =(Test const&);
};


}

#endif
