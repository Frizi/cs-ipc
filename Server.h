#ifndef SERVER_H
#define SERVER_H


#include <vector>
#include <map>
#include <string>

#include "EventMessage.h"


namespace CsIpc
{

    struct clientData
    {
        std::string name;
        void* privateQueue;
        std::vector<std::string> regEvts;
    };

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

            static const std::string GetQueueName(std::string servername)
            {
                return "scipcpub_" + servername;
            }
        protected:
            void Send(const clientData* targetData, EventMessage &msg);

            std::string name;
            void* publicQueue;
            // name of event, internal ID, times registered

            eventTable_t eventTable;

            std::vector<clientData*> clientRefs;

            clientsByName_t clientsByName;
    };
}

#endif // SERVER_H
