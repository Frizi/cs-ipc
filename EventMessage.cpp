#include "EventMessage.h"
#include <cassert>
#include <iostream>
#include <sstream>
namespace CsIpc
{
    EventMessage::EventMessage()
    {
        this->event = "";
    }
    EventMessage::EventMessage(std::string event)
    {
        this->event = event;
    }

    EventMessage::EventMessage(std::streambuf &sb)
    {
        this->deserialize(sb);
    }

    EventMessage::~EventMessage()
    {
        this->clear();
    }

    void EventMessage::clear()
    {
        // cleanup parameters, they are all private (copied)
        while( !parameters.empty() )
        {
            // ParamType vector empty too early
            assert(!parameterTypes.empty());

            ParamValue param = parameters.back();
            ParamType type = parameterTypes.back();

            parameters.pop_back();
            parameterTypes.pop_back();

            if(type == T_STR)
                delete param.string;
            else if(type == T_WSTR)
                delete param.wstring;
            else if(type == T_DATA)
                delete param.data;
        }
        // ParamType vector may not be empty after freeing, because of exceptions
        parameterTypes.clear();
    }

    void EventMessage::serialize(std::streambuf &sb)
    {
        assert(parameters.size() == parameterTypes.size());
        std::ostream out(&sb);

        // write event name
        size_t size = event.size();
        out.write((char*)&size, sizeof(size_t));
        out.write(event.data(), size);

        // write sender
        size = sender.size();
        out.write((char*)&size, sizeof(size_t));
        out.write(sender.data(), size);

        for(unsigned int i=0; i<parameters.size();i++)
        {
            ParamType type = parameterTypes[i];
            out.put(static_cast<char>(type));
        }
        out.put(static_cast<char>(T_END));
        for(unsigned int i=0; i<parameters.size();i++)
        {
            ParamValue param = parameters[i];
            ParamType type = parameterTypes[i];
            switch(type)
            {
                case T_STR:
                    size = param.string->size();
                    out.write((char*)&size, sizeof(size_t));
                    out.write(param.string->data(), size);
                    break;
                case T_WSTR:
                    size = param.wstring->size();
                    out.write((char*)&size, sizeof(size_t));
                    out.write(reinterpret_cast<const char*>(param.wstring->data()), size*sizeof(wchar_t));
                    break;
                case T_INT:
                    out.write(reinterpret_cast<char*>(&(param.integer)), sizeof(int));
                    break;
                case T_FLOAT:
                    out.write(reinterpret_cast<char*>(&(param.floating)), sizeof(float));
                    break;
                case T_DATA:
                    size = param.data->str().size();
                    out.write((char*)size, sizeof(size_t));
                    out.write(param.data->str().c_str(), size);
                    break;
                default:
                    assert(false);
            }
        }
    }

    void EventMessage::deserialize(std::streambuf &sb)
    {
        this->clear();
        std::istream in(&sb);

        // read event name
        size_t size = 0;
        in.read((char*)&size, sizeof(size_t));
        char* data = new char[size];
        in.read(data,size);
        event.assign(data,size);
        delete[] data;

        // read sender name
        in.read((char*)&size, sizeof(size_t));
        data = new char[size];
        in.read(data,size);
        sender.assign(data,size);
        delete[] data;

        // sign dead pointer
        data = (char*)0xDEADBEEF;

        char type = 0;
        while(in.get(type) && type != T_END)
        {
            parameterTypes.push_back(static_cast<ParamType>(type));
        }

        for(unsigned int i=0; i<parameterTypes.size();i++)
        {
            ParamValue param;
            ParamType type = parameterTypes[i];
            switch(type)
            {
                case T_STR:
                    {
                        in.read((char*)&size, sizeof(size_t));
                        char* data = new char[size];
                        in.read(data,size);
                        std::string* str = new std::string();
                        str->assign(data,size);
                        param.string = str;
                        delete[] data;
                        // sign dead pointer
                        data = (char*)0xDEADBEEF;
                    }
                    break;
                case T_WSTR:
                    {
                        in.read((char*)&size, sizeof(size_t));
                        char* data = new char[size*sizeof(wchar_t)];
                        in.read(data,size*sizeof(wchar_t));
                        std::wstring* wstr = new std::wstring();
                        wstr->assign(reinterpret_cast<wchar_t*>(data),size);
                        param.wstring = wstr;
                        delete[] data;
                        // sign dead pointer
                        data = (char*)0xDEADBEEF;
                    }
                    break;
                case T_INT:
                    in.read(reinterpret_cast<char*>(&(param.integer)), sizeof(int));
                    break;
                case T_FLOAT:
                    in.read(reinterpret_cast<char*>(&(param.floating)), sizeof(float));
                    break;
                case T_DATA:
                    {
                        in.read((char*)&size, sizeof(size_t));
                        char* data = new char[size];
                        in.read(data,size);
                        std::stringbuf* strbuf = new std::stringbuf();
                        strbuf->sputn(data,size);
                        param.data = strbuf;
                        delete[] data;
                        // sign dead pointer
                        data = (char*)0xDEADBEEF;
                    }
                    break;
                default:
                    throw("EventMessage: Wrong parameter type in deserialization");
            }
            parameters.push_back(param);
        }

        assert(parameters.size() == parameterTypes.size());
    }

