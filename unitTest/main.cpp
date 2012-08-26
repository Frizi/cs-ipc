#include <Server.h>
#include <Client.h>


#define BOOST_TEST_MODULE csipcTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( server_name )
{
    CsIpc::Server test_server( "serv" );
    BOOST_CHECK( test_server.GetName() == "serv" );
}

BOOST_AUTO_TEST_CASE( message_parameters )
{
    CsIpc::EventMessage message;
    message.pushParam(1);
    message.pushParam(2.5f);
    message.pushParam("testcase");
    message.pushParam(L"widestring");

    BOOST_CHECK_EQUAL(message.paramCount(), 4);
    BOOST_CHECK_EQUAL(message.getParamInt(0), 1);
    BOOST_CHECK_EQUAL(message.getParamFloat(1), 2.5f);
    BOOST_CHECK_EQUAL(message.getParamString(2), "testcase");
    BOOST_CHECK(message.getParamWstring(3) == L"widestring");
}

BOOST_AUTO_TEST_CASE( message_serialization )
{
    std::stringbuf buf;
    CsIpc::EventMessage message1;
    CsIpc::EventMessage message2;

    message1.pushParam(7);
    message1.pushParam(1.5f);
    message1.pushParam("testcase");
    message1.pushParam(L"widestring");

    message1.serialize(buf);
    message2.deserialize(buf);

    BOOST_CHECK_EQUAL(message2.getParameterType(0), CsIpc::T_INT);
    BOOST_CHECK_EQUAL(message2.getParameterType(1), CsIpc::T_FLOAT);
    BOOST_CHECK_EQUAL(message2.getParameterType(2), CsIpc::T_STR);
    BOOST_CHECK_EQUAL(message2.getParameterType(3), CsIpc::T_WSTR);
    BOOST_CHECK_EQUAL(message2.paramCount(), 4);
}

BOOST_AUTO_TEST_CASE( client_server )
{
    CsIpc::Server serv("serv");
    CsIpc::Client client("client", "serv");
    client.Register("testevt");

    CsIpc::EventMessage send;
    CsIpc::EventMessage recv;

    BOOST_CHECK(!serv.Peek(recv)); // server need to peek to handle handshake and register

    send.setEventType("testevt");
    send.pushParam(5);

    serv.Broadcast(send);

    BOOST_CHECK(client.Peek(recv));
    BOOST_CHECK_EQUAL(recv.getEventType(), send.getEventType());
    BOOST_CHECK_EQUAL(recv.getParamInt(0), 5);
}

BOOST_AUTO_TEST_CASE( client_wait )
{
    CsIpc::Server serv("serv");
    CsIpc::Client client("client", "serv");
    client.Register("waitfor");
    client.Register("test1");
    client.Register("test2");

    CsIpc::EventMessage send;
    CsIpc::EventMessage recv;

    BOOST_CHECK(!serv.Peek(recv)); // server need to peek to handle handshake and register

    BOOST_CHECK(!client.WaitForEvent(recv, "waitfor", 1)); // timeout

    send.pushParam(5);

    send.setEventType("test1");
    serv.Broadcast(send);
    send.setEventType("test2");
    serv.Broadcast(send);

    send.pushParam(1.5f);
    send.setEventType("waitfor");
    serv.Broadcast(send);

    BOOST_CHECK(client.WaitForEvent(recv, "waitfor"));

    BOOST_CHECK_EQUAL(recv.getEventType(), "waitfor");
    BOOST_CHECK_EQUAL(recv.getParamInt(0), 5);
    BOOST_CHECK_EQUAL(recv.getParamFloat(1), 1.5f);

    BOOST_CHECK(client.Peek(recv));

    BOOST_CHECK_EQUAL(recv.getEventType(), "test1");
    BOOST_CHECK_EQUAL(recv.getParamInt(0), 5);

    BOOST_CHECK(client.Peek(recv));

    BOOST_CHECK_EQUAL(recv.getEventType(), "test2");
    BOOST_CHECK_EQUAL(recv.getParamInt(0), 5);
}
