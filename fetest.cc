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
    int maxlen = 2;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str1, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str2, best_path[1]);
}

void fetest :: viterbiTest2 (void)
{
    std::map<std::string, double> vocab;
    std::string str1("a"); std::string str2("bc");
    std::string str3("ab"); std::string str4("c");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    std::string sentence("abc");
    std::vector<std::string> best_path;
    int maxlen = 2;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str3, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str4, best_path[1]);
}

// No possible segmentation
void fetest :: viterbiTest3 (void)
{
    std::map<std::string, double> vocab;
    std::string str1("a");
    vocab[str1] = -1.0;
    std::string sentence("abc");
    std::vector<std::string> best_path;
    int maxlen = 1;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// Empty string
void fetest :: viterbiTest4 (void)
{
    std::map<std::string, double> vocab;
    std::string str1("a");
    vocab[str1] = -1.0;
    std::string sentence("");
    std::vector<std::string> best_path;
    int maxlen = 1;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// One character sentence
void fetest :: viterbiTest5 (void)
{
    std::map<std::string, double> vocab;
    std::string str1("a");
    vocab[str1] = -1.0;
    std::string sentence("a");
    std::vector<std::string> best_path;
    int maxlen = 1;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(1, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str1, best_path[0]);
}
