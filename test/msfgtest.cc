#include <boost/test/unit_test.hpp>

#include "defs.hh"
#include "MSFG.hh"

using namespace std;


BOOST_AUTO_TEST_CASE(MultiStringFactorGraphTest1)
{
    MultiStringFactorGraph msfg(start_end_symbol);
    set<string> vocab = {"k", "i", "s", "a", "sa", "ki", "kis", "kissa",
                         "lle", "kin", "kala"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end_symbol, vocab, 5);
    string sentence2("kissallekin");
    FactorGraph fg2(sentence2, start_end_symbol, vocab, 5);
    string sentence3("kissakala");
    FactorGraph fg3(sentence3, start_end_symbol, vocab, 5);
    msfg.add(fg);
    BOOST_CHECK_EQUAL( 1, (int)msfg.string_end_nodes.size() );
    BOOST_CHECK_EQUAL( 7, (int)msfg.num_paths(sentence) );
    msfg.add(fg2);
    BOOST_CHECK_EQUAL( 2, (int)msfg.string_end_nodes.size() );
    BOOST_CHECK_EQUAL( 7, msfg.num_paths(sentence2) );
    msfg.add(fg3);
    BOOST_CHECK_EQUAL( 3, (int)msfg.string_end_nodes.size() );
    BOOST_CHECK_EQUAL( 7, (int)msfg.num_paths(sentence3) );
}

BOOST_AUTO_TEST_CASE(MultiStringFactorGraphTest2)
{
    MultiStringFactorGraph msfg(start_end_symbol);
    set<string> vocab = {"k", "i", "s", "a", "sa", "ki", "kis", "kissa",
                         "lle", "kin", "kala"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end_symbol, vocab, 5);
    string sentence2("kissallekin");
    FactorGraph fg2(sentence2, start_end_symbol, vocab, 5);
    string sentence3("kissakala");
    FactorGraph fg3(sentence3, start_end_symbol, vocab, 5);
    msfg.add(fg, false);
    BOOST_CHECK_EQUAL( 1, (int)msfg.string_end_nodes.size() );
    BOOST_CHECK_EQUAL( 7, (int)msfg.num_paths(sentence) );
    msfg.add(fg2, false);
    BOOST_CHECK_EQUAL( 2, (int)msfg.string_end_nodes.size() );
    BOOST_CHECK_EQUAL( 7, msfg.num_paths(sentence2) );
    msfg.add(fg3, false);
    BOOST_CHECK_EQUAL( 3, (int)msfg.string_end_nodes.size() );
    BOOST_CHECK_EQUAL( 7, (int)msfg.num_paths(sentence3) );
}

BOOST_AUTO_TEST_CASE(MultiStringFactorGraphTest3)
{
    MultiStringFactorGraph msfg(start_end_symbol);
    set<string> vocab = {"k", "i", "s", "a", "sa", "ki", "la", "kis", "kissa",
                         "lle", "kin", "kala"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end_symbol, vocab, 5);
    string sentence2("kala");
    FactorGraph fg2(sentence2, start_end_symbol, vocab, 5);
    string sentence3("kissakala");
    FactorGraph fg3(sentence3, start_end_symbol, vocab, 5);
    msfg.add(fg);
    msfg.add(fg2);
    msfg.add(fg3);
}

BOOST_AUTO_TEST_CASE(MultiStringFactorGraphTest4)
{
    MultiStringFactorGraph msfg(start_end_symbol);
    set<string> vocab = {"aarian", "aari", "an", "a", "ari", "ri", "ar", "i", "n", "r"};
    string word("aarian");
    FactorGraph fg(word, start_end_symbol, vocab, 6);
    msfg.add(fg);
    vector<vector<string> > paths;
    msfg.get_paths(word, paths);
    BOOST_CHECK_EQUAL( 11, fg.num_paths() );
    BOOST_CHECK_EQUAL( 11, (int)paths.size() );
    BOOST_CHECK_EQUAL( 11, msfg.num_paths(word) );
}
