#include "fetest.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION (fetest);

void fetest :: setUp (void)
{
    // set up test environment (initializing objects)
    blaa = new std::string("blaa");

}

void fetest :: tearDown (void)
{
    delete blaa;
}

void fetest :: addTest (void)
{
    std::cout << "Blaaaa" << std::endl;
    CPPUNIT_ASSERT_EQUAL ((int)blaa->length(), 8);
}

