#ifndef SERVER_H
#define SERVER_H


#include <vector>
#include <map>
#include <string>

#include "EventMessage.h"

#include <iostream>

namespace CsIpc
{
    struct clientData;

    typedef std::map<std::string, std::pair<std::vector<clientData*>,int> > eventTable_t;
    typedef std::map<std::string, clientData*> clientsByName_t;

    class Server
    {
        public:
            Server(const std::string name);
            virtual ~Server();
            const std::string& GetName();
            void Send(const std::string &target, EventMessage &msg);
            void Broadcast(EventMessage &msg);
            bool Peek(EventMessage &msg);

            size_t GetNumOfClients()
            {
                return clientRefs.size();
            }

            static const std::string GetQueueName(std::string servername)
            {
                return "scipcpub_" + servername;
            }
        protected:
            bool HandleMessage(EventMessage &msg, size_t priority);
            void Send(const clientData* targetData, EventMessage &msg);
            void UnregisterEvent(clientData* client, std::string eventType);
            std::string name;
            void* publicQueue;
            size_t emptyPacketSize;
            // name of event, internal ID, times registered
            eventTable_t eventTable;

            std::vector<clientData*> clientRefs;

            clientsByName_t clientsByName;
    };
}

#endif // SERVER_H
