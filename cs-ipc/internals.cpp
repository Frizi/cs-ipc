#include <boost/interprocess/ipc/message_queue.hpp>
#include "csipc/EventMessage.h"
#include "defines.h"
#include "internals.h"

using namespace boost::interprocess;

namespace CsIpc
{
    void MetaSendMessage(void* queuePtr, EventMessage & msg, unsigned int priority, size_t & emptyPacketSize)
    {
        message_queue* const mq = (message_queue*)queuePtr;

        std::stringbuf msgBuffer;
        msg.serialize(msgBuffer);

        size_t bufSize = msgBuffer.str().size();
        if(bufSize <= MAX_MSG_SIZE)
        {
            mq->send(msgBuffer.str().data(), bufSize, priority);
        }
        else
        {
            // get packet metadata size to know how much data one packet can have
            if(emptyPacketSize == 0)
            {
                std::stringbuf testPacketBuf;
                const std::string testData = "testData";
                EventMessage testPacket;
                testPacket.setSender(msg.getSender());
                testPacket.setEventType(PACKET_MSG);
                testPacket.pushParam(0);
                testPacket.pushParam(0);
                testPacket.pushParam(testData);
                testPacket.serialize(testPacketBuf);
                emptyPacketSize = testPacketBuf.str().size() - testData.size();
            }


            const size_t packetCapacity = MAX_MSG_SIZE - emptyPacketSize;
            char* dataBuf = new char[packetCapacity];
            const size_t numberOfPackets = static_cast<size_t>(
                ceil( static_cast<float>(bufSize) / static_cast<float>(packetCapacity) )
            );

            EventMessage packet;
            packet.setEventType(PACKET_MSG);
            packet.setSender(msg.getSender());

            for(size_t i = 0; i < numberOfPackets; i++)
            {
                size_t dataWritten = msgBuffer.sgetn(dataBuf, packetCapacity);

                packet.clear();
                packet.pushParam(static_cast<int>(i)); // packet num
                packet.pushParam(static_cast<int>(numberOfPackets)); // number of packets in msg
                packet.pushParam(std::string(dataBuf, dataWritten));

                /* debug info
                {
                    packet.setSender(this->name);
                    std::stringbuf sizeTesterBuf;
                    packet.serialize(sizeTesterBuf);
                    std::cout << "[packet] " << i << "/" << numberOfPackets
                    << " size: " << sizeTesterBuf.str().size() << "/" << MAX_MSG_SIZE
                    << " : " << dataWritten << "/" << packetCapacity
                    << std::endl;
                }
                //*/

                MetaSendMessage(queuePtr, packet, priority, emptyPacketSize);
            }
        }
    }

    bool HandlePacket(EventMessage &msg, packetData_t & packetData)
    {
        // arg0: current packet num
        // arg1: num packets
        // arg2: data

        int currentPacket = msg.getParamInt(0);
        int numPackets = msg.getParamInt(1);
        std::string data = msg.getParamString(2);

        if(packetData.isActive == false)
        {
            if(msg.getParamInt(0) == 0)
            {
                packetData.isActive = true;
                packetData.numPackets = numPackets;
                packetData.lastPacket = currentPacket;
                packetData.dataBuf = new std::stringbuf;
                packetData.dataBuf->sputn(data.data(), data.size());
            }
            else // discard packet
                return false;
        }
        else
        {
            if( packetData.lastPacket + 1 == currentPacket && packetData.numPackets == numPackets)
            {
                packetData.dataBuf->sputn(data.data(), data.size());
                packetData.lastPacket = currentPacket;
            }
            else // discard packet
            {
                packetData.isActive = false;
                delete packetData.dataBuf;
                packetData.dataBuf = NULL;
                return false;
            }
        }

        if(currentPacket + 1 == numPackets)
        {
            msg.deserialize(*packetData.dataBuf);

            packetData.isActive = false;
            delete packetData.dataBuf;
            return true;
        }
        return false;
    }
}
