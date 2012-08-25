#include "Client.h"
#include "Server.h"

#include <string>
#include <iostream>
#include <streambuf>
#include <sstream>

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

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
        this->Send(regMsg, PRIORITY_STANDARD);

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

        EventMessage targetPacket;
        targetPacket.setEventType(DIRECTSEND_MSG);
        targetPacket.pushParam(target);
        targetPacket.pushParam(msgBuffer.str());
        targetPacket.setSender(this->name);

        std::stringbuf packetBuffer;
        targetPacket.serialize(packetBuffer);

        size_t bufSize = packetBuffer.str().size();
        if(bufSize <= MAX_MSG_SIZE)
        {
            mq->send(packetBuffer.str().c_str(), bufSize, PRIORITY_COMMAND);
        }
        else
        {
            std::cerr << "[ERROR] Client::SendTo: packet too long\n";
            // TODO: implement spliting messages
        }
    }

    bool Client::WaitForEvent(EventMessage &msg, std::string eventType, unsigned int timeout)
    {
        using namespace boost::posix_time;

        message_queue* const mq = (message_queue*)this->publicQueue;

        // initialize timer
        ptime timeoutInstant;
        if(timeout != 0)
        {
            timeoutInstant = microsec_clock::universal_time() + boost::posix_time::millisec(timeout);
        }

        // check local deque for message
        if(!this->storedMessages.empty())
        {
            for (unsigned int i=0; i<this->storedMessages.size(); i++)
            {
                EventMessage & dequeMsg = this->storedMessages[i];
                if( 0 ==dequeMsg.getEventType().compare(eventType) )
                {
                    // copy message to the output
                    msg = dequeMsg;
                    // erase message from deque
                    this->storedMessages.erase(this->storedMessages.begin()+i);
                    return true;
                }
            }
        }

        char buff[MAX_MSG_SIZE];
        size_t recvd;
        unsigned int priority;
        std::stringbuf msgBuffer;

        bool gotRequest = false;
        while(!gotRequest)
        {
            // try to get message
            if(timeout == 0)
                mq->receive(buff, MAX_MSG_SIZE, recvd, priority);
            else
            {

                if(!mq->timed_receive(buff, MAX_MSG_SIZE, recvd, priority, timeoutInstant))
                {
                    // timeout reached
                    return false;
                }
            }

            // transform buffer into message
            msgBuffer.sputn(buff, recvd);
            msg.deserialize(msgBuffer);

            // check if recieved message has correct type
            if( 0 == msg.getEventType().compare(eventType) )
            {
                gotRequest = true;
            }
            else
            {
                // store recieved message into local deque
                this->storedMessages.push_back(msg);
            }

        }
        return true;
    }


    bool Client::IsClientConnected(std::string target)
    {
        message_queue* const mq = (message_queue*)this->publicQueue;
        EventMessage request;
        request.setEventType(ISCONNECTED_MSG);
        request.setSender(this->name);
        request.pushParam(target);

        std::stringbuf commandBuf;
        request.serialize(commandBuf);

        size_t bufSize = commandBuf.str().size();
        if(bufSize <= MAX_MSG_SIZE)
        {
            mq->send(commandBuf.str().c_str(), bufSize, PRIORITY_COMMAND);
        }
        else
        {
            std::cerr << "[ERROR] Client::IsClientConnected: command too long\n";
            return false;
            // TODO: implement spliting messages
        }




        return false;
    }

    bool Client::Peek(EventMessage &msg)
    {
        message_queue* const mq = (message_queue*)this->privateQueue;

        // check local deque for message
        if(!this->storedMessages.empty())
        {
            // get first queued element and pop it
            msg = this->storedMessages.front();
            this->storedMessages.pop_front();
            return true;
        }

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
