#include <cstring>
#include "Text.h" //newString()��
#include "RequestString.h"

RequestString::RequestString(uint size)
{
    chain = new char[size];
    this->size = size;
    pos = 0;
    chain[0] = 0;
}

RequestString::~RequestString()
{
    delete[] chain;
}

void RequestString::recycle(uint size)
{
    if (this->size > size)
    {
        delete[] chain;
        chain = new char[size];
        this->size = size;
    }
    pos = 0;
    chain[0] = 0;
}

char *RequestString::getString()
{
    return chain;
}

char *RequestString::giveString()
{
    return newString(chain);
}


/**
* @brief RequestString::addString
* @param[in] s
* @note
* ���ݵĲ���Ϊ������������ʱ����һ����������������������󵽵�ǰ��Ҫ����
*/
void RequestString::addString(const char *s)
{
    uint len = strlen(s);
    if (size <= pos + len)
    {
        size *= 2;
        if (size <= pos + len) size = pos + len + 1;
        char *tmp = new char[size];
        memcpy(tmp, chain, pos);
        delete[] chain;
        chain = tmp;
    }
    memcpy(chain + pos, s, len);
    pos += len;
    chain[pos] = 0;
}