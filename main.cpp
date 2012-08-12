#include <iostream>
#include "Server.h"
#include "Client.h"

#include <windows.h>
#include <ctime>

using namespace std;

int usage(char* progname)
{
    std::cout << "usage: " << progname << " (-s servername) | (-c clientname servername)\n";
    return 0;
}

int main(int argc, char *argv[])
{
    try
    {
        if(argc == 3 && strcmp(argv[1], "-s") == 0)
        {
            cout << "server\n";
            CsIpc::Server serv(argv[2]);
            CsIpc::EventMessage msg;
            CsIpc::EventMessage resp;
            resp.setEventType("say");
            resp.pushParam("Hello, World!");
            while(true)
            {
                if(serv.Peek(msg))
                {
                    std::cout << "[broadcasting " << msg.getSender()
                              << ": " << msg.getEventType();
                    if(msg.paramCount() >= 1 && msg.getParameterType(0) == CsIpc::T_STR)
                        std::cout << " - " << msg.getParamString(0);
                    std::cout << "]\n";
                    serv.Broadcast(msg);
                }
                Sleep(2);
            }
        }
        else if(argc == 4 && strcmp(argv[1], "-c") == 0)
        {
            srand(time(NULL));

            cout << "client\n";
            CsIpc::Client client(argv[2], argv[3]);
            std::string buf;
            CsIpc::EventMessage msg;

            client.Register("simplewrite");

            std::string msgdata;
            while(true)
            {
                if(client.Peek(msg))
                {
                    if(msg.getEventType().compare("simplewrite") == 0)
                    {
                        std::cout << msg.getSender() << ": " << msg.getParamString(0) << "\n";
                    }
                }
                Sleep(2);
                if(rand()%100 == 99)
                {
                    msgdata = "";
                    for(int i = 0; i < 4; i++)
                    {
                        msgdata += 'A' + ( rand() % ('Z' - 'A') );
                    }

                    CsIpc::EventMessage writemsg;
                    writemsg.setEventType("simplewrite");
                    writemsg.pushParam(msgdata);
                    client.Send(writemsg);
                    cout << "[sent " << msgdata << "]\n";
                }
            }
        }
        else
        {
            return usage(argv[0]);
        }
    } catch(const char* str)
    {
        std::cerr << "Exception raised: " << str << "\n";
        return 1;
    }

    return 0;
}
