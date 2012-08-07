#include "Server.h"
#include "Client.h"

#include <string>
#include <iostream>
#include <streambuf>
#include <sstream>

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/foreach.hpp>

#include "defines.h"

using namespace boost::interprocess;

namespace CsIpc
{

    Server::Server(const std::string name)
    {
        this->nextEventId = 0;

        // copy name
        this->name = name;

        message_queue::remove(Server::GetQueueName(this->name).c_str());

        try
        {
        this->publicQueue = new message_queue
                (create_only
                ,Server::GetQueueName(this->name).c_str()
                ,16                  // max messages in queue
                ,MAX_MSG_SIZE        // max message size
                );
        } catch(interprocess_exception e) {
            std::cerr << "[ERROR] Server: publicQueue exception: " << e.what();
            throw e;
        }

    }


    const std::string& Server::GetName()
    {
        return name;
    }

    Server::~Server()
    {
        std::cerr << "[DEBUG] message_queue remove" << Server::GetQueueName(this->name) << "\n";
        delete (message_queue*)this->publicQueue;
        message_queue::remove(Server::GetQueueName(this->name).c_str());
    }

    void Server::Send(const std::string &target, EventMessage &msg)
    {

    }

    void Server::Broadcast(EventMessage &msg)
    {

    }

    bool Server::Peek(EventMessage &msg)
    {
        message_queue* const mq = (message_queue*)this->publicQueue;

        std::stringbuf msgBuffer;

        char buff[MAX_MSG_SIZE];
        size_t recvd;
        unsigned int priority;
        if(mq->try_receive(buff, MAX_MSG_SIZE, recvd, priority))
        {
            msgBuffer.sputn(buff, recvd);
            msg.deserialize(msgBuffer);

            if(priority == PRIORITY_HANDSHAKE
               && (0 == msg.getEventType().compare(HANDSHAKE_MSG)) )
            {
                std::string clientName = msg.getParamString(0);

                clientData* client = new clientData;

                client->name = msg.getParamString(0);
                //client->regEvts.clear();
                try
                {
                    client->privateQueue = new message_queue
                            (open_only
                            ,Client::GetQueueName(client->name).c_str()
                            );
                } catch(interprocess_exception e) {
                    delete[] client;
                    std::cerr << "[ERROR] Client: handshake exception: " << e.what();
                    throw e;
                }

                clientRefs.push_back(client);
                clientsByName[client->name] = client;
            }
            else if(priority == PRIORITY_REGISTER
               && (0 == msg.getEventType().compare(REGISTER_MSG)) )
            {
                std::string eventName = msg.getParamString(0);

                eventTable_t::iterator it;
                it = eventTable.find(eventName);

                if(it == eventTable.end())
                {
                    eventTable[eventName] = std::pair<int,int>(nextEventId, 1);
                    nextEventId++;
                }
                else
                {
                    it->second.second++;
                }

            }

            else
            {
                // broadcast
            }
        }
        else
            return false;
        return true;
    }
}
