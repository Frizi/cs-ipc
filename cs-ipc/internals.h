#ifndef INTERNALS_H_INCLUDED
#define INTERNALS_H_INCLUDED

namespace CsIpc
{
    void MetaSendMessage(void* queuePtr, EventMessage & msg, unsigned int priority, size_t & emptyPacketSize);
}

#endif // INTERNALS_H_INCLUDED