    int EventMessage::paramCount()
    {
        assert(parameterTypes.size() == parameters.size());
        return parameters.size();
    }

    void EventMessage::pushParam( const int param )
    {
        ParamValue val;
        val.integer = param;
        parameters.push_back(val);
        parameterTypes.push_back(T_INT);
    }

    void EventMessage::pushParam( const std::string param )
    {
        std::string* paramPtr = new std::string(param);
        ParamValue val;
        val.string = paramPtr;
        parameters.push_back(val);
        parameterTypes.push_back(T_STR);
    }

    void EventMessage::pushParam( const std::wstring param )
    {
        std::wstring* paramPtr = new std::wstring(param);
        ParamValue val;
        val.wstring = paramPtr;
        parameters.push_back(val);
        parameterTypes.push_back(T_WSTR);
    }

    void EventMessage::pushParam( const float param )
    {
        ParamValue val;
        val.floating = param;
        parameters.push_back(val);
        parameterTypes.push_back(T_FLOAT);
    }

    void EventMessage::pushParam( void* dataPtr, unsigned int dataSize)
    {
        ParamValue val;

        std::stringbuf* dataBuf = new std::stringbuf;
        dataBuf->sputn(reinterpret_cast<char*>(dataPtr),dataSize);
        val.data = dataBuf;
        parameters.push_back(val);
        parameterTypes.push_back(T_DATA);
    }

    ParamType EventMessage::getParameterType(const unsigned int which)
    {
        if(parameterTypes.size() <= which)
            throw("EventMessage: Attempt to access nonexistent parameter's type");
        return parameterTypes[which];
    }

    int EventMessage::getParamInt(const unsigned int which)
    {
        if(parameters.size() <= which)
            throw("EventMessage: Attempt to access nonexistent parameter");
        if(parameterTypes[which] != T_INT)
            throw("EventMessage: Attempt to retrieve parameter with wrong type");

        return parameters[which].integer;
    }

    std::string EventMessage::getParamString(const unsigned int which)
    {
        if(parameters.size() <= which)
            throw("EventMessage: Attempt to access nonexistent parameter");
        if(parameterTypes[which] != T_STR)
            throw("EventMessage: Attempt to retrieve parameter with wrong type");

        return *(parameters[which].string);
    }

    std::wstring EventMessage::getParamWstring(const unsigned int which)
    {
        if(parameters.size() <= which)
            throw("EventMessage: Attempt to access nonexistent parameter");
        if(parameterTypes[which] != T_WSTR)
            throw("EventMessage: Attempt to retrieve parameter with wrong type");

        return *(parameters[which].wstring);
    }

    float EventMessage::getParamFloat(const unsigned int which)
    {
        if(parameters.size() <= which)
            throw("EventMessage: Attempt to access nonexistent parameter");
        if(parameterTypes[which] != T_FLOAT)
            throw("EventMessage: Attempt to retrieve parameter with wrong type");

        return parameters[which].floating;
    }

    const void* EventMessage::getParamData(const unsigned int which, unsigned int &dataSize)
    {
        if(parameters.size() <= which)
            throw("EventMessage: Attempt to access nonexistent parameter");
        if(parameterTypes[which] != T_DATA)
            throw("EventMessage: Attempt to retrieve parameter with wrong type");

        const void* dataptr = parameters[which].data->str().c_str();
        dataSize = parameters[which].data->str().size();
        return dataptr;
    }
}
