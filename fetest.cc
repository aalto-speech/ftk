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

string start_end("*");


// Testing constructor
// only forward possible routes in the graph
void fetest :: FactorGraphTest1 (void)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("hal", 0.0));
    vocab.insert(make_pair("halojaa", 0.0));
    vocab.insert(make_pair("ojaa", 0.0));
    vocab.insert(make_pair("jaa", 0.0));

    FactorGraph fg("halojaa", start_end, vocab, 7);
    CPPUNIT_ASSERT_EQUAL(5, (int)fg.nodes.size());

    StringSet<flt_type> ssvocab(vocab);
    FactorGraph ssfg("halojaa", start_end, ssvocab);
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

    FactorGraph fg("halojaa", start_end, vocab, 7);
    CPPUNIT_ASSERT_EQUAL(5, (int)fg.nodes.size());

    StringSet<flt_type> ssvocab(vocab);
    FactorGraph ssfg("halojaa", start_end, ssvocab);
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

    FactorGraph fg("halojaa", start_end, vocab, 3);
    CPPUNIT_ASSERT_EQUAL(0, (int)fg.nodes.size());

    StringSet<flt_type> ssvocab(vocab);
    FactorGraph ssfg("halojaa", start_end, ssvocab);
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

    FactorGraph fg("kaupungistuminen", start_end, vocab, 6);
    CPPUNIT_ASSERT_EQUAL(23, (int)fg.nodes.size());

    StringSet<flt_type> ssvocab(vocab);
    FactorGraph ssfg("kaupungistuminen", start_end, ssvocab);
    CPPUNIT_ASSERT(fg.assert_equal(ssfg));

    int node_idx = 0;
    assert_node(fg, node_idx++, start_end, 0, 3);
    assert_node(fg, node_idx++, std::string("k"), 1, 1);
    assert_node(fg, node_idx++, std::string("kau"), 1, 1);
    assert_node(fg, node_idx++, std::string("kaupun"), 1, 3);
    assert_node(fg, node_idx++, std::string("a"), 1, 1);
    assert_node(fg, node_idx++, std::string("u"), 1, 1);
    assert_node(fg, node_idx++, std::string("p"), 2, 1);
    assert_node(fg, node_idx++, std::string("u"), 1, 1);
    assert_node(fg, node_idx++, std::string("n"), 1, 3);
    assert_node(fg, node_idx++, std::string("g"), 2, 1);
    assert_node(fg, node_idx++, std::string("gis"), 2, 1);
    assert_node(fg, node_idx++, std::string("gistu"), 2, 2);
    assert_node(fg, node_idx++, std::string("i"), 1, 1);
    assert_node(fg, node_idx++, std::string("s"), 1, 1);
    assert_node(fg, node_idx++, std::string("t"), 2, 1);
    assert_node(fg, node_idx++, std::string("u"), 1, 2);
    assert_node(fg, node_idx++, std::string("m"), 2, 1);
    assert_node(fg, node_idx++, std::string("minen"), 2, 1);
    assert_node(fg, node_idx++, std::string("i"), 1, 1);
    assert_node(fg, node_idx++, std::string("n"), 1, 1);
    assert_node(fg, node_idx++, std::string("e"), 1, 1);
    assert_node(fg, node_idx++, std::string("n"), 1, 1);
    assert_node(fg, node_idx++, start_end, 2, 0);
}


bool includes_path(vector<vector<string> > &all_paths, vector<string> &path) {
    for (int i=0; i<all_paths.size(); i++) {
        if (path.size() != all_paths[i].size()) continue;
        int j=0;
        for (; j<path.size(); j++)
            if (all_paths[i][j] != path[j]) break;
        if (j==path.size()) return true;
    }
    return false;
}

