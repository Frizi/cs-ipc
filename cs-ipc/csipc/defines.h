#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#define MAX_MSG_SIZE 512

#define MSG_PREFIX "csipc."
#define RESP_PREFIX "resp."

#define HANDSHAKE_MSG   MSG_PREFIX "handshake"
#define DISCONNECT_MSG  MSG_PREFIX "disconnect"
#define REGISTER_MSG    MSG_PREFIX "register"
#define UNREGISTER_MSG  MSG_PREFIX "unregister"
#define ISCONNECTED_MSG MSG_PREFIX "isconnected"
#define DIRECTSEND_MSG  MSG_PREFIX "direct"
#define REGISTERED_MSG  MSG_PREFIX "numregistered"
#define PACKET_MSG      MSG_PREFIX "packet"

#define ISCONNECTED_RESP MSG_PREFIX RESP_PREFIX "isconnected"
#define REGISTERED_RESP  MSG_PREFIX RESP_PREFIX "numregistered"


#define PRIORITY_HANDSHAKE 4
#define PRIORITY_REGISTER 3
#define PRIORITY_COMMAND 2
#define PRIORITY_STANDARD 1


#endif // DEFINES_H_INCLUDED
