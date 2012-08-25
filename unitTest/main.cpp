#include <Server.h>
#define BOOST_TEST_MODULE csipcTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( my_test )
{
    CsIpc::Server test_server( "serv" );

    BOOST_CHECK( test_server.GetName() == "serv" );
}
