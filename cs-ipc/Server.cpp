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
        delete (message_queue*)this->publicQueue;
        message_queue::remove(Server::GetQueueName(this->name).c_str());
    }

    void Server::Send(const std::string &target, EventMessage &msg)
    {
        clientsByName_t::iterator cdit;
        cdit = clientsByName.find(msg.getSender());
        if(cdit == clientsByName.end())
        {
            throw "Unknown client choosen as target";
        }
        Send(cdit->second, msg);
    }

    void Server::Send(const clientData* targetData, EventMessage &msg)
    {
        message_queue* const mq = (message_queue*)targetData->privateQueue;

        // sender must be original
        // --msg.setSender(this->name);

        std::stringbuf msgBuffer;
        msg.serialize(msgBuffer);

        size_t bufSize = msgBuffer.str().size();
        if(bufSize <= MAX_MSG_SIZE)
        {
            mq->send(msgBuffer.str().c_str(), bufSize, PRIORITY_STANDARD);
        }
        else
        {
            std::cerr << "[ERROR] Server::Send(ClientData*, EventMessage&): message too long\n";
            // TODO: implement spliting messages
        }
    }

    void Server::Broadcast(EventMessage &msg)
    {
        std::string type = msg.getEventType();
        std::vector<clientData*>* clients;

        eventTable_t::iterator it;
        it = eventTable.find(type);
        if(it == eventTable.end())
            return;

        clients = &(it->second.first);

        BOOST_FOREACH(clientData* client, *clients)
        {
            this->Send(client, msg);
        }
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

            clientData* cd;

            const bool msgIsHandshake = (priority == PRIORITY_HANDSHAKE
               && (0 == msg.getEventType().compare(HANDSHAKE_MSG)));

            if(!msgIsHandshake)
            {
                clientsByName_t::iterator cdit;
                cdit = clientsByName.find(msg.getSender());
                if(cdit == clientsByName.end())
                {
                    throw "Unknown client tried to send message";
                }
                cd = cdit->second;
            }

            if(msgIsHandshake)
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

                return this->Peek(msg);
            }
            else if(priority == PRIORITY_REGISTER
               && (0 == msg.getEventType().compare(REGISTER_MSG)) )
            {
                std::string eventName = msg.getParamString(0);

                BOOST_FOREACH(std::string registered, cd->regEvts)
                {
                    if(0 == registered.compare(eventName))
                        return this->Peek(msg);
                }
                cd->regEvts.push_back(eventName);

                eventTable_t::iterator it;
                it = eventTable.find(eventName);

                if(it == eventTable.end())
                {
                    eventTable[eventName] = std::make_pair(std::vector<clientData*>(),1);
                    eventTable[eventName].first.push_back(cd);
                }
                else
                {
                    it->second.second++;
                    it->second.first.push_back(cd);
                }
                return this->Peek(msg);
            }
            else if(priority == PRIORITY_COMMAND
               && (0 == msg.getEventType().compare(DIRECTSEND_MSG)) )
            {
                std::string target = msg.getParamString(0);
                clientsByName_t::iterator targetDataIt = clientsByName.find(target);
                if(targetDataIt != clientsByName.end())
                {
                    std::stringbuf buf;

                    std::string serializedMsg = msg.getParamString(1);
                    buf.sputn(serializedMsg.data(), serializedMsg.size());
                    msg.deserialize(buf);
                    this->Send(targetDataIt->second, msg);
                }
                return this->Peek(msg);
            }
            else if(priority == PRIORITY_REGISTER
               && (0 == msg.getEventType().compare(UNREGISTER_MSG)) )
            {
                std::string eventName = msg.getParamString(0);
                this->UnregisterEvent(cd, eventName);

                return this->Peek(msg);
            }
            else if(priority == PRIORITY_STANDARD
               && (0 == msg.getEventType().compare(DISCONNECT_MSG)) )
            {
                std::vector<std::string> regCopy = cd->regEvts;

                BOOST_FOREACH(std::string & eventName, regCopy)
                {
                    this->UnregisterEvent(cd, eventName);
                }


                clientsByName.erase(cd->name);

                BOOST_FOREACH(clientData* & cdSearch, this->clientRefs)
                {
                    if(cdSearch == cd)
                    {
                        if(this->clientRefs.size() > 1)
                            cdSearch = this->clientRefs.back();
                        this->clientRefs.pop_back();

                        delete (message_queue*)cd->privateQueue;
                        message_queue::remove(Client::GetQueueName(cd->name).c_str());
                        delete cd;

                        break;
                    }
                }

                return this->Peek(msg);
            }
            else
            {
                // msg currently deserialized, all work is done
            }
        }
        else
            return false;
        return true;
    }

    void Server::UnregisterEvent(clientData* client, std::string eventType)
    {
        // search for event in client's vector
        BOOST_FOREACH(std::string & registered, client->regEvts)
        {
            if(0 == registered.compare(eventType))
            {
                // swap with end and pop
                if(client->regEvts.size() > 1)
                    registered = client->regEvts.back();
                client->regEvts.pop_back();

                eventTable_t::iterator eventIter = eventTable.find(eventType);

                BOOST_FOREACH(clientData* & eventCd, eventIter->second.first)
                {
                    // exact pointer match
                    if(eventCd == client)
                    {
                        // swap with end
                        if(eventIter->second.first.size() > 1)
                            eventCd = eventIter->second.first.back();
                        eventIter->second.first.pop_back();

                        // decrement
                        eventIter->second.second--;
                        break;
                    }
                }
                // if no registers left, delete event from table
                if(eventIter->second.second == 0)
                {
                    eventTable.erase(eventIter);
                }

                return;
            }
        }
    }

}
