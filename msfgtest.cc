
#include "msfgtest.hh"


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
    set<string> vocab = {"k", "i", "s", "a", "sa", "ki", "kis", "kissa",
                         "lle", "kin", "kala"};
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
    CPPUNIT_ASSERT_EQUAL( 7, msfg.num_paths(sentence2) );
    msfg.add(fg3);
    CPPUNIT_ASSERT_EQUAL( 3, (int)msfg.string_end_nodes.size() );
    CPPUNIT_ASSERT_EQUAL( 7, (int)msfg.num_paths(sentence3) );
}

void msfgtest :: MultiStringFactorGraphTest2 (void)
{
    MultiStringFactorGraph msfg(start_end);
    set<string> vocab = {"k", "i", "s", "a", "sa", "ki", "kis", "kissa",
                         "lle", "kin", "kala"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    string sentence2("kissallekin");
    FactorGraph fg2(sentence2, start_end, vocab, 5);
    string sentence3("kissakala");
    FactorGraph fg3(sentence3, start_end, vocab, 5);
    msfg.add(fg, false);
    CPPUNIT_ASSERT_EQUAL( 1, (int)msfg.string_end_nodes.size() );
    CPPUNIT_ASSERT_EQUAL( 7, (int)msfg.num_paths(sentence) );
    msfg.add(fg2, false);
    CPPUNIT_ASSERT_EQUAL( 2, (int)msfg.string_end_nodes.size() );
    CPPUNIT_ASSERT_EQUAL( 7, msfg.num_paths(sentence2) );
    msfg.add(fg3, false);
    CPPUNIT_ASSERT_EQUAL( 3, (int)msfg.string_end_nodes.size() );
    CPPUNIT_ASSERT_EQUAL( 7, (int)msfg.num_paths(sentence3) );
}

void msfgtest :: MultiStringFactorGraphTest3 (void)
{
    MultiStringFactorGraph msfg(start_end);
    set<string> vocab = {"k", "i", "s", "a", "sa", "ki", "la", "kis", "kissa",
                         "lle", "kin", "kala"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    string sentence2("kala");
    FactorGraph fg2(sentence2, start_end, vocab, 5);
    string sentence3("kissakala");
    FactorGraph fg3(sentence3, start_end, vocab, 5);
    msfg.add(fg);
    msfg.add(fg2);
    msfg.add(fg3);
}

void msfgtest :: MultiStringFactorGraphTest4 (void)
{
    MultiStringFactorGraph msfg(start_end);
    set<string> vocab = {"aarian", "aari", "an", "a", "ari", "ri", "ar", "i", "n", "r"};
    string word("aarian");
    FactorGraph fg(word, start_end, vocab, 6);
    msfg.add(fg);
    vector<vector<string> > paths;
    msfg.get_paths(word, paths);
    CPPUNIT_ASSERT_EQUAL( 11, fg.num_paths() );
    CPPUNIT_ASSERT_EQUAL( 11, (int)paths.size() );
    CPPUNIT_ASSERT_EQUAL( 11, msfg.num_paths(word) );
}
