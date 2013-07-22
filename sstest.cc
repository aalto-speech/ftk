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
    StringSet<flt_type> ss;
    string factor("hei");
    ss.add(factor, -1.0);
    CPPUNIT_ASSERT_EQUAL ( -1.0, ss.get_score(factor) );
    CPPUNIT_ASSERT ( ss.includes(factor) );

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


void sstest :: StringSetTest2 (void)
{
}


void sstest :: StringSetTest3 (void)
{
}
