#include <boost/test/unit_test.hpp>

#include "Bigrams.hh"
#include "EM.hh"


using namespace std;

double DBL_ACCURACY = 0.0001;


void
viterbiChecks(const map<string, flt_type> &vocab,
              int maxlen,
              string &sentence,
              vector<string> &correct_path,
              flt_type correct_lp,
              bool utf8=true)
{
    vector<string> result_path;
    flt_type result_lp = viterbi(vocab, maxlen, sentence, result_path, true, utf8);
    BOOST_CHECK_EQUAL( correct_path.size(), result_path.size() );
    BOOST_CHECK_EQUAL( correct_lp, result_lp );
    for (unsigned int i=0; i<correct_path.size(); i++)
        BOOST_CHECK_EQUAL( correct_path[i], result_path[i] );

    result_path.clear();
    StringSet ssvocab(vocab);
    result_lp = viterbi(ssvocab, sentence, result_path, true, utf8);
    BOOST_CHECK_EQUAL( correct_path.size(), result_path.size() );
    BOOST_CHECK_EQUAL( correct_lp, result_lp );
    for (unsigned int i=0; i<correct_path.size(); i++)
        BOOST_CHECK_EQUAL( correct_path[i], result_path[i] );
}


BOOST_AUTO_TEST_CASE(viterbiTest1)
{
    map<string, flt_type> vocab;
    string str1("a"); string str2("bc");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    string sentence("abc");
    int maxlen = 2;
    vector<string> correct_path;
    correct_path.push_back(str1);
    correct_path.push_back(str2);
    viterbiChecks(vocab, maxlen, sentence, correct_path, -3.0);
}

BOOST_AUTO_TEST_CASE(viterbiTest2)
{
    map<string, flt_type> vocab;
    string str1("a"); string str2("bc");
    string str3("ab"); string str4("c");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    string sentence("abc");
    int maxlen = 2;
    vector<string> correct_path;
    correct_path.push_back(str3);
    correct_path.push_back(str4);
    viterbiChecks(vocab, maxlen, sentence, correct_path, 0.0);
}

// No possible segmentation
BOOST_AUTO_TEST_CASE(viterbiTest3)
{
    map<string, flt_type> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("abc");
    int maxlen = 1;
    vector<string> correct_path;
    viterbiChecks(vocab, maxlen, sentence, correct_path, MIN_FLOAT);
}

// Empty string
BOOST_AUTO_TEST_CASE(viterbiTest4)
{
    map<string, flt_type> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("");
    int maxlen = 1;
    vector<string> correct_path;
    viterbiChecks(vocab, maxlen, sentence, correct_path, MIN_FLOAT);
}

// One character sentence
BOOST_AUTO_TEST_CASE(viterbiTest5)
{
    map<string, flt_type> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("a");
    int maxlen = 1;
    vector<string> correct_path;
    correct_path.push_back(str1);
    viterbiChecks(vocab, maxlen, sentence, correct_path, -1.0);
}

// No segmentation
BOOST_AUTO_TEST_CASE(viterbiTest6)
{
    map<string, flt_type> vocab;
    string str1("a"); string str2("b");
    string str3("c"); string str4("d");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = 0.0;
    vocab[str4] = 0.0;
    string sentence("a-bcd");
    int maxlen = 1;
    vector<string> correct_path;
    viterbiChecks(vocab, maxlen, sentence, correct_path, MIN_FLOAT);
}

BOOST_AUTO_TEST_CASE(viterbiTest7)
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
    int maxlen = 8;
    vector<string> correct_path;
    correct_path.push_back(str8);
    viterbiChecks(vocab, maxlen, sentence, correct_path, -1);
    correct_path.clear();
    correct_path.push_back(str6);
    correct_path.push_back(str7);
    vocab[str8] = -6.0;
    viterbiChecks(vocab, maxlen, sentence, correct_path, -5.0);
}


// UTF-8
BOOST_AUTO_TEST_CASE(viterbiTest8)
{
    map<string, flt_type> vocab;
    string str1("p"); string str2("ö");
    string str3("l"); string str4("i");
    string str5("pölli");
    string str6("llä"); string str7("pöllillä");
    string str8("�");
    string str9("�");
    string str10("�");
    vocab[str1] = -1.0;
    vocab[str2] = -2.0;
    vocab[str3] = -1.5;
    vocab[str4] = -2.0;
    vocab[str5] = -3.0;
    vocab[str6] = -2.0;
    vocab[str7] = -1.0;
    string sentence("pöllillä");
    int maxlen = 8;
    vector<string> correct_path;
    correct_path.push_back(str7);
    viterbiChecks(vocab, maxlen, sentence, correct_path, -1, true);
    correct_path.clear();
    correct_path.push_back(str5);
    correct_path.push_back(str6);
    vocab[str7] = -6.0;
    viterbiChecks(vocab, maxlen, sentence, correct_path, -5.0, true);
    vocab[str1] = -0.1;
    vocab[str3] = -0.1;
    vocab[str4] = -0.1;
    vocab[str8] = -0.1;
    vocab[str9] = -0.1;
    vocab[str10] = -0.1;
    vocab.erase(str2);
    viterbiChecks(vocab, maxlen, sentence, correct_path, -5.0, true);
}


// Empty string
BOOST_AUTO_TEST_CASE(ForwardBackwardTest1)
{
    map<string, flt_type> vocab;
    string str1("a");
    vocab[str1] = -1.0;
    string sentence("");
    map<string, flt_type> stats;
    flt_type lp = forward_backward(vocab, sentence, stats);
    BOOST_CHECK_EQUAL(0, (int)stats.size());
    BOOST_CHECK_EQUAL(MIN_FLOAT, lp);
}

