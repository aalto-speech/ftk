#ifndef FACTOR_ENCODER_CPPUNIT
#define FACTOR_ENCODER_CPPUNIT

#include "fe.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class fetest : public CPPUNIT_NS :: TestFixture
{
    CPPUNIT_TEST_SUITE (fetest);
    CPPUNIT_TEST (addTest);
    CPPUNIT_TEST_SUITE_END ();

    public:
        void setUp (void);
        void tearDown (void);

    protected:
        void addTest (void);

    private:
        std::string *blaa;
};

#endif

