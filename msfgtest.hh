#ifndef MSFG_CPPUNIT
#define MSFG_CPPUNIT

#include "MSFG.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class msfgtest : public CPPUNIT_NS :: TestFixture
{
    CPPUNIT_TEST_SUITE (msfgtest);
    CPPUNIT_TEST (MultiStringFactorGraphTest1);
    CPPUNIT_TEST (MultiStringFactorGraphTest2);
    CPPUNIT_TEST (MultiStringFactorGraphTest3);
    CPPUNIT_TEST (MultiStringFactorGraphTest4);
    CPPUNIT_TEST_SUITE_END ();

    public:
        void setUp(void);
        void tearDown(void);

    protected:
        void MultiStringFactorGraphTest1(void);
        void MultiStringFactorGraphTest2(void);
        void MultiStringFactorGraphTest3(void);
        void MultiStringFactorGraphTest4(void);

    private:
        std::string start_end;
};

#endif
