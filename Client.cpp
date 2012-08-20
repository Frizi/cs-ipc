#include "Client.h"
#include "Server.h"

#include <string>
#include <iostream>
#include <streambuf>
#include <sstream>

#include <boost/interprocess/ipc/message_queue.hpp>

#include "defines.h"

using namespace boost::interprocess;

namespace CsIpc
{
    Client::Client(const std::string name, const std::string servername)
    {
        // copy name
        this->name = name;
        message_queue::remove(Client::GetQueueName(this->name).c_str());

        try
        {
            this->publicQueue = new message_queue
                    (open_only
                    ,Server::GetQueueName(servername).c_str()
                    );
        } catch(interprocess_exception e) {
            std::cerr << "[ERROR] Client: publicQueue exception: " << e.what();
            throw e;
        }

        try
        {
            this->privateQueue = new message_queue
                    (create_only
                    ,Client::GetQueueName(this->name).c_str()
                    ,4                   // max messages in queue
                    ,MAX_MSG_SIZE        // max message size
                    );
        } catch(interprocess_exception e) {
            std::cerr << "[ERROR] Client: privateQueue exception: " << e.what();
            throw e;
        }

        EventMessage handshakeMsg(HANDSHAKE_MSG);
        handshakeMsg.pushParam(this->name);
        this->Send(handshakeMsg, PRIORITY_HANDSHAKE);
    }

    Client::~Client()
    {
        EventMessage regMsg(DISCONNECT_MSG);
        this->Send(regMsg, PRIORITY_HANDSHAKE);

        delete (message_queue*)this->privateQueue;
        delete (message_queue*)this->publicQueue;
        message_queue::remove(Client::GetQueueName(this->name).c_str());
    }

    void Client::Send(EventMessage &msg)
    {
        this->Send(msg, PRIORITY_STANDARD);
    }

    void Client::Send(EventMessage &msg, unsigned int priority)
    {
        message_queue* const mq = (message_queue*)this->publicQueue;

        msg.setSender(this->name);
        std::stringbuf msgBuffer;
        msg.serialize(msgBuffer);

        size_t bufSize = msgBuffer.str().size();
        if(bufSize <= MAX_MSG_SIZE)
        {
            mq->send(msgBuffer.str().c_str(), bufSize, priority);
        }
        else
        {
            std::cerr << "[ERROR] Client::Send: message too long\n";
            // TODO: implement spliting messages
        }
    }

    void Client::SendTo(const std::string target, EventMessage &msg)
    {
        message_queue* const mq = (message_queue*)this->publicQueue;

        msg.setSender(this->name);
        std::stringbuf msgBuffer;
        msg.serialize(msgBuffer);

        size_t bufSize = msgBuffer.str().size();

        EventMessage targetPacket;



        if(bufSize <= MAX_MSG_SIZE)
        {
            mq->send(msgBuffer.str().c_str(), bufSize, PRIORITY_STANDARD);
        }
        else
        {
            std::cerr << "[ERROR] Client::Send: message too long\n";
            // TODO: implement spliting messages
        }
    }

    bool Client::Peek(EventMessage &msg)
    {
        message_queue* const mq = (message_queue*)this->privateQueue;

        std::stringbuf msgBuffer;

        char buff[MAX_MSG_SIZE];
        size_t recvd;
        unsigned int priority;

        if(mq->try_receive(buff, MAX_MSG_SIZE, recvd, priority))
        {
            msgBuffer.sputn(buff, recvd);
            msg.deserialize(msgBuffer);
        }
        else
            return false;
        return true;
    }

    void Client::Register(std::string eventType)
    {
        EventMessage regMsg(REGISTER_MSG);
        regMsg.pushParam(eventType);
        this->Send(regMsg, PRIORITY_REGISTER);
    }

    void Client::Unregister(std::string eventType)
    {
        EventMessage regMsg(UNREGISTER_MSG);
        regMsg.pushParam(eventType);
        this->Send(regMsg, PRIORITY_REGISTER);
    }
}
