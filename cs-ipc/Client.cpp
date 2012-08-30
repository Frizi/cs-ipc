#include "Client.h"
#include "Server.h"
#include "internals.h"

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
        this->emptyPacketSize = 0; // auto fill

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

        this->packetData = new packetData_t;

        EventMessage handshakeMsg(HANDSHAKE_MSG);
        handshakeMsg.pushParam(this->name);
        this->Send(handshakeMsg, PRIORITY_HANDSHAKE);
    }

    Client::~Client()
    {
        EventMessage regMsg(DISCONNECT_MSG);
        this->Send(regMsg, PRIORITY_STANDARD);
        delete this->packetData;

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
        msg.setSender(this->name);
        MetaSendMessage(this->publicQueue, msg, priority, this->emptyPacketSize);
    }

    void Client::SendTo(const std::string target, EventMessage &msg)
    {
        msg.setSender(this->name);
        std::stringbuf msgBuffer;
        msg.serialize(msgBuffer);

        EventMessage targetPacket;
        targetPacket.setEventType(DIRECTSEND_MSG);
        targetPacket.pushParam(target);
        targetPacket.pushParam(msgBuffer.str());
        targetPacket.setSender(this->name);

        this->Send(targetPacket, PRIORITY_COMMAND);
    }

    bool Client::WaitForEvent(EventMessage &msg, std::string eventType, unsigned int timeout)
    {
        using namespace boost::posix_time;

        message_queue* const mq = (message_queue*)this->privateQueue;

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
        EventMessage msg;
        msg.setEventType(ISCONNECTED_MSG);
        msg.pushParam(target);

        this->Send(msg, PRIORITY_COMMAND);
        this->WaitForEvent(msg, ISCONNECTED_RESP);

        return ( msg.getParamInt(0) != 0 );
    }

    size_t Client::ClientsRegistered(std::string eventType)
    {
        EventMessage msg;
        msg.setEventType(REGISTERED_MSG);
        msg.pushParam(eventType);

        this->Send(msg, PRIORITY_COMMAND);
        this->WaitForEvent(msg, REGISTERED_RESP);

        return (size_t)msg.getParamInt(0);
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

            return HandleMessage(msg, priority);
        }
        else
            return false;
        return true;
    }

    bool Client::HandleMessage(EventMessage &msg, size_t priority)
    {
        if( // dataPacket
           0 == msg.getEventType().compare(PACKET_MSG) )
        {
            if(HandlePacket(msg, *this->packetData))
                return this->HandleMessage(msg, priority); // handle recieved data
            else
                return this->Peek(msg); // packet discarded, continue peek
        }
        else
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