// Enumerating paths
void fetest :: FactorGraphTestGetList (void)
{
    map<string, flt_type> vocab;
    vocab["k"] = 0.0;
    vocab["i"] = 0.0;
    vocab["s"] = 0.0;
    vocab["a"] = 0.0;
    vocab["sa"] = 0.0;
    vocab["ki"] = 0.0;
    vocab["kis"] = 0.0;
    vocab["kissa"] = 0.0;

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    vector<vector<string> > paths;
    fg.get_paths(paths);
    vector<string> seg = {"*", "kissa", "*" };
    CPPUNIT_ASSERT( includes_path(paths, seg) );
    vector<string> seg2 = {"*", "kis", "sa", "*" };
    CPPUNIT_ASSERT( includes_path(paths, seg2) );
    vector<string> seg3 = {"*", "kis", "s", "a", "*" };
    CPPUNIT_ASSERT( includes_path(paths, seg3) );
    vector<string> seg4 = {"*", "ki", "s", "s", "a", "*" };
    CPPUNIT_ASSERT( includes_path(paths, seg4) );
    vector<string> seg5 = {"*", "ki", "s", "sa", "*" };
    CPPUNIT_ASSERT( includes_path(paths, seg5) );
    vector<string> seg6 = {"*", "ki", "s", "s", "a", "*" };
    CPPUNIT_ASSERT( includes_path(paths, seg6) );
    vector<string> seg7 = {"*", "k", "i", "s", "s", "a", "*" };
    CPPUNIT_ASSERT( includes_path(paths, seg7) );
}


void fetest :: TransitionViterbiTest1 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("a"); string str2("bc");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    vocab[str2] = -1.0;
    transitions[make_pair(str1, str2)] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str2, start_end)] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 2);
    vector<string> best_path;
    viterbi(transitions, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(4, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str1, best_path[1]);
    CPPUNIT_ASSERT_EQUAL(str2, best_path[2]);
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[3]);
}

void fetest :: TransitionViterbiTest2 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
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
    FactorGraph fg(sentence, start_end, vocab, 2);
    vector<string> best_path;
    viterbi(transitions, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(4, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str3, best_path[1]);
    CPPUNIT_ASSERT_EQUAL(str4, best_path[2]);
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[3]);
}

// No possible segmentation
void fetest :: TransitionViterbiTest3 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 1);
    vector<string> best_path;
    viterbi(transitions, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// Empty string
void fetest :: TransitionViterbiTest4 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("");
    FactorGraph fg(sentence, start_end, vocab, 1);
    vector<string> best_path;
    viterbi(transitions, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// One character sentence
void fetest :: TransitionViterbiTest5 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("a");
    FactorGraph fg(sentence, start_end, vocab, 1);
    vector<string> best_path;
    viterbi(transitions, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(3, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str1, best_path[1]);
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[2]);
}

// No segmentation
void fetest :: TransitionViterbiTest6 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
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
    FactorGraph fg(sentence, start_end, vocab, 1);
    vector<string> best_path;
    viterbi(transitions, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(0, (int)best_path.size());
}

// Normal scenario with few variations
void fetest :: TransitionViterbiTest7 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("k"); string str2("i");
    string str3("s"); string str4("a");
    string str5("l"); string str6("kissa");
    string str7("lla"); string str8("kissalla");
    vocab[start_end] = 0.0;
    vocab[str1] = 0.0;
    vocab[str2] = 0.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    vocab[str5] = 0.0;
    vocab[str6] = 0.0;
    vocab[str7] = 0.0;
    vocab[str8] = 0.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(start_end, str6)] = -1.0;
    transitions[make_pair(start_end, str8)] = -6.0;
    transitions[make_pair(str4, start_end)] = -1.0;
    transitions[make_pair(str7, start_end)] = -1.0;
    transitions[make_pair(str8, start_end)] = -6.0;
    transitions[make_pair(str1, str2)] = -1.0;
    transitions[make_pair(str2, str3)] = -1.0;
    transitions[make_pair(str3, str3)] = -1.0;
    transitions[make_pair(str3, str4)] = -1.0;
    transitions[make_pair(str4, str5)] = -1.0;
    transitions[make_pair(str4, str7)] = -1.0;
    transitions[make_pair(str5, str4)] = -1.0;
    transitions[make_pair(str5, str5)] = -1.0;
    transitions[make_pair(str6, str7)] = -1.0;
    transitions[make_pair(str6, str5)] = -1.0;
    string sentence("kissalla");
    int maxlen = 8;
    FactorGraph fg(sentence, start_end, vocab, maxlen);
    vector<string> best_path;

    viterbi(transitions, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(4, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str6, best_path[1]);
    CPPUNIT_ASSERT_EQUAL(str7, best_path[2]);
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[3]);

    transitions[make_pair(str6, str7)] = -10.0;
    viterbi(transitions, fg, best_path);
    CPPUNIT_ASSERT_EQUAL(6, (int)best_path.size());
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[0]);
    CPPUNIT_ASSERT_EQUAL(str6, best_path[1]);
    CPPUNIT_ASSERT_EQUAL(str5, best_path[2]);
    CPPUNIT_ASSERT_EQUAL(str5, best_path[3]);
    CPPUNIT_ASSERT_EQUAL(str4, best_path[4]);
    CPPUNIT_ASSERT_EQUAL(start_end, best_path[5]);
}

