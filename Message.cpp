#include "Message.h"

#include <iostream>
#include <string>
#include <cstring>

namespace CsIpc
{
    Message::Message(const char* event)
    {
        size_t nameSize = strlen(event);
        eventName = new char[nameSize+1];
        strcpy(eventName, event);
    }


    char* serialize(std::streambuf &streambuf, std::string &sender)
    {
        std::ostream buffer;
    }

    Message(std::streambuf &streambuf, std::string &sender)
    { // deserialize

    }


    Message::~Message()
    {
        delete[] eventName;
    }


    void Message::pushParamInt(int value)
    {
        Param param;
        param.type = t_int;
        param.val_int = value;
        this->pushParam(param);
    }

    void Message::pushParamFloat(float value)
    {
        Param param;
        param.type = t_float;
        param.val_float = value;
        this->pushParam(param);
    }

    void Message::pushParamDouble(double value)
    {
        Param param;
        param.type = t_double;
        param.val_double = value;
        this->pushParam(param);
    }

    void Message::pushParamString(const char* value)
    {
        Param param;
        param.initWithStr(value);
        this->pushParam(param);
    }

    void Message::pushParam(Param &param)
    {
        this->params.push_back(param);
    }

    int Message::popParamInt(e_poperror &err)
    {
        Param param = popParam(err);
        if(param.type != t_int)
        {
            err = e_bad_type;
            return -1;
        }
        return param.val_int;
    }

    float Message::popParamFloat(e_poperror &err)
    {
        Param param = popParam(err);
        if(param.type != t_float)
        {
            err = e_bad_type;
            return -1.0f;
        }
        return param.val_float;
    }

    double Message::popParamDouble(e_poperror &err)
    {
        Param param = popParam(err);
        if(param.type != t_double)
        {
            err = e_bad_type;
            return -1.0;
        }
        return param.val_double;
    }

    char* Message::popParamStringAlloc(e_poperror &err)
    {
        Param param = popParam(err);
        if(param.type != t_string)
        {
            err = e_bad_type;
            return NULL;
        }
        size_t len = strlen(param.val_string);
        char* buffer = new char[len+1]();
        strcpy(buffer, param.val_string);
        buffer[len] = 0;
        return buffer;
    }

    Param Message::popParam(e_poperror &err)
    {
        Param ret = this->params.back();
        if(this->params.size() <= 0)
        {
            err = e_no_param;
            return ret;
        }
        err = e_fine;

        this->params.pop_back();
        return ret;
    }
}
