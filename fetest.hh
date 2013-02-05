#ifndef FACTOR_ENCODER_CPPUNIT
#define FACTOR_ENCODER_CPPUNIT

#include "factorencoder.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class fetest : public CPPUNIT_NS :: TestFixture
{
    CPPUNIT_TEST_SUITE (fetest);
    CPPUNIT_TEST (viterbiTest1);
    CPPUNIT_TEST (viterbiTest2);
    CPPUNIT_TEST (viterbiTest3);
    CPPUNIT_TEST_SUITE_END ();

    public:
        void setUp(void);
        void tearDown(void);

    protected:
        void viterbiTest1(void);
        void viterbiTest2(void);
        void viterbiTest3(void);
};

#endif
