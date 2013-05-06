#include <map>
#include <string>
#include <vector>
#include <cmath>

#include "defs.hh"
#include "fetest.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

double DBL_ACCURACY = 0.0001;

CPPUNIT_TEST_SUITE_REGISTRATION (fetest);

void fetest :: setUp (void)
{
}

void fetest :: tearDown (void)
{
}

void fetest :: viterbiTest1 (void)
{
    map<string, flt_type> vocab;
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
    map<string, flt_type> vocab;
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
    map<string, flt_type> vocab;
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
    map<string, flt_type> vocab;
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
    map<string, flt_type> vocab;
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
    map<string, flt_type> vocab;
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

void fetest :: viterbiTest7 (void)
{
    map<string, flt_type> vocab;
    string str1("k"); string str2("i");
    string str3("s"); string str4("a");
    string str5("l"); string str6("kissa");
    string str7("lla"); string str8("kissalla");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = -1.5;
    vocab[str4] = -2.0;
    vocab[str5] = -3.0;
    vocab[str6] = -3.0;
    vocab[str7] = -2.0;
    vocab[str8] = -1.0;
    string sentence("kissalla");
    vector<string> best_path;
    int maxlen = 8;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(1, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str8, best_path[0]);
    vocab[str8] = -6.0;
    viterbi(vocab, maxlen, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str6, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str7, best_path[1]);
}


// Empty string
void fetest :: ForwardBackwardTest1 (void)
{
    map<string, flt_type> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("");
    map<string, flt_type> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(0, (int)stats.size());
}

// No segmentation
void fetest :: ForwardBackwardTest2 (void)
{
    map<string, flt_type> vocab;
    string str1("a"); string str2("b");
    string str3("c"); string str4("d");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    string sentence("a-bcd");
    map<string, flt_type> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(0, (int)stats.size());
}

// One character string
void fetest :: ForwardBackwardTest3 (void)
{
    map<string, flt_type> vocab;
    vocab["a"] = -1.0;
    string sentence("a");
    map<string, flt_type> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(1, (int)stats.size());
    CPPUNIT_ASSERT_EQUAL((flt_type)1.0, stats["a"]);
}

// Two character string, one segmentation
void fetest :: ForwardBackwardTest4 (void)
{
    map<string, flt_type> vocab;
    vocab["a"] = -1.0;
    vocab["b"] = -1.0;
    string sentence("ab");
    map<string, flt_type> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(2, (int)stats.size());
    CPPUNIT_ASSERT_EQUAL((flt_type)1.0, stats["b"]);
    CPPUNIT_ASSERT_EQUAL((flt_type)1.0, stats["a"]);
}

// Two character string, two segmentations
// Independent paths
void fetest :: ForwardBackwardTest5 (void)
{
    map<string, flt_type> vocab;
    vocab["a"] = -1.0;
    vocab["b"] = -1.0;
    vocab["ab"] = -2.0;
    string sentence("ab");
    map<string, flt_type> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(3, (int)stats.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (flt_type)0.50, stats["ab"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (flt_type)0.50, stats["b"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (flt_type)0.50, stats["a"], DBL_ACCURACY );
}

// Three character string, two segmentations
// Dependent paths
void fetest :: ForwardBackwardTest6 (void)
{
    map<string, flt_type> vocab;
    vocab["a"] = -1.0;
    vocab["b"] = -1.0;
    vocab["c"] = -1.0;
    vocab["d"] = -1.0;
    vocab["bc"] = -2.0;
    string sentence("abc");
    map<string, flt_type> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(4, (int)stats.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (flt_type)0.50, stats["bc"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (flt_type)0.50, stats["c"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (flt_type)0.50, stats["b"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (flt_type)1.0, stats["a"], DBL_ACCURACY );
}

// Multiple paths
void fetest :: ForwardBackwardTest7 (void)
{
    map<string, flt_type> vocab;
    vocab["a"] = log(0.25);
    vocab["sa"] = log(0.25);
    vocab["s"] = log(0.25);
    vocab["ki"] = log(0.50);
    vocab["kis"] = log(0.50);
    string sentence("kissa");
    map<string, flt_type> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(5, (int)stats.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.80, stats["kis"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.20, stats["ki"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.80, stats["sa"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.20+0.20, stats["s"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.20, stats["a"], DBL_ACCURACY );
}

// Multiple paths
void fetest :: ForwardBackwardTest8 (void)
{
    map<string, flt_type> vocab;
    vocab["a"] = log(0.25);
    vocab["sa"] = log(0.25);
    vocab["s"] = log(0.25);
    vocab["ki"] = log(0.50);
    vocab["kis"] = log(0.50);
    vocab["kissa"] = log(0.1953125);
    string sentence("kissa");
    map<string, flt_type> stats;
    forward_backward(vocab, sentence, stats);
    CPPUNIT_ASSERT_EQUAL(6, (int)stats.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.5, stats["kissa"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.80/2.0, stats["kis"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.20/2.0, stats["ki"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.8/2.0, stats["sa"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (0.2+0.2)/2.0, stats["s"], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.2/2.0, stats["a"], DBL_ACCURACY );
}

void fetest :: TransitionViterbiTest1 (void)
{
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a"); string str2("bc");
    transitions[make_pair(str1, str2)] = -1.0;
    transitions[make_pair(start_end, str2)] = -1.0;
    transitions[make_pair(str2, start_end)] = -1.0;
    string sentence("abc");
    vector<string> best_path;
    int maxlen = 2;
    viterbi(transitions, maxlen, start_end, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str1, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str2, best_path[1]);
}

void fetest :: TransitionViterbiTest2 (void)
{
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a"); string str2("bc");
    string str3("ab"); string str4("c");
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(start_end, str3)] = -1.0;
    transitions[make_pair(str2, start_end)] = -1.0;
    transitions[make_pair(str4, start_end)] = -1.0;
    transitions[make_pair(str1, str2)] = -2.0;
    transitions[make_pair(str3, str4)] = -1.0;
    string sentence("abc");
    vector<string> best_path;
    int maxlen = 2;
    viterbi(transitions, maxlen, start_end, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str3, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str4, best_path[1]);
}

// No possible segmentation
void fetest :: TransitionViterbiTest3 (void)
{
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a");
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("abc");
    vector<string> best_path;
    int maxlen = 1;
    viterbi(transitions, maxlen, start_end, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// Empty string
void fetest :: TransitionViterbiTest4 (void)
{
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a");
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("");
    vector<string> best_path;
    int maxlen = 1;
    viterbi(transitions, maxlen, start_end, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// One character sentence
void fetest :: TransitionViterbiTest5 (void)
{
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a");
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("a");
    vector<string> best_path;
    int maxlen = 1;
    viterbi(transitions, maxlen, start_end, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(1, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str1, best_path[0]);
}

// No segmentation
void fetest :: TransitionViterbiTest6 (void)
{
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a"); string str2("b");
    string str3("c"); string str4("d");
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str4, start_end)] = -1.0;
    transitions[make_pair(str1, str2)] = -1.0;
    transitions[make_pair(str2, str3)] = -1.0;
    transitions[make_pair(str3, str4)] = -1.0;
    string sentence("a-bcd");
    vector<string> best_path;
    int maxlen = 1;
    viterbi(transitions, maxlen, start_end, sentence, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// Testing constructor
// only forward possible routes in the graph
void fetest :: FactorGraphTest1 (void)
{
    string str1("hal");
    string str2("halojaa");
    string str3("ojaa");
    string str4("jaa");

    map<std::string, flt_type> vocab;
    vocab[str1] = 0.0;
    vocab[str2] = 0.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;

    FactorGraph fg("halojaa", vocab, 7);
    CPPUNIT_ASSERT_EQUAL(3, (int)fg.nodes.size());
}

// Testing constructor
// pruning out impossible paths, simple case
void fetest :: FactorGraphTest2 (void)
{
    string str1("hal");
    string str2("halojaa");
    string str3("ojaa");
    string str4("oj");

    map<std::string, flt_type> vocab;
    vocab[str1] = 0.0;
    vocab[str2] = 0.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;

    FactorGraph fg("halojaa", vocab, 7);
    CPPUNIT_ASSERT_EQUAL(3, (int)fg.nodes.size());
}

// Testing constructor
// No possible segmentations
void fetest :: FactorGraphTest3 (void)
{
    string str1("hal");
    string str2("oja");
    string str3("oj");

    map<std::string, flt_type> vocab;
    vocab[str1] = 0.0;
    vocab[str2] = 0.0;
    vocab[str3] = 0.0;

    FactorGraph fg("halojaa", vocab, 3);
    CPPUNIT_ASSERT_EQUAL(0, (int)fg.nodes.size());
}


