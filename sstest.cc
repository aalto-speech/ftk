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

// Check that arcs are sorted by their cumulative counts in each node
void sstest :: StringSetTest5 (void)
{
    StringSet ss;
    ss.add("joo", -0.25);
    ss.add("heippa", -1.5);
    ss.add("hei", -1.0);
    ss.add("heh", -2.0);
    ss.optimize_arcs(&ss.root_node, true);

    StringSet::Node *node = &ss.root_node;
    StringSet::Arc *arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'j', arc->letter );
    arc = arc->sibling_arc;
    CPPUNIT_ASSERT_EQUAL ( 'h', arc->letter );
    CPPUNIT_ASSERT ( arc->sibling_arc == NULL );

    node = arc->target_node;
    arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'e', arc->letter );
    node = arc->target_node;

    arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'i', arc->letter );
    arc = arc->sibling_arc;
    CPPUNIT_ASSERT_EQUAL ( 'h', arc->letter );
    CPPUNIT_ASSERT ( arc->sibling_arc == NULL );
}

// Check that arcs are sorted by their cumulative counts in each node
// Non-log domain
void sstest :: StringSetTest6 (void)
{
    StringSet ss;
    ss.add("joo", 50);
    ss.add("heippa", 10);
    ss.add("hei", 10);
    ss.add("heh", 15);
    ss.optimize_arcs(&ss.root_node, false);

    StringSet::Node *node = &ss.root_node;
    StringSet::Arc *arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'j', arc->letter );
    arc = arc->sibling_arc;
    CPPUNIT_ASSERT_EQUAL ( 'h', arc->letter );
    CPPUNIT_ASSERT ( arc->sibling_arc == NULL );

    node = arc->target_node;
    arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'e', arc->letter );
    node = arc->target_node;

    arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'i', arc->letter );
    arc = arc->sibling_arc;
    CPPUNIT_ASSERT_EQUAL ( 'h', arc->letter );
    CPPUNIT_ASSERT ( arc->sibling_arc == NULL );
}

// Test pruning unused nodes and arcs.
void sstest :: StringSetTest7 (void)
{
    StringSet ss;
    ss.add("joo", 50);
    ss.add("heippa", 10);
    ss.add("hei", 10);
    ss.add("heh", 15);
    ss.optimize_arcs(&ss.root_node, false);

    ss.remove("heippa");
    ss.prune();

    StringSet::Node *node = &ss.root_node;
    StringSet::Arc *arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'j', arc->letter );
    arc = arc->sibling_arc;
    CPPUNIT_ASSERT_EQUAL ( 'h', arc->letter );
    CPPUNIT_ASSERT ( arc->sibling_arc == NULL );

    node = arc->target_node;
    arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'e', arc->letter );
    node = arc->target_node;

    arc = node->first_arc;
    CPPUNIT_ASSERT_EQUAL ( 'i', arc->letter );
    node = arc->target_node;
    CPPUNIT_ASSERT ( node->first_arc == NULL );
}