// Check that non-existing transitions throw for now
void fetest :: TransitionViterbiTest8 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("k"); string str2("i");
    string str3("s"); string str4("a");
    string str5("l"); string str6("kissa");
    string str7("lla"); string str8("kissalla");
    vocab[start_end] = 0.0;
    vocab[str1] = 0.0;
    vocab[str2] = 0.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    vocab[str5] = 0.0;
    vocab[str6] = 0.0;
    vocab[str7] = 0.0;
    vocab[str8] = 0.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(start_end, str6)] = -1.0;
    transitions[make_pair(start_end, str8)] = -6.0;
    transitions[make_pair(str4, start_end)] = -1.0;
    transitions[make_pair(str7, start_end)] = -1.0;
    transitions[make_pair(str8, start_end)] = -6.0;
    transitions[make_pair(str1, str2)] = -1.0;
    transitions[make_pair(str2, str3)] = -1.0;
    transitions[make_pair(str3, str4)] = -1.0;
    transitions[make_pair(str4, str5)] = -1.0;
    transitions[make_pair(str4, str7)] = -1.0;
    transitions[make_pair(str5, str4)] = -1.0;
    transitions[make_pair(str6, str7)] = -1.0;
    transitions[make_pair(str6, str5)] = -1.0;
    string sentence("kissalla");
    int maxlen = 8;
    FactorGraph fg(sentence, start_end, vocab, maxlen);
    vector<string> best_path;

    try {
        viterbi(transitions, fg, best_path);
    }
    catch (exception& e)
    {
        return;
    }
    CPPUNIT_ASSERT(false);
}


void fetest :: TransitionForwardBackwardTest1 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("a"); string str2("bc");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    vocab[str2] = -1.0;
    transitions[make_pair(str1, str2)] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str2, start_end)] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 2);
    map<pair<string,string>, flt_type> stats;
    forward_backward(transitions, fg, stats);
    CPPUNIT_ASSERT_EQUAL(3, (int)stats.size());
    CPPUNIT_ASSERT_EQUAL(1.0, stats[make_pair(start_end, str1)]);
    CPPUNIT_ASSERT_EQUAL(1.0, stats[make_pair(str1, str2)]);
    CPPUNIT_ASSERT_EQUAL(1.0, stats[make_pair(str2, start_end)]);
}

