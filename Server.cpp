#include "Server.h"
#include <Windows.h>
#include <string>

#include <iostream>
#include <boost/foreach.hpp>

namespace CsIpc
{
    Server::Server(const char* name)
    {
        this->name = new char[strlen(name)+1];
        strcpy(this->name, name);

        std::wstring pipename = GetPublicPipename(this->name);


        hPublicPipe = CreateNamedPipe(
                    pipename.c_str(),
                    PIPE_ACCESS_INBOUND,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                    PIPE_UNLIMITED_INSTANCES,
                    0,
                    1024,
                    0,
                    NULL);

        std::wcerr << L"[DEBUG] Pipe name: " << pipename << L"\n";
    }


    const char* Server::GetName()
    {
        return name;
    }

    Server::~Server()
    {
        delete[] name;
        CloseHandle(hPublicPipe);
    }

    void Server::Send(std::string &target, Message &msg)
    {

    }

    void Server::Broadcast(Message &msg)
    {

    }

    bool Server::PeekMessage(std::string &sender, Message &msg)
    {

    }
}
