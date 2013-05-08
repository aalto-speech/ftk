#ifndef FACTOR_ENCODER_CPPUNIT
#define FACTOR_ENCODER_CPPUNIT

#include "FactorEncoder.hh"

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
    CPPUNIT_TEST (viterbiTest7);
    CPPUNIT_TEST (ForwardBackwardTest1);
    CPPUNIT_TEST (ForwardBackwardTest2);
    CPPUNIT_TEST (ForwardBackwardTest3);
    CPPUNIT_TEST (ForwardBackwardTest4);
    CPPUNIT_TEST (ForwardBackwardTest5);
    CPPUNIT_TEST (ForwardBackwardTest6);
    CPPUNIT_TEST (ForwardBackwardTest7);
    CPPUNIT_TEST (ForwardBackwardTest8);
    CPPUNIT_TEST (FactorGraphTest1);
    CPPUNIT_TEST (FactorGraphTest2);
    CPPUNIT_TEST (FactorGraphTest3);
    CPPUNIT_TEST (FactorGraphTest4);
    CPPUNIT_TEST (FactorGraphTestGetList);
    CPPUNIT_TEST (TransitionViterbiTest1);
    CPPUNIT_TEST (TransitionViterbiTest2);
    CPPUNIT_TEST (TransitionViterbiTest3);
    CPPUNIT_TEST (TransitionViterbiTest4);
    CPPUNIT_TEST (TransitionViterbiTest5);
    CPPUNIT_TEST (TransitionViterbiTest6);
    CPPUNIT_TEST (TransitionViterbiTest7);
    CPPUNIT_TEST (TransitionViterbiTest8);
    CPPUNIT_TEST (TransitionForwardBackwardTest1);
    CPPUNIT_TEST (TransitionForwardBackwardTest2);
    CPPUNIT_TEST (TransitionForwardBackwardTest3);
    CPPUNIT_TEST (TransitionForwardBackwardTest4);
    CPPUNIT_TEST (TransitionForwardBackwardTest5);
    CPPUNIT_TEST (TransitionForwardBackwardTest6);
    CPPUNIT_TEST (TransitionForwardBackwardTest7);
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
        void viterbiTest7(void);
        void ForwardBackwardTest1(void);
        void ForwardBackwardTest2(void);
        void ForwardBackwardTest3(void);
        void ForwardBackwardTest4(void);
        void ForwardBackwardTest5(void);
        void ForwardBackwardTest6(void);
        void ForwardBackwardTest7(void);
        void ForwardBackwardTest8(void);
        void FactorGraphTest1(void);
        void FactorGraphTest2(void);
        void FactorGraphTest3(void);
        void FactorGraphTest4(void);
        void FactorGraphTestGetList(void);
        void TransitionViterbiTest1(void);
        void TransitionViterbiTest2(void);
        void TransitionViterbiTest3(void);
        void TransitionViterbiTest4(void);
        void TransitionViterbiTest5(void);
        void TransitionViterbiTest6(void);
        void TransitionViterbiTest7(void);
        void TransitionViterbiTest8(void);
        void TransitionForwardBackwardTest1(void);
        void TransitionForwardBackwardTest2(void);
        void TransitionForwardBackwardTest3(void);
        void TransitionForwardBackwardTest4(void);
        void TransitionForwardBackwardTest5(void);
        void TransitionForwardBackwardTest6(void);
        void TransitionForwardBackwardTest7(void);
};

#endif
