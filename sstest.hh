#ifndef STRINGSET_CPPUNIT
#define STRINGSET_CPPUNIT

#include "StringSet.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class sstest : public CPPUNIT_NS :: TestFixture
{
    CPPUNIT_TEST_SUITE (sstest);
    CPPUNIT_TEST (StringSetTest1);
    CPPUNIT_TEST (StringSetTest2);
    CPPUNIT_TEST (StringSetTest3);
    CPPUNIT_TEST (StringSetTest4);
    CPPUNIT_TEST (StringSetTest5);
    CPPUNIT_TEST_SUITE_END ();

    public:
        void setUp(void);
        void tearDown(void);

    protected:
        void StringSetTest1(void);
        void StringSetTest2(void);
        void StringSetTest3(void);
        void StringSetTest4(void);
        void StringSetTest5(void);

    private:
        std::string start_end;
};

#endif
