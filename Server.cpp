#include "Server.h"

#include <Windows.h>

#include <string>
#include <iostream>
#include <streambuf>
#include <sstream>

#include <boost/foreach.hpp>

namespace CsIpc
{
    Server::Server(const std::string name)
    {
        // check: http://stackoverflow.com/questions/3650876/not-getting-any-response-from-named-pipe-server?rq=1
        // copy name
        this->name = name;

        std::wstring pipename = GetPublicPipename(this->name);

        hPublicPipe = CreateNamedPipe(
                    pipename.c_str(),
                    PIPE_ACCESS_INBOUND,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                    PIPE_UNLIMITED_INSTANCES,
                    0,
                    2048,
                    0,
                    NULL);



        std::wcerr << L"[DEBUG] Server: Pipe name: " << pipename << "\n";

        if(hPublicPipe == INVALID_HANDLE_VALUE)
        {
            throw ("Server: Error creating public pipe");
        }

        bool connected = ConnectNamedPipe(hPublicPipe,NULL);
        std::cerr << "[DEBUG] Server: Pipe connect status: " << connected << "\n";
    }


    const std::string& Server::GetName()
    {
        return name;
    }

    Server::~Server()
    {
        CloseHandle(hPublicPipe);
    }

    void Server::Send(std::string &target, EventMessage &msg)
    {

    }

    void Server::Broadcast(EventMessage &msg)
    {

    }

    bool Server::Peek(EventMessage &msg)
    {
        std::stringbuf msgBuffer;

        char readBuffer[2048];
        DWORD nread;
        if(!ReadFile(hPublicPipe,readBuffer,2048,&nread,NULL)) return false;

        msgBuffer.sputn(readBuffer, nread);
        msg.deserialize(msgBuffer);

        return true;
    }
}
