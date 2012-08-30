#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <deque>
#include "EventMessage.h"

namespace CsIpc
{
    struct packetData_t;

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
            size_t ClientsRegistered(std::string eventType);

            void SendTo(std::string target, EventMessage &msg);

            static const std::string GetQueueName(std::string clientname)
            {
                return "scipcpriv_" + clientname;
            }

        protected:
            bool HandleMessage(EventMessage &msg, size_t priority);
            std::string name;
            void* publicQueue;
            void* privateQueue;
            size_t emptyPacketSize;
            std::deque<EventMessage> storedMessages;
            packetData_t* packetData;
    };
}
#endif // CLIENT_H
