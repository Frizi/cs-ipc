#include "Client.h"

#include <Windows.h>
#include "macros.h"

#include <string>
#include <iostream>
#include <streambuf>
#include <sstream>

namespace CsIpc
{
    Client::Client(const std::string name, const std::string servername)
    {
        std::wstring pipename = GetPublicPipename(servername);
        hPublicPipe = CreateFile(
                pipename.c_str(),
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

        if(hPublicPipe == INVALID_HANDLE_VALUE)
        {
            throw ("Client: Error opening public pipe");
        }


        pipename = GetPublicPipename("client_"+name);
        hPrivatePipe = CreateNamedPipe(
            pipename.c_str(),
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            0,
            1024,
            0,
            NULL);

        if(hPrivatePipe == INVALID_HANDLE_VALUE)
        {
            throw ("Client: Error creating private pipe");
        }


    }

    Client::~Client()
    {

        CloseHandle(hPublicPipe);
        CloseHandle(hPrivatePipe);
    }

    void Client::Send(EventMessage &msg)
    {
        msg.setSender(this->name);
        std::stringbuf msgBuffer;
        msg.serialize(msgBuffer);

        DWORD nwritten;
        WriteFile(hPublicPipe,
                  msgBuffer.str().c_str(),
                  msgBuffer.str().size(),
                  &nwritten,NULL);
    }

    bool Client::Peek(EventMessage &msg)
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
