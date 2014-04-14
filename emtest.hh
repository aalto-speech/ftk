#ifndef EM_CPPUNIT
#define EM_CPPUNIT

#include "EM.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class emtest : public CPPUNIT_NS :: TestFixture
{
    CPPUNIT_TEST_SUITE (emtest);
    CPPUNIT_TEST (viterbiTest1);
    CPPUNIT_TEST (viterbiTest2);
    CPPUNIT_TEST (viterbiTest3);
    CPPUNIT_TEST (viterbiTest4);
    CPPUNIT_TEST (viterbiTest5);
    CPPUNIT_TEST (viterbiTest6);
    CPPUNIT_TEST (viterbiTest7);
    CPPUNIT_TEST (viterbiTest8);
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
    CPPUNIT_TEST (FactorGraphTestNumPaths);
    CPPUNIT_TEST (FactorGraphTestGetList);
    CPPUNIT_TEST (FactorGraphTestRemoveArcs);
    CPPUNIT_TEST (FactorGraphTestRemoveArcs2);
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
    CPPUNIT_TEST (TransitionForwardBackwardTest8);
    CPPUNIT_TEST (TransitionForwardBackwardRemoveArcs);
    CPPUNIT_TEST (TransitionForwardBackwardBlockFactor);
    CPPUNIT_TEST_SUITE_END ();

    public:
        void setUp(void);
        void tearDown(void);

    protected:
        void viterbiChecks(const std::map<std::string, flt_type> &vocab,
                           int maxlen,
                           std::string &sentence,
                           std::vector<std::string> &correct_path,
                           flt_type correct_lp,
                           bool utf8=false);
        void viterbiTest1(void);
        void viterbiTest2(void);
        void viterbiTest3(void);
        void viterbiTest4(void);
        void viterbiTest5(void);
        void viterbiTest6(void);
        void viterbiTest7(void);
        void viterbiTest8(void);
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
        void FactorGraphTestNumPaths(void);
        void FactorGraphTestGetList(void);
        void FactorGraphTestRemoveArcs(void);
        void FactorGraphTestRemoveArcs2(void);
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
        void TransitionForwardBackwardTest8(void);
        void TransitionForwardBackwardRemoveArcs(void);
        void TransitionForwardBackwardBlockFactor(void);
};

#endif
