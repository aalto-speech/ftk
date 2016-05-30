#include <boost/test/unit_test.hpp>

#include "StringSet.hh"

using namespace std;


// Basic add & get
BOOST_AUTO_TEST_CASE(StringSetTest1)
{
    map<string, flt_type> vocab;
    vocab["hei"] = -1.0;
    StringSet ss(vocab);

    BOOST_CHECK_EQUAL ( -1.0, ss.get_score("hei") );
    BOOST_CHECK ( ss.includes("hei") );

    bool throws = false;
    try {
        ss.get_score("blaa");
    }
    catch (string &e) {
        throws = true;
    }
    BOOST_CHECK( throws );
    BOOST_CHECK ( !ss.includes("blaa") );

    throws = false;
    try {
        ss.get_score("h");
    }
    catch (string &e) {
        throws = true;
    }
    BOOST_CHECK( throws );
    BOOST_CHECK ( !ss.includes("h") );
}

// Add multiple strings
BOOST_AUTO_TEST_CASE(StringSetTest2)
{
    map<string, flt_type> vocab;
    vocab["hei"] = -1.0;
    vocab["heippa"] = -2.0;
    vocab["heh"] = -3.0;
    vocab["hassua"] = -4.0;
    vocab["hassu"] = -5.0;
    StringSet ss(vocab);

    BOOST_CHECK ( ss.includes("hei") );
    BOOST_CHECK ( ss.includes("heippa") );
    BOOST_CHECK ( ss.includes("heh") );
    BOOST_CHECK ( ss.includes("hassua") );
    BOOST_CHECK ( ss.includes("hassu") );
    BOOST_CHECK ( !ss.includes("h") );
    BOOST_CHECK ( !ss.includes("he") );
    BOOST_CHECK ( !ss.includes("heip") );

    BOOST_CHECK_EQUAL ( -1.0, ss.get_score("hei") );
    BOOST_CHECK_EQUAL ( -2.0, ss.get_score("heippa") );
    BOOST_CHECK_EQUAL ( -3.0, ss.get_score("heh") );
    BOOST_CHECK_EQUAL ( -4.0, ss.get_score("hassua") );
    BOOST_CHECK_EQUAL ( -5.0, ss.get_score("hassu") );

    bool throws = false;
    try {
        ss.get_score("he");
    }
    catch (string &e) {
        throws = true;
    }
    BOOST_CHECK( throws );
}

// Add and remove
BOOST_AUTO_TEST_CASE(StringSetTest3)
{
    map<string, flt_type> vocab;
    vocab["hei"] = -1.0;
    vocab["heippa"] = -2.0;
    vocab["heh"] = -3.0;
    StringSet ss(vocab);

    BOOST_CHECK ( ss.includes("hei") );
    BOOST_CHECK ( ss.includes("heippa") );
    BOOST_CHECK ( ss.includes("heh") );

    ss.remove("hei");
    BOOST_CHECK ( ss.includes("heh") );
    BOOST_CHECK ( ss.includes("heippa") );
    BOOST_CHECK ( !ss.includes("hei") );

    BOOST_CHECK_EQUAL ( -2.0, ss.get_score("heippa") );
    BOOST_CHECK_EQUAL ( -3.0, ss.get_score("heh") );

    bool throws = false;
    try {
        ss.get_score("hei");
    }
    catch (string &e) {
        throws = true;
    }
    BOOST_CHECK( throws );
}

// Add, remove and add back
BOOST_AUTO_TEST_CASE(StringSetTest4)
{
    map<string, flt_type> vocab;
    vocab["hei"] = -1.0;
    vocab["heippa"] = -2.0;
    vocab["heh"] = -3.0;
    StringSet ss(vocab);

    ss.remove("hei");
    ss.add("hei", -4.0);

    BOOST_CHECK ( ss.includes("hei") );
    BOOST_CHECK_EQUAL ( -4.0, ss.get_score("hei") );
}


// Test for assign_scores
// also removes strings not in the vocabulary
BOOST_AUTO_TEST_CASE(StringSetTest5)
{
    map<string, flt_type> vocab;
    vocab["joo"] = 50;
    vocab["heippa"] = 10;
    vocab["hei"] = 10;
    vocab["heh"] = 15;

    StringSet ss(vocab);

    auto it = vocab.find(string("heippa"));
    vocab.erase(it);
    vocab["jepsis"] = 30;

    ss.assign_scores(vocab);
    BOOST_CHECK_EQUAL ( vocab["joo"], ss.get_score("joo") );
    BOOST_CHECK_EQUAL ( vocab["hei"], ss.get_score("hei") );
    BOOST_CHECK_EQUAL ( vocab["heh"], ss.get_score("heh") );
    BOOST_CHECK_EQUAL ( vocab["jepsis"], ss.get_score("jepsis") );

    bool throws = false;
    try {
        ss.get_score("heippa");
    }
    catch (string &e) {
        throws = true;
    }
    BOOST_CHECK ( throws );
    BOOST_CHECK ( vocab.size() == ss.string_count() );
}
