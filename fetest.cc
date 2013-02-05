#include "fetest.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION (fetest);

void fetest :: setUp (void)
{
}

void fetest :: tearDown (void)
{
}

void fetest :: viterbiTest1 (void)
{
    std::map<std::string, double> vocab;
    std::string str1("a"); std::string str2("bc");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    std::string sentence("abc");
    std::vector<std::string> best_path;
    int maxlen = 0;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL (maxlen, 2);
}
