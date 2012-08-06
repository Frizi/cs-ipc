#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <Windows.h>
#include "EventMessage.h"

namespace CsIpc
{
    class Client
    {
        public:
            Client(const std::string name, const std::string servername);
            virtual ~Client();
            void Send(EventMessage &msg);
            bool Peek(EventMessage &msg);

        protected:
            std::string name;
            HANDLE hPrivatePipe;
            HANDLE hPublicPipe;
    };
}
#endif // CLIENT_H
