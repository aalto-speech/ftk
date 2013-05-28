#include <map>
#include <string>
#include <vector>
#include <cmath>

#include "defs.hh"
#include "msfgtest.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;


CPPUNIT_TEST_SUITE_REGISTRATION (msfgtest);

void msfgtest :: setUp (void)
{
    start_end.assign("*");
}

void msfgtest :: tearDown (void)
{
}

void msfgtest :: MultiStringFactorGraphTest1 (void)
{
    MultiStringFactorGraph msfg(start_end);

    map<string, flt_type> vocab;
    vocab["k"] = 0.0;
    vocab["i"] = 0.0;
    vocab["a"] = 0.0;
    vocab["sa"] = 0.0;
    vocab["s"] = 0.0;
    vocab["ki"] = 0.0;
    vocab["kis"] = 0.0;
    vocab["kissa"] = 0.0;
    vocab["lle"] = 0.0;
    vocab["kin"] = 0.0;
    vocab["kala"] = 0.0;

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    string sentence2("kissallekin");
    FactorGraph fg2(sentence2, start_end, vocab, 5);
    string sentence3("kissakala");
    FactorGraph fg3(sentence3, start_end, vocab, 5);
    msfg.add(fg);
    CPPUNIT_ASSERT_EQUAL( 1, (int)msfg.string_end_nodes.size() );
    CPPUNIT_ASSERT_EQUAL( 7, (int)msfg.num_paths(sentence) );
    msfg.add(fg2);
    CPPUNIT_ASSERT_EQUAL( 2, (int)msfg.string_end_nodes.size() );
    CPPUNIT_ASSERT_EQUAL( 7, (int)msfg.num_paths(sentence2) );
    msfg.add(fg3);
    CPPUNIT_ASSERT_EQUAL( 3, (int)msfg.string_end_nodes.size() );
    CPPUNIT_ASSERT_EQUAL( 7, (int)msfg.num_paths(sentence3) );
}

void msfgtest :: MultiStringFactorGraphTest2 (void)
{
    CPPUNIT_ASSERT (false);
}

// No possible segmentation
void msfgtest :: MultiStringFactorGraphTest3 (void)
{
    CPPUNIT_ASSERT (false);
}
