#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#define MAX_MSG_SIZE 512

#define HANDSHAKE_MSG "csipc.handshake"
#define DISCONNECT_MSG "csipc.disconnect"
#define REGISTER_MSG "csipc.register"
#define UNREGISTER_MSG "csipc.unregister"
#define CONNECTED_MSG "csipc.isconnected"


#define PRIORITY_HANDSHAKE 4
#define PRIORITY_REGISTER 3
#define PRIORITY_COMMAND 2
#define PRIORITY_STANDARD 1


#endif // DEFINES_H_INCLUDED
