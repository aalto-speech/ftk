#include "fetest.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION (fetest);

void fetest :: setUp (void)
{
    // set up test environment (initializing objects)
    std::string blaa("blaa");

}

void fetest :: tearDown (void)
{ }

void fetest :: addTest (void)
{
    CPPUNIT_ASSERT_EQUAL (blaa.length(), 4);
}
