#ifndef EVENTMESSAGE_H
#define EVENTMESSAGE_H

#include <string>
#include <vector>
#include <streambuf>

namespace CsIpc
{
    typedef enum {
        T_INT,
        T_FLOAT,
        T_STR,
        T_WSTR,
        T_END // for header end
    } ParamType;

    typedef union {
        int integer;
        float floating;
        std::string* string;
        std::wstring* wstring;
    } ParamValue;

    class EventMessage {
    public:
        EventMessage();
        EventMessage( std::string eventType );
        EventMessage(std::streambuf &sb ); // shorthand to deserialize
        ~EventMessage();
        void clear();
        void serialize(std::streambuf &sb);
        void deserialize(std::streambuf &sb);

        void setEventType(std::string event)
        {
            this->event = event;
        }
        void setSender(std::string sender)
        {
            this->sender = sender;
        }

        const std::string getSender()
        {
            return this->sender;
        }
        const std::string getEventType()
        {
            return this->event;
        }

        void pushParam( const int param );
        void pushParam( const std::string param );
        void pushParam( const std::wstring param );
        void pushParam( const float param );

        int          paramCount();
        ParamType    getParameterType(unsigned int which);
        int          getParamInt(unsigned int which);
        std::string  getParamString(unsigned int which);
        std::wstring getParamWstring(unsigned int which);
        float        getParamFloat(unsigned int which);

    protected:
        std::string sender;
        std::string event;
        std::vector<ParamValue> parameters;
        std::vector<ParamType> parameterTypes;
    };
}

#endif // EVENTMESSAGE_H
