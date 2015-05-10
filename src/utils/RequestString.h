/**
* @file RequestString.h
* @brief ���ļ���������RequestString
* @author 10gic
* @date 2011/4/28
* @version
* @note
* Detailed description.
*/

#ifndef STRING_H
#define STRING_H

#include<cassert>

#include "../Types.h"

#define STRING_SIZE 1024

class RequestString
{
public:
    /** @brief Ĭ��ʵ�εĹ��캯�� */
    RequestString(uint size = STRING_SIZE);

    /** @brief �������� */
    ~RequestString();

    void recycle(uint size = STRING_SIZE);

    /** @brief �����ַ���������������ɾ��RequestString����ʱ���Զ�ɾ���� */
    char *getString();

    /** @brief �����ַ��������ص��Ǵ�����һ���µ��ַ�����������ֶ�delete���� */
    char *giveString();


    /** @brief  */
    void addString(const char *s);



private:
    char * chain;
    uint pos;
    uint size;

};

#endif