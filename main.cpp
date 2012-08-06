#include <iostream>
#include "Server.h"
#include "Client.h"


using namespace std;

int main(int argc, char *argv[])
{
    try
    {
        cout << "client or server? : ";
        std::string in;
        cin >> in;

        if(in.c_str()[0] == 's')
        {
            cout << "server\n";
            CsIpc::Server serv("testServ");
            CsIpc::EventMessage msg;
            std::string sender;
            while(true)
            {
                if(serv.Peek(sender, msg))
                {
                    std::cout << "[MSG] " << sender << ": " << msg.getEventType() << "\n";
                }
            }
        }
        else
        {
            cout << "client\n";
            CsIpc::Client client("testClient", "testServer");
            std::string buf;
            CsIpc::EventMessage msg;
            while(true)
            {
                cin >> buf;
                msg.setEventType(buf);
                client.Send(msg);
            }
        }
    } catch(const char* str)
    {
        std::cerr << "Exception raised: " << str << "\n";
        exit(1);
    }

    return 0;
}
