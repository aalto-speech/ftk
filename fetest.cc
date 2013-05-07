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
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a"); string str2("bc");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    vocab[str2] = -1.0;
    transitions[make_pair(str1, str2)] = -1.0;
    transitions[make_pair(start_end, str2)] = -1.0;
    transitions[make_pair(str2, start_end)] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, vocab, 2);
    vector<string> best_path;
    viterbi(transitions, start_end, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str1, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str2, best_path[1]);
}

void fetest :: TransitionViterbiTest2 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a"); string str2("bc");
    string str3("ab"); string str4("c");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    vocab[str2] = -1.0;
    vocab[str3] = -1.0;
    vocab[str4] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(start_end, str3)] = -1.0;
    transitions[make_pair(str2, start_end)] = -1.0;
    transitions[make_pair(str4, start_end)] = -1.0;
    transitions[make_pair(str1, str2)] = -2.0;
    transitions[make_pair(str3, str4)] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, vocab, 2);
    vector<string> best_path;
    viterbi(transitions, start_end, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(2, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str3, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str4, best_path[1]);
}

// No possible segmentation
void fetest :: TransitionViterbiTest3 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, vocab, 1);
    vector<string> best_path;
    viterbi(transitions, start_end, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// Empty string
void fetest :: TransitionViterbiTest4 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("");
    FactorGraph fg(sentence, vocab, 1);
    vector<string> best_path;
    viterbi(transitions, start_end, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// One character sentence
void fetest :: TransitionViterbiTest5 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("a");
    FactorGraph fg(sentence, vocab, 1);
    vector<string> best_path;
    viterbi(transitions, start_end, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(1, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(str1, best_path[0]);
}

// No segmentation
void fetest :: TransitionViterbiTest6 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string start_end("_");
    string str1("a"); string str2("b");
    string str3("c"); string str4("d");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    vocab[str2] = -1.0;
    vocab[str3] = -1.0;
    vocab[str4] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str4, start_end)] = -1.0;
    transitions[make_pair(str1, str2)] = -1.0;
    transitions[make_pair(str2, str3)] = -1.0;
    transitions[make_pair(str3, str4)] = -1.0;
    string sentence("a-bcd");
    FactorGraph fg(sentence, vocab, 4);
    vector<string> best_path;
    viterbi(transitions, start_end, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}


void assert_node(const FactorGraph &fg,
                 int node,
                 const std::string &nstr,
                 unsigned int incoming_sz,
                 unsigned int outgoing_sz)
{
    std::string tst;
    fg.get_string(fg.nodes[node], tst);
    CPPUNIT_ASSERT_EQUAL( nstr, tst );
    CPPUNIT_ASSERT_EQUAL( incoming_sz, (unsigned int)fg.nodes[node].incoming.size() );
    CPPUNIT_ASSERT_EQUAL( outgoing_sz, (unsigned int)fg.nodes[node].outgoing.size() );
}


// Testing constructor
// only forward possible routes in the graph
void fetest :: FactorGraphTest1 (void)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("hal", 0.0));
    vocab.insert(make_pair("halojaa", 0.0));
    vocab.insert(make_pair("ojaa", 0.0));
    vocab.insert(make_pair("jaa", 0.0));

    FactorGraph fg("halojaa", vocab, 7);
    CPPUNIT_ASSERT_EQUAL(3, (int)fg.nodes.size());

    StringSet<flt_type> ssvocab(vocab);
    FactorGraph ssfg("halojaa", ssvocab);
    CPPUNIT_ASSERT(fg.assert_equal(ssfg));
}

// Testing constructor
// pruning out impossible paths, simple case
void fetest :: FactorGraphTest2 (void)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("hal", 0.0));
    vocab.insert(make_pair("halojaa", 0.0));
    vocab.insert(make_pair("ojaa", 0.0));
    vocab.insert(make_pair("oj", 0.0));

    FactorGraph fg("halojaa", vocab, 7);
    CPPUNIT_ASSERT_EQUAL(3, (int)fg.nodes.size());

    StringSet<flt_type> ssvocab(vocab);
    FactorGraph ssfg("halojaa", ssvocab);
    CPPUNIT_ASSERT(fg.assert_equal(ssfg));
}

// Testing constructor
// No possible segmentations
void fetest :: FactorGraphTest3 (void)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("hal", 0.0));
    vocab.insert(make_pair("oja", 0.0));
    vocab.insert(make_pair("oj", 0.0));

    FactorGraph fg("halojaa", vocab, 3);
    CPPUNIT_ASSERT_EQUAL(0, (int)fg.nodes.size());

    StringSet<flt_type> ssvocab(vocab);
    FactorGraph ssfg("halojaa", ssvocab);
    CPPUNIT_ASSERT(fg.assert_equal(ssfg));
}

// Testing constructor
// Normal case
void fetest :: FactorGraphTest4 (void)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("k", 0.0));
    vocab.insert(make_pair("a", 0.0));
    vocab.insert(make_pair("u", 0.0));
    vocab.insert(make_pair("p", 0.0));
    vocab.insert(make_pair("n", 0.0));
    vocab.insert(make_pair("g", 0.0));
    vocab.insert(make_pair("i", 0.0));
    vocab.insert(make_pair("s", 0.0));
    vocab.insert(make_pair("t", 0.0));
    vocab.insert(make_pair("m", 0.0));
    vocab.insert(make_pair("e", 0.0));
    vocab.insert(make_pair("kaupun", 0.0));
    vocab.insert(make_pair("gis", 0.0));
    vocab.insert(make_pair("gistu", 0.0));
    vocab.insert(make_pair("minen", 0.0));
    vocab.insert(make_pair("kau", 0.0));

    FactorGraph fg("kaupungistuminen", vocab, 6);
    CPPUNIT_ASSERT_EQUAL(21, (int)fg.nodes.size());

    StringSet<flt_type> ssvocab(vocab);
    FactorGraph ssfg("kaupungistuminen", ssvocab);
    CPPUNIT_ASSERT(fg.assert_equal(ssfg));

    assert_node(fg, 0, std::string("k"), 1, 1);
    assert_node(fg, 1, std::string("kau"), 1, 1);
    assert_node(fg, 2, std::string("kaupun"), 1, 3);
    assert_node(fg, 3, std::string("a"), 1, 1);
    assert_node(fg, 4, std::string("u"), 1, 1);
    assert_node(fg, 5, std::string("p"), 2, 1);
    assert_node(fg, 6, std::string("u"), 1, 1);
    assert_node(fg, 7, std::string("n"), 1, 3);
    assert_node(fg, 8, std::string("g"), 2, 1);
    assert_node(fg, 9, std::string("gis"), 2, 1);
    assert_node(fg, 10, std::string("gistu"), 2, 2);
    assert_node(fg, 11, std::string("i"), 1, 1);
    assert_node(fg, 12, std::string("s"), 1, 1);
    assert_node(fg, 13, std::string("t"), 2, 1);
    assert_node(fg, 14, std::string("u"), 1, 2);
    assert_node(fg, 15, std::string("m"), 2, 1);
    assert_node(fg, 16, std::string("minen"), 2, 0);
    assert_node(fg, 17, std::string("i"), 1, 1);
    assert_node(fg, 18, std::string("n"), 1, 1);
    assert_node(fg, 19, std::string("e"), 1, 1);
    assert_node(fg, 20, std::string("n"), 1, 0);
}