void fetest :: TransitionForwardBackwardTest2 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
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
    FactorGraph fg(sentence, start_end, vocab, 2);
    map<pair<string,string>, flt_type> stats;
    forward_backward(transitions, fg, stats);
    flt_type path_1_score = exp(-4)/(exp(-3) + exp(-4));
    flt_type path_2_score = exp(-3)/(exp(-3) + exp(-4));
    CPPUNIT_ASSERT_EQUAL(6, (int)stats.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL( path_1_score, stats[make_pair(str2, start_end)], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( path_2_score, stats[make_pair(str4, start_end)], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( path_1_score, stats[make_pair(start_end, str1)], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( path_2_score, stats[make_pair(start_end, str3)], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( path_1_score, stats[make_pair(str1, str2)], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( path_2_score, stats[make_pair(str3, str4)], DBL_ACCURACY );
}


// No possible segmentation
void fetest :: TransitionForwardBackwardTest3 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 1);
    map<pair<string,string>, flt_type> stats;
    forward_backward(transitions, fg, stats);
    CPPUNIT_ASSERT_EQUAL(0, (int)stats.size());
}

// Empty string
void fetest :: TransitionForwardBackwardTest4 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("");
    FactorGraph fg(sentence, start_end, vocab, 1);
    map<pair<string,string>, flt_type> stats;
    forward_backward(transitions, fg, stats);
    CPPUNIT_ASSERT_EQUAL(0, (int)stats.size());
}

// One character sentence
void fetest :: TransitionForwardBackwardTest5 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    string str1("a");
    vocab[start_end] = -1.0;
    vocab[str1] = -1.0;
    transitions[make_pair(start_end, str1)] = -1.0;
    transitions[make_pair(str1, start_end)] = -1.0;
    string sentence("a");
    FactorGraph fg(sentence, start_end, vocab, 1);
    map<pair<string,string>, flt_type> stats;
    forward_backward(transitions, fg, stats);
    CPPUNIT_ASSERT_EQUAL(2, (int)stats.size());
    CPPUNIT_ASSERT_EQUAL( 1.0, stats[make_pair(start_end, str1)] );
    CPPUNIT_ASSERT_EQUAL( 1.0, stats[make_pair(str1, start_end)] );
}

// No segmentation
void fetest :: TransitionForwardBackwardTest6 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
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
    FactorGraph fg(sentence, start_end, vocab, 1);
    map<pair<string,string>, flt_type> stats;
    forward_backward(transitions, fg, stats);
    CPPUNIT_ASSERT_EQUAL(0, (int)stats.size());
}

// Multiple paths
void fetest :: TransitionForwardBackwardTest7 (void)
{
    map<string, flt_type> vocab;
    map<pair<string,string>, flt_type> transitions;
    vocab["k"] = 0.0;
    vocab["i"] = 0.0;
    vocab["a"] = 0.0;
    vocab["sa"] = 0.0;
    vocab["s"] = 0.0;
    vocab["ki"] = 0.0;
    vocab["kis"] = 0.0;
    vocab["kissa"] = 0.0;
    transitions[make_pair(start_end, "k")] = log(0.5);
    transitions[make_pair(start_end, "ki")] = log(0.25);
    transitions[make_pair(start_end, "kis")] = log(0.4);
    transitions[make_pair(start_end, "kissa")] = log(0.1);
    transitions[make_pair("a", start_end)] = log(0.5);
    transitions[make_pair("kissa", start_end)] = log(0.10);
    transitions[make_pair("sa", start_end)] = log(0.4);
    transitions[make_pair("ki", "s")] = log(0.25);
    transitions[make_pair("k", "i")] = log(0.5);
    transitions[make_pair("i", "s")] = log(0.5);
    transitions[make_pair("s", "s")] = log(0.5);
    transitions[make_pair("s", "sa")] = log(0.5);
    transitions[make_pair("s", "a")] = log(0.5);
    transitions[make_pair("kis", "sa")] = log(0.4);
    transitions[make_pair("kis", "s")] = log(0.4);

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    map<pair<string,string>, flt_type> stats;
    forward_backward(transitions, fg, stats);
    CPPUNIT_ASSERT_EQUAL(15, (int)stats.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.3626295105394784, stats[make_pair("a", start_end)], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.05716327259735619, stats[make_pair("kissa", start_end)], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.05716327259735619, stats[make_pair(start_end, "kissa")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.5802072168631653, stats[make_pair("sa", start_end)], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.3626295105394784, stats[make_pair("s", "a")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.21436227224008575, stats[make_pair("s", "sa")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.3658449446230797, stats[make_pair("kis", "sa")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.22865309038942486, stats[make_pair("kis", "s")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.5944980350125045, stats[make_pair(start_end, "kis")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.13397642015005362, stats[make_pair("s", "s")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.23222579492675957, stats[make_pair("i", "s")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.23222579492675957, stats[make_pair("k", "i")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.23222579492675957, stats[make_pair(start_end, "k")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.11611289746337979, stats[make_pair("ki", "s")], DBL_ACCURACY );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.11611289746337979, stats[make_pair(start_end, "ki")], DBL_ACCURACY );
}

