#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "EventMessage.h"

namespace CsIpc
{
    class Client
    {
        public:
            Client(const std::string name, const std::string servername);
            virtual ~Client();

            void Send(EventMessage &msg);
            void Send(EventMessage &msg, unsigned int priority);
            bool Peek(EventMessage &msg);
            void Register(std::string eventType);

            static const std::string GetQueueName(std::string clientname)
            {
                return "scipcpriv_" + clientname;
            }

        protected:
            std::string name;
            void* publicQueue;
            void* privateQueue;
    };
}
#endif // CLIENT_H
