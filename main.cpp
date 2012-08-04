#include <iostream>
#include "Server.h"

using namespace std;

int main(int argc, char *argv[])
{

    cout << "client or server? : ";
    std::string in;
    cin >> in;

    if(in.c_str()[0] == 's')
    {
        cout << "server\n";
        CsIpc::Server serv("testServ");
        cin >> in;
    }
    else
    {
        cout << "client\n";
    }


    return 0;
}
