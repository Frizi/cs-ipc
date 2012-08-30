#ifndef INTERNALS_H_INCLUDED
#define INTERNALS_H_INCLUDED

namespace CsIpc
{
    struct packetData_t
    {
        bool isActive;
        std::streambuf *dataBuf;
        int lastPacket;
        int numPackets;

        packetData_t() : isActive(false), dataBuf(NULL), lastPacket(-1), numPackets(0) {} ;
    };

    struct clientData
    {
        std::string name;
        void* privateQueue;
        std::vector<std::string> regEvts;
        packetData_t packetData;
    };

    void MetaSendMessage(void* queuePtr, EventMessage & msg, unsigned int priority, size_t & emptyPacketSize);
    bool HandlePacket(EventMessage &msg, packetData_t & packetData);
}

#endif // INTERNALS_H_INCLUDED
