#include <string>

#include "defs.hh"
#include "sstest.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;


CPPUNIT_TEST_SUITE_REGISTRATION (sstest);

void sstest :: setUp (void)
{
}

void sstest :: tearDown (void)
{
}

// Basic add & get
void sstest :: StringSetTest1 (void)
{
    map<string, flt_type> vocab;
    vocab["hei"] = -1.0;
    StringSet ss(vocab);

    CPPUNIT_ASSERT_EQUAL ( -1.0, ss.get_score("hei") );
    CPPUNIT_ASSERT ( ss.includes("hei") );

    bool throws = false;
    try {
        ss.get_score("blaa");
    }
    catch (string e) {
        throws = true;
    }
    CPPUNIT_ASSERT( throws );
    CPPUNIT_ASSERT ( !ss.includes("blaa") );

    throws = false;
    try {
        ss.get_score("h");
    }
    catch (string e) {
        throws = true;
    }
    CPPUNIT_ASSERT( throws );
    CPPUNIT_ASSERT ( !ss.includes("h") );
}

// Add multiple strings
void sstest :: StringSetTest2 (void)
{
    map<string, flt_type> vocab;
    vocab["hei"] = -1.0;
    vocab["heippa"] = -2.0;
    vocab["heh"] = -3.0;
    vocab["hassua"] = -4.0;
    vocab["hassu"] = -5.0;
    StringSet ss(vocab);

    CPPUNIT_ASSERT ( ss.includes("hei") );
    CPPUNIT_ASSERT ( ss.includes("heippa") );
    CPPUNIT_ASSERT ( ss.includes("heh") );
    CPPUNIT_ASSERT ( ss.includes("hassua") );
    CPPUNIT_ASSERT ( ss.includes("hassu") );
    CPPUNIT_ASSERT ( !ss.includes("h") );
    CPPUNIT_ASSERT ( !ss.includes("he") );
    CPPUNIT_ASSERT ( !ss.includes("heip") );

    CPPUNIT_ASSERT_EQUAL ( -1.0, ss.get_score("hei") );
    CPPUNIT_ASSERT_EQUAL ( -2.0, ss.get_score("heippa") );
    CPPUNIT_ASSERT_EQUAL ( -3.0, ss.get_score("heh") );
    CPPUNIT_ASSERT_EQUAL ( -4.0, ss.get_score("hassua") );
    CPPUNIT_ASSERT_EQUAL ( -5.0, ss.get_score("hassu") );

    bool throws = false;
    try {
        ss.get_score("he");
    }
    catch (string e) {
        throws = true;
    }
    CPPUNIT_ASSERT( throws );
}

// Add and remove
void sstest :: StringSetTest3 (void)
{
    map<string, flt_type> vocab;
    vocab["hei"] = -1.0;
    vocab["heippa"] = -2.0;
    vocab["heh"] = -3.0;
    StringSet ss(vocab);

    CPPUNIT_ASSERT ( ss.includes("hei") );
    CPPUNIT_ASSERT ( ss.includes("heippa") );
    CPPUNIT_ASSERT ( ss.includes("heh") );

    ss.remove("hei");
    CPPUNIT_ASSERT ( ss.includes("heh") );
    CPPUNIT_ASSERT ( ss.includes("heippa") );
    CPPUNIT_ASSERT ( !ss.includes("hei") );

    CPPUNIT_ASSERT_EQUAL ( -2.0, ss.get_score("heippa") );
    CPPUNIT_ASSERT_EQUAL ( -3.0, ss.get_score("heh") );

    bool throws = false;
    try {
        ss.get_score("hei");
    }
    catch (string e) {
        throws = true;
    }
    CPPUNIT_ASSERT( throws );
}

// Add, remove and add back
void sstest :: StringSetTest4 (void)
{
    map<string, flt_type> vocab;
    vocab["hei"] = -1.0;
    vocab["heippa"] = -2.0;
    vocab["heh"] = -3.0;
    StringSet ss(vocab);

    ss.remove("hei");
    ss.add("hei", -4.0);

    CPPUNIT_ASSERT ( ss.includes("hei") );
    CPPUNIT_ASSERT_EQUAL ( -4.0, ss.get_score("hei") );
}


// Test for assign_scores
// also removes strings not in the vocabulary
void sstest :: StringSetTest5 (void)
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
    CPPUNIT_ASSERT_EQUAL ( vocab["joo"], ss.get_score("joo") );
    CPPUNIT_ASSERT_EQUAL ( vocab["hei"], ss.get_score("hei") );
    CPPUNIT_ASSERT_EQUAL ( vocab["heh"], ss.get_score("heh") );
    CPPUNIT_ASSERT_EQUAL ( vocab["jepsis"], ss.get_score("jepsis") );

    bool throws = false;
    try {
        ss.get_score("heippa");
    }
    catch (string e) {
        throws = true;
    }
    CPPUNIT_ASSERT ( throws );
    CPPUNIT_ASSERT ( vocab.size() == ss.string_count() );
}
