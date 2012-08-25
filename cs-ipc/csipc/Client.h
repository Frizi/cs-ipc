#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <deque>
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
            bool WaitForEvent(EventMessage &msg, std::string eventType, unsigned int timeout = 0);
            void Register(std::string eventType);
            void Unregister(std::string eventType);

            bool IsClientConnected(std::string target);
            void SendTo(std::string target, EventMessage &msg);

            static const std::string GetQueueName(std::string clientname)
            {
                return "scipcpriv_" + clientname;
            }

        protected:
            std::string name;
            void* publicQueue;
            void* privateQueue;
            std::deque<EventMessage> storedMessages;
    };
}
#endif // CLIENT_H