// No segmentation
BOOST_AUTO_TEST_CASE(ForwardBackwardTest2)
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
    flt_type lp = forward_backward(vocab, sentence, stats);
    BOOST_CHECK_EQUAL(0, (int)stats.size());
    BOOST_CHECK_EQUAL(MIN_FLOAT, lp);
}

// One character string
BOOST_AUTO_TEST_CASE(ForwardBackwardTest3)
{
    map<string, flt_type> vocab;
    vocab["a"] = -1.0;
    string sentence("a");
    map<string, flt_type> stats;
    flt_type lp = forward_backward(vocab, sentence, stats);
    BOOST_CHECK_EQUAL(1, (int)stats.size());
    BOOST_CHECK_EQUAL((flt_type)1.0, stats["a"]);
    BOOST_CHECK_EQUAL(-1.0, lp);
}

// Two character string, one segmentation
BOOST_AUTO_TEST_CASE(ForwardBackwardTest4)
{
    map<string, flt_type> vocab;
    vocab["a"] = -1.0;
    vocab["b"] = -1.0;
    string sentence("ab");
    map<string, flt_type> stats;
    flt_type lp = forward_backward(vocab, sentence, stats);
    BOOST_CHECK_EQUAL(2, (int)stats.size());
    BOOST_CHECK_EQUAL((flt_type)1.0, stats["b"]);
    BOOST_CHECK_EQUAL((flt_type)1.0, stats["a"]);
    BOOST_CHECK_EQUAL(-2.0, lp);
}

