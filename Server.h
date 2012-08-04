#ifndef SERVER_H
#define SERVER_H

#include "defines.h"
#include "macros.h"
#include "Message.h"
#include <Windows.h>
#include <map>

namespace CsIpc
{

    class Server
    {
        public:
            Server(const char* name);
            virtual ~Server();
            const char* GetName();
            void Send(std::string &target, Message &msg);
            void Broadcast(Message &msg);
            bool PeekMessage(std::string &sender, Message &msg);

        protected:
            char* name;
            HANDLE hPublicPipe;
        private:
            std::map<std::string, HANDLE> clientPipes;
    };
}

#endif // SERVER_H
