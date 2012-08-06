#ifndef SERVER_H
#define SERVER_H

#include "defines.h"
#include "macros.h"
#include "EventMessage.h"
#include <Windows.h>
#include <string>
#include <map>

namespace CsIpc
{

    class Server
    {
        public:
            Server(const std::string name);
            virtual ~Server();
            const std::string& GetName();
            void Send(std::string &target, EventMessage &msg);
            void Broadcast(EventMessage &msg);
            bool Peek(EventMessage &msg);

        protected:
            std::string name;
            HANDLE hPublicPipe;
            std::map<std::string, HANDLE> clientPipes;
    };
}

#endif // SERVER_H