// Two character string, two segmentations
// Independent paths
BOOST_AUTO_TEST_CASE(ForwardBackwardTest5)
{
    map<string, flt_type> vocab;
    vocab["a"] = -1.0;
    vocab["b"] = -1.0;
    vocab["ab"] = -2.0;
    string sentence("ab");
    map<string, flt_type> stats;
    flt_type lp = forward_backward(vocab, sentence, stats);
    BOOST_CHECK_EQUAL(3, (int)stats.size());
    BOOST_CHECK_CLOSE( (flt_type)0.50, stats["ab"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (flt_type)0.50, stats["b"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (flt_type)0.50, stats["a"], DBL_ACCURACY );
    // -2.0 + math.log(1+math.exp(-2.0-(-2.0)))
    BOOST_CHECK_CLOSE( (flt_type)-1.3068528194400546, lp, DBL_ACCURACY );
}

// Three character string, two segmentations
// Dependent paths
BOOST_AUTO_TEST_CASE(ForwardBackwardTest6)
{
    map<string, flt_type> vocab;
    vocab["a"] = -1.0;
    vocab["b"] = -1.0;
    vocab["c"] = -1.0;
    vocab["d"] = -1.0;
    vocab["bc"] = -2.0;
    string sentence("abc");
    map<string, flt_type> stats;
    flt_type lp = forward_backward(vocab, sentence, stats);
    BOOST_CHECK_EQUAL(4, (int)stats.size());
    BOOST_CHECK_CLOSE( (flt_type)0.50, stats["bc"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (flt_type)0.50, stats["c"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (flt_type)0.50, stats["b"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (flt_type)1.0, stats["a"], DBL_ACCURACY );
    // -3.0 + math.log(1+math.exp(-3.0-(-3.0)))
    BOOST_CHECK_CLOSE( (flt_type)-2.3068528194400546, lp, DBL_ACCURACY);
}

// Multiple paths
BOOST_AUTO_TEST_CASE(ForwardBackwardTest7)
{
    map<string, flt_type> vocab;
    vocab["a"] = log(0.25);
    vocab["sa"] = log(0.25);
    vocab["s"] = log(0.25);
    vocab["ki"] = log(0.50);
    vocab["kis"] = log(0.50);
    string sentence("kissa");
    map<string, flt_type> stats;
    flt_type lp = forward_backward(vocab, sentence, stats);
    BOOST_CHECK_EQUAL(5, (int)stats.size());
    BOOST_CHECK_CLOSE( 0.80, stats["kis"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.20, stats["ki"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.80, stats["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.20+0.20, stats["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.20, stats["a"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (flt_type)-1.63315443905, lp, DBL_ACCURACY );
}

// Multiple paths
BOOST_AUTO_TEST_CASE(ForwardBackwardTest8)
{
    map<string, flt_type> vocab;
    vocab["a"] = log(0.25);
    vocab["sa"] = log(0.25);
    vocab["s"] = log(0.25);
    vocab["ki"] = log(0.50);
    vocab["kis"] = log(0.50);
    vocab["kissa"] = log(0.1953125);
    vocab["k"] = log(0.00000001);
    string sentence("kissa");
    map<string, flt_type> stats;
    flt_type lp = forward_backward(vocab, sentence, stats);
    BOOST_CHECK_EQUAL(6, (int)stats.size());
    BOOST_CHECK_CLOSE( 0.5, stats["kissa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.80/2.0, stats["kis"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.20/2.0, stats["ki"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.8/2.0, stats["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (0.2+0.2)/2.0, stats["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.2/2.0, stats["a"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (flt_type)-0.940007258491, lp, DBL_ACCURACY);
}


// UTF-8
BOOST_AUTO_TEST_CASE(ForwardBackwardTest9)
{
    map<string, flt_type> vocab;
    vocab["i"] = log(0.25);
    vocab["si"] = log(0.25);
    vocab["s"] = log(0.25);
    vocab["kö"] = log(0.50);
    vocab["kös"] = log(0.50);
    vocab["kössi"] = log(0.1953125);
    vocab["k"] = log(0.00000000001);
    vocab["�"] = log(0.00000000001);
    vocab["�"] = log(0.00000000001);
    string sentence("kössi");
    map<string, flt_type> stats;
    flt_type lp = forward_backward(vocab, sentence, stats, true);
    BOOST_CHECK_EQUAL(6, (int)stats.size());
    BOOST_CHECK_CLOSE( 0.5, stats["kössi"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.80/2.0, stats["kös"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.20/2.0, stats["kö"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.8/2.0, stats["si"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (0.2+0.2)/2.0, stats["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.2/2.0, stats["i"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( (flt_type)-0.940007258491, lp, DBL_ACCURACY);
}


void assert_node(const FactorGraph &fg,
                 int node,
                 const std::string &nstr,
                 unsigned int incoming_sz,
                 unsigned int outgoing_sz)
{
    std::string tst;
    fg.get_factor(fg.nodes[node], tst);
    BOOST_CHECK_EQUAL( nstr, tst );
    BOOST_CHECK_EQUAL( incoming_sz, (unsigned int)fg.nodes[node].incoming.size() );
    BOOST_CHECK_EQUAL( outgoing_sz, (unsigned int)fg.nodes[node].outgoing.size() );
}

string start_end("*");


// Testing constructor
// only forward possible routes in the graph
BOOST_AUTO_TEST_CASE(FactorGraphTest1)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("hal", 0.0));
    vocab.insert(make_pair("halojaa", 0.0));
    vocab.insert(make_pair("ojaa", 0.0));
    vocab.insert(make_pair("jaa", 0.0));

    FactorGraph fg("halojaa", start_end, vocab, 7);
    BOOST_CHECK_EQUAL(5, (int)fg.nodes.size());

    StringSet ssvocab(vocab);
    FactorGraph ssfg("halojaa", start_end, ssvocab);
    BOOST_CHECK(fg.assert_equal(ssfg));
}

// Testing constructor
// pruning out impossible paths, simple case
BOOST_AUTO_TEST_CASE(FactorGraphTest2)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("hal", 0.0));
    vocab.insert(make_pair("halojaa", 0.0));
    vocab.insert(make_pair("ojaa", 0.0));
    vocab.insert(make_pair("oj", 0.0));

    FactorGraph fg("halojaa", start_end, vocab, 7);
    BOOST_CHECK_EQUAL(5, (int)fg.nodes.size());

    StringSet ssvocab(vocab);
    FactorGraph ssfg("halojaa", start_end, ssvocab);
    BOOST_CHECK(fg.assert_equal(ssfg));
}

// Testing constructor
// No possible segmentations
BOOST_AUTO_TEST_CASE(FactorGraphTest3)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("hal", 0.0));
    vocab.insert(make_pair("oja", 0.0));
    vocab.insert(make_pair("oj", 0.0));

    FactorGraph fg("halojaa", start_end, vocab, 3);
    BOOST_CHECK_EQUAL(0, (int)fg.nodes.size());

    StringSet ssvocab(vocab);
    FactorGraph ssfg("halojaa", start_end, ssvocab);
    BOOST_CHECK(fg.assert_equal(ssfg));
}


// Testing constructor
// Normal case
BOOST_AUTO_TEST_CASE(FactorGraphTest4)
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
    BOOST_CHECK_EQUAL(23, (int)fg.nodes.size());

    StringSet ssvocab(vocab);
    FactorGraph ssfg("kaupungistuminen", start_end, ssvocab);
    BOOST_CHECK(fg.assert_equal(ssfg));

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


// Testing constructor
// Utf-8 encoding
BOOST_AUTO_TEST_CASE(FactorGraphTest5)
{
    map<std::string, flt_type> vocab;
    vocab.insert(make_pair("k", 0.0));
    vocab.insert(make_pair("ö", 0.0));
    vocab.insert(make_pair("kö", 0.0));
    vocab.insert(make_pair("kös", 0.0));
    vocab.insert(make_pair("�", 0.0));
    vocab.insert(make_pair("�", 0.0));
    vocab.insert(make_pair("s", 0.0));
    vocab.insert(make_pair("i", 0.0));
    vocab.insert(make_pair("si", 0.0));
    vocab.insert(make_pair("ssi", 0.0));

    FactorGraph fg("kössi", start_end, vocab, 3, true);
    BOOST_CHECK_EQUAL(11, (int)fg.nodes.size());

    StringSet ssvocab(vocab);
    FactorGraph ssfg("kössi", start_end, ssvocab, true);
    BOOST_CHECK(fg.assert_equal(ssfg));

    int node_idx = 0;
    assert_node(fg, node_idx++, start_end, 0, 3);
    assert_node(fg, node_idx++, std::string("k"), 1, 1);
    assert_node(fg, node_idx++, std::string("kö"), 1, 2);
    assert_node(fg, node_idx++, std::string("kös"), 1, 2);
    assert_node(fg, node_idx++, std::string("ö"), 1, 2);
    assert_node(fg, node_idx++, std::string("s"), 2, 2);
    assert_node(fg, node_idx++, std::string("ssi"), 2, 1);
    assert_node(fg, node_idx++, std::string("s"), 2, 1);
    assert_node(fg, node_idx++, std::string("si"), 2, 1);
    assert_node(fg, node_idx++, std::string("i"), 1, 1);
    assert_node(fg, node_idx++, start_end, 3, 0);
}


bool includes_path(vector<vector<string> > &all_paths, vector<string> &path) {
    for (unsigned int i=0; i<all_paths.size(); i++) {
        if (path.size() != all_paths[i].size()) continue;
        unsigned int j=0;
        for (; j<path.size(); j++)
            if (all_paths[i][j] != path[j]) break;
        if (j==path.size()) return true;
    }
    return false;
}

// Test number of paths
BOOST_AUTO_TEST_CASE(FactorGraphTestNumPaths)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    BOOST_CHECK_EQUAL( 7, fg.num_paths() );
    fg.remove_arcs(string("kissa"));
    BOOST_CHECK_EQUAL( 6, fg.num_paths() );
}

// Enumerating paths
BOOST_AUTO_TEST_CASE(FactorGraphTestGetList)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    vector<vector<string> > paths;
    fg.get_paths(paths);
    BOOST_CHECK_EQUAL( 7, (int)paths.size() );
    vector<string> seg = {"*", "kissa", "*" };
    BOOST_CHECK( includes_path(paths, seg) );
    vector<string> seg2 = {"*", "kis", "sa", "*" };
    BOOST_CHECK( includes_path(paths, seg2) );
    vector<string> seg3 = {"*", "kis", "s", "a", "*" };
    BOOST_CHECK( includes_path(paths, seg3) );
    vector<string> seg4 = {"*", "ki", "s", "s", "a", "*" };
    BOOST_CHECK( includes_path(paths, seg4) );
    vector<string> seg5 = {"*", "ki", "s", "sa", "*" };
    BOOST_CHECK( includes_path(paths, seg5) );
    vector<string> seg6 = {"*", "ki", "s", "s", "a", "*" };
    BOOST_CHECK( includes_path(paths, seg6) );
    vector<string> seg7 = {"*", "k", "i", "s", "s", "a", "*" };
    BOOST_CHECK( includes_path(paths, seg7) );
}

// Enumerating paths after removing some arcs
BOOST_AUTO_TEST_CASE(FactorGraphTestRemoveArcs)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    vector<vector<string> > paths;
    fg.remove_arcs(string("k"), string("i"));
    fg.remove_arcs(string("s"), string("a"));
    fg.get_paths(paths);
    BOOST_CHECK_EQUAL( 3, (int)paths.size() );
    vector<string> seg = {"*", "kissa", "*" };
    BOOST_CHECK( includes_path(paths, seg) );
    vector<string> seg2 = {"*", "kis", "sa", "*" };
    BOOST_CHECK( includes_path(paths, seg2) );
    vector<string> seg3 = {"*", "ki", "s", "sa", "*" };
    BOOST_CHECK( includes_path(paths, seg3) );
}


// Enumerating paths after removing some arcs
BOOST_AUTO_TEST_CASE(FactorGraphTestRemoveArcs2)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};
    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    vector<vector<string> > paths;
    fg.remove_arcs(string("k"));
    fg.remove_arcs(string("a"));
    fg.get_paths(paths);
    BOOST_CHECK_EQUAL( 3, (int)paths.size() );
    vector<string> seg = {"*", "kissa", "*" };
    BOOST_CHECK( includes_path(paths, seg) );
    vector<string> seg2 = {"*", "kis", "sa", "*" };
    BOOST_CHECK( includes_path(paths, seg2) );
    vector<string> seg3 = {"*", "ki", "s", "sa", "*" };
    BOOST_CHECK( includes_path(paths, seg3) );
}


int transition_count(const transitions_t &transitions) {
    int count = 0;
    for (auto srcit = transitions.cbegin(); srcit != transitions.cend(); ++srcit)
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit)
            count++;
    return count;
}


BOOST_AUTO_TEST_CASE(TransitionViterbiTest1)
{
    string str1("a"); string str2("bc");
    set<string> vocab = {start_end,str1,str2};

    transitions_t transitions;
    transitions[str1][str2] = -1.0;
    transitions[start_end][str1] = -1.0;
    transitions[str2][start_end] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 2);
    vector<string> best_path;
    flt_type lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(4, (int)best_path.size());
    BOOST_CHECK_EQUAL(start_end, best_path[0]);
    BOOST_CHECK_EQUAL(str1, best_path[1]);
    BOOST_CHECK_EQUAL(str2, best_path[2]);
    BOOST_CHECK_EQUAL(start_end, best_path[3]);
    BOOST_CHECK_CLOSE( -3.0, lp, DBL_ACCURACY );
}

BOOST_AUTO_TEST_CASE(TransitionViterbiTest2)
{
    string str1("a"); string str2("bc");
    string str3("ab"); string str4("c");
    set<string> vocab = {start_end,str1,str2,str3,str4};

    transitions_t transitions;
    transitions[start_end][str1] = -1.0;
    transitions[start_end][str3] = -1.0;
    transitions[str2][start_end] = -1.0;
    transitions[str4][start_end] = -1.0;
    transitions[str1][str2] = -2.0;
    transitions[str3][str4] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 2);
    vector<string> best_path;
    flt_type lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(4, (int)best_path.size());
    BOOST_CHECK_EQUAL(start_end, best_path[0]);
    BOOST_CHECK_EQUAL(str3, best_path[1]);
    BOOST_CHECK_EQUAL(str4, best_path[2]);
    BOOST_CHECK_EQUAL(start_end, best_path[3]);
    BOOST_CHECK_CLOSE( -3.0, lp, DBL_ACCURACY );
}

// No possible segmentation
BOOST_AUTO_TEST_CASE(TransitionViterbiTest3)
{
    string str1("a");
    set<string> vocab = {start_end,str1};
    transitions_t transitions;
    transitions[start_end][str1] = -1.0;
    transitions[str1][start_end] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 1);
    vector<string> best_path;
    flt_type lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(0, (int)best_path.size());
    BOOST_CHECK_CLOSE( MIN_FLOAT, lp, DBL_ACCURACY );
}

// Empty string
BOOST_AUTO_TEST_CASE(TransitionViterbiTest4)
{
    string str1("a");
    set<string> vocab = {start_end,str1};
    transitions_t transitions;
    transitions[start_end][str1] = -1.0;
    transitions[str1][start_end] = -1.0;
    string sentence("");
    FactorGraph fg(sentence, start_end, vocab, 1);
    vector<string> best_path;
    flt_type lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(0, (int)best_path.size());
    BOOST_CHECK_CLOSE( MIN_FLOAT, lp, DBL_ACCURACY );
}

// One character sentence
BOOST_AUTO_TEST_CASE(TransitionViterbiTest5)
{
    string str1("a");
    set<string> vocab = {start_end,str1};

    transitions_t transitions;
    transitions[start_end][str1] = -1.0;
    transitions[str1][start_end] = -1.0;
    string sentence("a");
    FactorGraph fg(sentence, start_end, vocab, 1);
    vector<string> best_path;
    flt_type lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(3, (int)best_path.size());
    BOOST_CHECK_EQUAL(start_end, best_path[0]);
    BOOST_CHECK_EQUAL(str1, best_path[1]);
    BOOST_CHECK_EQUAL(start_end, best_path[2]);
    BOOST_CHECK_CLOSE( -2.0, lp, DBL_ACCURACY );
}

// No segmentation
BOOST_AUTO_TEST_CASE(TransitionViterbiTest6)
{
    string str1("a"); string str2("b");
    string str3("c"); string str4("d");
    set<string> vocab = {start_end,str1,str2,str3,str4};

    transitions_t transitions;
    transitions[start_end][str1] = -1.0;
    transitions[str4][start_end] = -1.0;
    transitions[str1][str2] = -1.0;
    transitions[str2][str3] = -1.0;
    transitions[str3][str4] = -1.0;
    string sentence("a-bcd");
    FactorGraph fg(sentence, start_end, vocab, 1);
    vector<string> best_path;
    flt_type lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(0, (int)best_path.size());
    BOOST_CHECK_CLOSE( MIN_FLOAT, lp, DBL_ACCURACY );
}

// Normal scenario with few variations
BOOST_AUTO_TEST_CASE(TransitionViterbiTest7)
{
    string str1("k"); string str2("i");
    string str3("s"); string str4("a");
    string str5("l"); string str6("kissa");
    string str7("lla"); string str8("kissalla");
    set<string> vocab = {start_end,str1,str2,str3,str4,str5,str6,str7,str8};

    transitions_t transitions;
    transitions[start_end][str1] = -1.0;
    transitions[start_end][str6] = -1.0;
    transitions[start_end][str8] = -6.0;
    transitions[str4][start_end] = -1.0;
    transitions[str7][start_end] = -1.0;
    transitions[str8][start_end] = -6.0;
    transitions[str1][str2] = -1.0;
    transitions[str2][str3] = -1.0;
    transitions[str3][str3] = -1.0;
    transitions[str3][str4] = -1.0;
    transitions[str4][str5] = -1.0;
    transitions[str4][str7] = -1.0;
    transitions[str5][str4] = -1.0;
    transitions[str5][str5] = -1.0;
    transitions[str6][str7] = -1.0;
    transitions[str6][str5] = -1.0;
    string sentence("kissalla");
    int maxlen = 8;
    FactorGraph fg(sentence, start_end, vocab, maxlen);
    vector<string> best_path;

    flt_type lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(4, (int)best_path.size());
    BOOST_CHECK_EQUAL(start_end, best_path[0]);
    BOOST_CHECK_EQUAL(str6, best_path[1]);
    BOOST_CHECK_EQUAL(str7, best_path[2]);
    BOOST_CHECK_EQUAL(start_end, best_path[3]);
    BOOST_CHECK_CLOSE( -3.0, lp, DBL_ACCURACY );

    transitions[str6][str7] = -10.0;
    lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(6, (int)best_path.size());
    BOOST_CHECK_EQUAL(start_end, best_path[0]);
    BOOST_CHECK_EQUAL(str6, best_path[1]);
    BOOST_CHECK_EQUAL(str5, best_path[2]);
    BOOST_CHECK_EQUAL(str5, best_path[3]);
    BOOST_CHECK_EQUAL(str4, best_path[4]);
    BOOST_CHECK_EQUAL(start_end, best_path[5]);
    BOOST_CHECK_CLOSE( -5.0, lp, DBL_ACCURACY );
}

// Check that non-existing transitions are ok
BOOST_AUTO_TEST_CASE(TransitionViterbiTest8)
{
    string str1("k"); string str2("i");
    string str3("s"); string str4("a");
    string str5("l"); string str6("kissa");
    string str7("lla"); string str8("kissalla");
    set<string> vocab = {start_end,str1,str2,str3,str4,str5,str6,str7,str8};

    transitions_t transitions;
    transitions[start_end][str1] = -1.0;
    transitions[start_end][str6] = -1.0;
    transitions[start_end][str8] = -6.0;
    transitions[str4][start_end] = -1.0;
    transitions[str7][start_end] = -1.0;
    transitions[str8][start_end] = -6.0;
    transitions[str1][str2] = -1.0;
    transitions[str2][str3] = -1.0;
    transitions[str3][str3] = -1.0;
    transitions[str3][str4] = -1.0;
    transitions[str4][str5] = -1.0;
    transitions[str4][str7] = -1.0;
    transitions[str5][str4] = -1.0;
    transitions[str5][str5] = -1.0;
    //transitions[str6][str7] = -1.0;
    transitions[str6][str5] = -1.0;
    string sentence("kissalla");
    int maxlen = 8;
    FactorGraph fg(sentence, start_end, vocab, maxlen);
    vector<string> best_path;

    flt_type lp = viterbi(transitions, fg, best_path);
    BOOST_CHECK_EQUAL(6, (int)best_path.size());
    BOOST_CHECK_EQUAL(start_end, best_path[0]);
    BOOST_CHECK_EQUAL(str6, best_path[1]);
    BOOST_CHECK_EQUAL(str5, best_path[2]);
    BOOST_CHECK_EQUAL(str5, best_path[3]);
    BOOST_CHECK_EQUAL(str4, best_path[4]);
    BOOST_CHECK_EQUAL(start_end, best_path[5]);
    BOOST_CHECK_CLOSE( -5.0, lp, DBL_ACCURACY );
}


BOOST_AUTO_TEST_CASE(TransitionForwardBackwardTest1)
{
    string str1("a"); string str2("bc");
    set<string> vocab = {start_end,str1,str2};
    transitions_t transitions;
    transitions[str1][str2] = -1.0;
    transitions[start_end][str1] = -1.0;
    transitions[str2][start_end] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 2);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    BOOST_CHECK_EQUAL(3, transition_count(stats));
    BOOST_CHECK_EQUAL(1.0, stats[start_end][str1]);
    BOOST_CHECK_EQUAL(1.0, stats[str1][str2]);
    BOOST_CHECK_EQUAL(1.0, stats[str2][start_end]);
    BOOST_CHECK_CLOSE( -3.0, lp, DBL_ACCURACY );
}

BOOST_AUTO_TEST_CASE(TransitionForwardBackwardTest2)
{
    string str1("a"); string str2("bc");
    string str3("ab"); string str4("c");
    set<string> vocab = {start_end,str1,str2,str3,str4};
    transitions_t transitions;
    transitions[start_end][str1] = -1.0;
    transitions[start_end][str3] = -1.0;
    transitions[str2][start_end] = -1.0;
    transitions[str4][start_end] = -1.0;
    transitions[str1][str2] = -2.0;
    transitions[str3][str4] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 2);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    flt_type path_1_score = exp(-4)/(exp(-3) + exp(-4));
    flt_type path_2_score = exp(-3)/(exp(-3) + exp(-4));
    BOOST_CHECK_EQUAL(6, transition_count(stats));
    BOOST_CHECK_CLOSE( path_1_score, stats[str2][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( path_2_score, stats[str4][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( path_1_score, stats[start_end][str1], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( path_2_score, stats[start_end][str3], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( path_1_score, stats[str1][str2], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( path_2_score, stats[str3][str4], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( -2.68673831248, lp, DBL_ACCURACY );
}


// No possible segmentation
BOOST_AUTO_TEST_CASE(TransitionForwardBackwardTest3)
{
    transitions_t transitions;
    string str1("a");
    set<string> vocab = {start_end,str1};
    transitions[start_end][str1] = -1.0;
    transitions[str1][start_end] = -1.0;
    string sentence("abc");
    FactorGraph fg(sentence, start_end, vocab, 1);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    BOOST_CHECK_EQUAL(0, (int)stats.size());
    BOOST_CHECK_CLOSE( MIN_FLOAT, lp, DBL_ACCURACY );
}

// Empty string
BOOST_AUTO_TEST_CASE(TransitionForwardBackwardTest4)
{
    transitions_t transitions;
    string str1("a");
    set<string> vocab = {start_end,str1};
    transitions[start_end][str1] = -1.0;
    transitions[str1][start_end] = -1.0;
    string sentence("");
    FactorGraph fg(sentence, start_end, vocab, 1);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    BOOST_CHECK_EQUAL(0, (int)stats.size());
    BOOST_CHECK_CLOSE( MIN_FLOAT, lp, DBL_ACCURACY );
}

// One character sentence
BOOST_AUTO_TEST_CASE(TransitionForwardBackwardTest5)
{
    transitions_t transitions;
    string str1("a");
    set<string> vocab = {start_end,str1};
    transitions[start_end][str1] = -1.0;
    transitions[str1][start_end] = -1.0;
    string sentence("a");
    FactorGraph fg(sentence, start_end, vocab, 1);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    BOOST_CHECK_EQUAL(2, (int)stats.size());
    BOOST_CHECK_EQUAL( 1.0, stats[start_end][str1] );
    BOOST_CHECK_EQUAL( 1.0, stats[str1][start_end] );
    BOOST_CHECK_CLOSE( -2.0, lp, DBL_ACCURACY );
}

// No segmentation
BOOST_AUTO_TEST_CASE(TransitionForwardBackwardTest6)
{
    transitions_t transitions;
    string str1("a"); string str2("b");
    string str3("c"); string str4("d");
    set<string> vocab = {start_end,str1,str2,str3,str4};
    transitions[start_end][str1] = -1.0;
    transitions[str4][start_end] = -1.0;
    transitions[str1][str2] = -1.0;
    transitions[str2][str3] = -1.0;
    transitions[str3][str4] = -1.0;
    string sentence("a-bcd");
    FactorGraph fg(sentence, start_end, vocab, 1);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    BOOST_CHECK_EQUAL(0, (int)stats.size());
    BOOST_CHECK_CLOSE( MIN_FLOAT, lp, DBL_ACCURACY );
}

// Multiple paths
BOOST_AUTO_TEST_CASE(TransitionForwardBackwardTest7)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    BOOST_CHECK_EQUAL(15, transition_count(stats));
    BOOST_CHECK_CLOSE( 0.3626295105394784, stats["a"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.05716327259735619, stats["kissa"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.05716327259735619, stats[start_end]["kissa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.5802072168631653, stats["sa"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.3626295105394784, stats["s"]["a"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.21436227224008575, stats["s"]["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.3658449446230797, stats["kis"]["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.22865309038942486, stats["kis"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.5944980350125045, stats[start_end]["kis"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.13397642015005362, stats["s"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.23222579492675957, stats["i"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.23222579492675957, stats["k"]["i"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.23222579492675957, stats[start_end]["k"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.11611289746337979, stats["ki"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.11611289746337979, stats[start_end]["ki"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( -1.74332651171, lp, DBL_ACCURACY );
}

// Multiple paths, some non-scored arcs
BOOST_AUTO_TEST_CASE(TransitionForwardBackwardTest8)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    //transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    //transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    BOOST_CHECK_EQUAL(15, transition_count(stats));
    BOOST_CHECK_SMALL( stats["a"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.11560693641618498, stats["kissa"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.11560693641618498, stats[start_end]["kissa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.8843930635838151, stats["sa"][start_end], DBL_ACCURACY );
    BOOST_CHECK_SMALL( stats["s"]["a"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.14450867052023122, stats["s"]["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.7398843930635839, stats["kis"]["sa"], DBL_ACCURACY );
    BOOST_CHECK_SMALL( stats["kis"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.7398843930635839, stats[start_end]["kis"], DBL_ACCURACY );
    BOOST_CHECK_SMALL( stats["s"]["s"], DBL_ACCURACY );
    BOOST_CHECK_SMALL( stats["i"]["s"], DBL_ACCURACY );
    BOOST_CHECK_SMALL( stats["k"]["i"], DBL_ACCURACY );
    BOOST_CHECK_SMALL( stats[start_end]["k"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.14450867052023122, stats["ki"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.14450867052023122, stats[start_end]["ki"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( -2.44761086504, lp, DBL_ACCURACY );
}


// Multiple paths, remove some arcs
BOOST_AUTO_TEST_CASE(TransitionForwardBackwardRemoveArcs)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    fg.remove_arcs(string("k"), string("i"));
    fg.remove_arcs(string("s"), string("a"));
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);
    BOOST_CHECK_EQUAL(8, transition_count(stats));
    BOOST_CHECK_CLOSE( 0.11560693641618498, stats["kissa"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.11560693641618498, stats[start_end]["kissa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.8843930635838151, stats["sa"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.14450867052023122, stats["s"]["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.7398843930635839, stats["kis"]["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.7398843930635839, stats[start_end]["kis"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.14450867052023122, stats["ki"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.14450867052023122, stats[start_end]["ki"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( -2.44761086504, lp, DBL_ACCURACY );
}


// Multiple paths, block a factor
BOOST_AUTO_TEST_CASE(TransitionForwardBackwardBlockFactor)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats, string("k"));
    BOOST_CHECK_EQUAL(12, transition_count(stats));
    BOOST_CHECK_CLOSE( 0.07445323406235459, stats["kissa"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.07445323406235459, stats[start_end]["kissa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.5695672405770126, stats["sa"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.3559795253606329, stats["a"][start_end], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.09306654257794324, stats["s"]["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.3559795253606329, stats["s"]["a"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.05816658911121452, stats["s"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.47650069799906936, stats["kis"]["sa"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.29781293624941835, stats["kis"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.7743136342484878, stats[start_end]["kis"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.15123313168915775, stats["ki"]["s"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( 0.15123313168915775, stats[start_end]["ki"], DBL_ACCURACY );
    BOOST_CHECK_CLOSE( -2.00758610458, lp, DBL_ACCURACY );
}


// Normal scenario for one word, same data as in TransitionForwardBackwardTest7
BOOST_AUTO_TEST_CASE(MSFGForwardBackwardTest1)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);

    MultiStringFactorGraph msfg(start_end);
    msfg.add(fg);
    msfg.update_factor_node_map();
    assign_scores(transitions, msfg);
    transitions_t msfg_stats;
    flt_type msfg_lp = forward_backward(msfg, sentence, msfg_stats);

    BOOST_CHECK_CLOSE( lp, msfg_lp, DBL_ACCURACY );
    BOOST_CHECK( stats == msfg_stats );
}


// Normal scenario for one word, same data as in TransitionForwardBackwardTest7
// Multiple words in the MSFG
BOOST_AUTO_TEST_CASE(MSFGForwardBackwardTest2)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    transitions["i"]["sa"] = log(0.8);
    transitions["a"]["a"] = log(0.8);
    transitions["ki"]["sa"] = log(0.8);
    transitions["kis"]["a"] = log(0.8);
    transitions["kissa"]["a"] = log(0.8);
    transitions["sa"]["a"] = log(0.8);

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);
    transitions_t stats;
    flt_type lp = forward_backward(transitions, fg, stats);

    FactorGraph fg2("kisa", start_end, vocab, 5);
    FactorGraph fg3("kissaa", start_end, vocab, 5);

    MultiStringFactorGraph msfg(start_end);
    msfg.add(fg);
    msfg.add(fg2);
    msfg.add(fg3);
    msfg.update_factor_node_map();
    assign_scores(transitions, msfg);
    transitions_t msfg_stats;
    flt_type msfg_lp = forward_backward(msfg, sentence, msfg_stats);

    BOOST_CHECK_CLOSE( lp, msfg_lp, DBL_ACCURACY );
    BOOST_CHECK( stats == msfg_stats );
}


// Normal scenario for multiple words
BOOST_AUTO_TEST_CASE(MSFGForwardBackwardTest3)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    transitions["i"]["sa"] = log(0.8);
    transitions["a"]["a"] = log(0.8);
    transitions["ki"]["sa"] = log(0.8);
    transitions["kis"]["a"] = log(0.8);
    transitions["kissa"]["a"] = log(0.8);
    transitions["sa"]["a"] = log(0.8);

    map<string, flt_type> word_freqs = {{"kissa", 1.0}, {"kisa", 2.0}, {"kissaa", 3.0}};
    transitions_t stats;
    MultiStringFactorGraph msfg(start_end);
    flt_type lp = 0.0;
    for (auto wit = word_freqs.begin(); wit != word_freqs.end(); ++wit) {
        FactorGraph fg(wit->first, start_end, vocab, 5);
        transitions_t curr_stats;
        flt_type curr_lp = forward_backward(transitions, fg, curr_stats);
        lp += wit->second * curr_lp;
        Bigrams::update_trans_stats(curr_stats, wit->second, stats);
        msfg.add(fg);
    }
    msfg.update_factor_node_map();
    assign_scores(transitions, msfg);

    transitions_t msfg_stats;
    flt_type msfg_lp = forward_backward(msfg, word_freqs, msfg_stats);

    BOOST_CHECK_CLOSE( lp, msfg_lp, DBL_ACCURACY );
    BOOST_CHECK( stats == msfg_stats );
}


// Normal scenario for one word, same data as in TransitionForwardBackwardTest7
// Multiple words in the MSFG
BOOST_AUTO_TEST_CASE(MSFGViterbiTest1)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    transitions["i"]["sa"] = log(0.8);
    transitions["a"]["a"] = log(0.8);
    transitions["ki"]["sa"] = log(0.8);
    transitions["kis"]["a"] = log(0.8);
    transitions["kissa"]["a"] = log(0.8);
    transitions["sa"]["a"] = log(0.8);

    string sentence("kissa");
    FactorGraph fg(sentence, start_end, vocab, 5);

    MultiStringFactorGraph msfg(start_end);
    msfg.add(fg);
    FactorGraph fg2("kisa", start_end, vocab, 5);
    msfg.add(fg2);
    FactorGraph fg3("kissaa", start_end, vocab, 5);
    msfg.add(fg3);
    msfg.update_factor_node_map();
    assign_scores(transitions, msfg);

    transitions_t stats;
    vector<string> viterbi_path;
    flt_type lp = viterbi(transitions, fg, viterbi_path);
    lp = viterbi(transitions, fg, stats);

    transitions_t msfg_stats;
    vector<string> msfg_viterbi_path;
    flt_type msfg_lp = viterbi(msfg, sentence, msfg_viterbi_path);
    msfg_lp = viterbi(msfg, sentence, msfg_stats);

    BOOST_CHECK_CLOSE( lp, msfg_lp, DBL_ACCURACY );
    BOOST_CHECK( viterbi_path == msfg_viterbi_path );
    BOOST_CHECK( stats == msfg_stats );
}


// Normal scenario for multiple words
BOOST_AUTO_TEST_CASE(MSFGViterbiTest2)
{
    set<string> vocab = {"k","i","s","a","sa","ki","kis","kissa"};

    transitions_t transitions;
    transitions[start_end]["k"] = log(0.5);
    transitions[start_end]["ki"] = log(0.25);
    transitions[start_end]["kis"] = log(0.4);
    transitions[start_end]["kissa"] = log(0.1);
    transitions["a"][start_end] = log(0.5);
    transitions["kissa"][start_end] = log(0.10);
    transitions["sa"][start_end] = log(0.4);
    transitions["ki"]["s"] = log(0.25);
    transitions["k"]["i"] = log(0.5);
    transitions["i"]["s"] = log(0.5);
    transitions["s"]["s"] = log(0.5);
    transitions["s"]["sa"] = log(0.5);
    transitions["s"]["a"] = log(0.5);
    transitions["kis"]["sa"] = log(0.4);
    transitions["kis"]["s"] = log(0.4);

    transitions["i"]["sa"] = log(0.8);
    transitions["a"]["a"] = log(0.8);
    transitions["ki"]["sa"] = log(0.8);
    transitions["kis"]["a"] = log(0.8);
    transitions["kissa"]["a"] = log(0.8);
    transitions["sa"]["a"] = log(0.8);

    map<string, flt_type> word_freqs = {{"kissa", 1.0}, {"kisa", 2.0}, {"kissaa", 3.0}};
    transitions_t stats;
    MultiStringFactorGraph msfg(start_end);
    flt_type lp = 0.0;
    for (auto wit = word_freqs.begin(); wit != word_freqs.end(); ++wit) {
        FactorGraph fg(wit->first, start_end, vocab, 5);
        transitions_t curr_stats;
        flt_type curr_lp = viterbi(transitions, fg, curr_stats);
        lp += wit->second * curr_lp;
        Bigrams::update_trans_stats(curr_stats, wit->second, stats);
        msfg.add(fg);
    }
    msfg.update_factor_node_map();
    assign_scores(transitions, msfg);

    transitions_t msfg_stats;
    flt_type msfg_lp = viterbi(msfg, word_freqs, msfg_stats);

    BOOST_CHECK_CLOSE( lp, msfg_lp, DBL_ACCURACY );
    BOOST_CHECK( stats == msfg_stats );
}
