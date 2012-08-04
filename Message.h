#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <cstring>
namespace CsIpc
{
    class Message;

    enum e_poperror
    {
        e_fine = 0,
        e_bad_type,
        e_no_param,
    };

    enum e_type
    {
        t_uninitialized,
        t_string,
        t_int,
        t_float,
        t_double,
    };

    struct Param
    {
        Param()
        {
            this->type = t_uninitialized;
        }

        void initWithStr(const char* str)
        {
            if(this->type == t_string)
            {
                delete[] val_string;
            }

            size_t length = strlen(str);
            char* buffer = new char[length+1]();
            strcpy(buffer, str);
            buffer[length] = 0;

            this->type = t_string;
            this->val_string = buffer;
        }

        ~Param()
        {
            if(this->type == t_string)
            {
                delete[] val_string;
            }
        }

        Param(const Param &copy)
        {
            this->type = copy.type;
            switch(this->type)
            {
                case(t_int):
                    this->val_int = copy.val_int;
                    break;
                case(t_float):
                    this->val_int = copy.val_float;
                    break;
                case(t_double):
                    this->val_int = copy.val_double;
                    break;
                case(t_string):
                    this->initWithStr(copy.val_string);
                    break;
                case(t_uninitialized):
                    break;
            }
        }

        e_type type;
        union
        {
            char* val_string;
            int val_int;
            float val_float;
            double val_double;
        };
    };

    class Message
    {
        public:
            Message(const char* event);
            Message(const char* serialized, int bufLen, std::string &sender); // deserializer

            virtual ~Message();

        void pushParamInt(int value);
        void pushParamString(const char* value);
        void pushParamFloat(float value);
        void pushParamDouble(double value);

        int popParamInt(e_poperror &err);
        char* popParamStringAlloc(e_poperror &err);
        float popParamFloat(e_poperror &err);
        double popParamDouble(e_poperror &err);

        char* serialize(std::string &sender, int &bufLen);

        protected:
            Param popParam(e_poperror &err);
            void pushParam(Param &param);
            char* eventName;
            std::vector<Param> params;
    };
}


#endif // MESSAGE_H
