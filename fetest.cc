#include <map>
#include <string>
#include <vector>

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
    map<string, double> vocab;
    string str1("a"); string str2("bc");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    string sentence("abc");
    vector<string> best_path;
    int maxlen = 2;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str1, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str2, best_path[1]);
}

void fetest :: viterbiTest2 (void)
{
    map<string, double> vocab;
    string str1("a"); string str2("bc");
    string str3("ab"); string str4("c");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    string sentence("abc");
    vector<string> best_path;
    int maxlen = 2;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str3, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str4, best_path[1]);
}

// No possible segmentation
void fetest :: viterbiTest3 (void)
{
    map<string, double> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("abc");
    vector<string> best_path;
    int maxlen = 1;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// Empty string
void fetest :: viterbiTest4 (void)
{
    map<string, double> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("");
    vector<string> best_path;
    int maxlen = 1;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// One character sentence
void fetest :: viterbiTest5 (void)
{
    map<string, double> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("a");
    vector<string> best_path;
    int maxlen = 1;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(1, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str1, best_path[0]);
}

// No segmentation
void fetest :: viterbiTest6 (void)
{
    map<string, double> vocab;
    string str1("a"); string str2("b");
    string str3("c"); string str4("d");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    string sentence("a-bcd");
    vector<string> best_path;
    int maxlen = 1;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// Empty string
void fetest :: ForwardBackwardTest1 (void)
{
    map<string, double> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("");
    map<string, double> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(0, (int)stats.size());
}

// No segmentation
void fetest :: ForwardBackwardTest2 (void)
{
    map<string, double> vocab;
    string str1("a"); string str2("b");
    string str3("c"); string str4("d");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    string sentence("a-bcd");
    map<string, double> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(0, (int)stats.size());
}

// One character string
void fetest :: ForwardBackwardTest3 (void)
{
    map<string, double> vocab;
    vocab["a"] = -1.0;
    string sentence("a");
    map<string, double> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(1, (int)stats.size());
    CPPUNIT_ASSERT_EQUAL(1.0, stats["a"]);
}

// Two character string, one segmentation
void fetest :: ForwardBackwardTest4 (void)
{
    map<string, double> vocab;
    vocab["a"] = -1.0;
    vocab["b"] = -1.0;
    string sentence("ab");
    map<string, double> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(2, (int)stats.size());
    CPPUNIT_ASSERT_EQUAL(1.0, stats["b"]);
    CPPUNIT_ASSERT_EQUAL(1.0, stats["a"]);
}

// Two character string, two segmentations
// Independent paths
void fetest :: ForwardBackwardTest5 (void)
{
    map<string, double> vocab;
    vocab["a"] = -1.0;
    vocab["b"] = -1.0;
    vocab["ab"] = -1.0;
    string sentence("ab");
    map<string, double> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(3, (int)stats.size());
    CPPUNIT_ASSERT_EQUAL(0.25, stats["b"]);
    CPPUNIT_ASSERT_EQUAL(0.25, stats["a"]);
    CPPUNIT_ASSERT_EQUAL(0.75, stats["ab"]);
}

