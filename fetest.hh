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
    CPPUNIT_TEST (viterbiTest4);
    CPPUNIT_TEST (viterbiTest5);
    CPPUNIT_TEST (viterbiTest6);
    CPPUNIT_TEST (ForwardBackwardTest1);
    CPPUNIT_TEST (ForwardBackwardTest2);
    CPPUNIT_TEST (ForwardBackwardTest3);
    CPPUNIT_TEST (ForwardBackwardTest4);
    CPPUNIT_TEST (ForwardBackwardTest5);
    CPPUNIT_TEST (ForwardBackwardTest6);
    CPPUNIT_TEST_SUITE_END ();

    public:
        void setUp(void);
        void tearDown(void);

    protected:
        void viterbiTest1(void);
        void viterbiTest2(void);
        void viterbiTest3(void);
        void viterbiTest4(void);
        void viterbiTest5(void);
        void viterbiTest6(void);
        void ForwardBackwardTest1(void);
        void ForwardBackwardTest2(void);
        void ForwardBackwardTest3(void);
        void ForwardBackwardTest4(void);
        void ForwardBackwardTest5(void);
        void ForwardBackwardTest6(void);
};

#endif
