#include <iostream>
#include "Server.h"
#include "Client.h"

#include <windows.h>

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
            CsIpc::Server serv("testServer");
            CsIpc::EventMessage msg;
            CsIpc::EventMessage resp;
            resp.setEventType("say");
            resp.pushParam("Hello, World!");
            while(true)
            {
                if(serv.Peek(msg))
                {
                    std::cout << "[MSG] " << msg.getSender() << ": " << msg.getEventType() << "\n";
                    serv.Send(msg.getSender(), resp);
                }
                Sleep(2);
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
                std::getline(std::cin, buf);
                msg.setEventType(buf);
                client.Send(msg);
                cout << "[MSG SENT]\n";
            }
        }
    } catch(const char* str)
    {
        std::cerr << "Exception raised: " << str << "\n";
        return 1;
    }

    return 0;
}
