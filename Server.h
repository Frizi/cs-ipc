#ifndef SERVER_H
#define SERVER_H


#include <vector>
#include <map>
#include <string>

#include "EventMessage.h"


namespace CsIpc
{
    typedef std::map<std::string, std::pair<int,int> > eventTable_t;

    struct clientData
    {
        std::string name;
        void* privateQueue;
        std::vector<int> regEvts;
    };

    class Server
    {
        public:
            Server(const std::string name);
            virtual ~Server();
            const std::string& GetName();
            void Send(const std::string &target, EventMessage &msg);
            void Broadcast(EventMessage &msg);
            bool Peek(EventMessage &msg);

            static const std::string GetQueueName(std::string servername)
            {
                return "scipcpub_" + servername;
            }
        protected:
            std::string name;
            void* publicQueue;
            // name of event, internal ID, times registered

            unsigned int nextEventId;
            std::map<std::string, std::pair<int,int> > eventTable;

            std::vector<clientData*> clientRefs;

            std::map<std::string, clientData*> clientsByName;
    };
}

#endif // SERVER_H
