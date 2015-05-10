/**
* @file Url.h
* @brief ���ļ��ﶨ������Url
* @author 10gic
* @date 2011/4/28
* @version 1.0
* @note
* ͷ�ļ���ֻ����Ķ��壬���Ա������ʵ��������Ӧ��Դ�ļ��
*/

#ifndef URL_H
#define URL_H

#include <WinSock2.h>
#include <cassert>
#include "../Types.h"

/**
* @class Url
* @brief 
* @note
* Detailed description.
*/
class Url
{
public:
    /** @brief Url�Ĺ��캯�� */
    Url(const char *u,int8_t depth, Url *base);

    Url(char *host,uint port,char *file);

    Url(char *line);

    /** @brief ������Url��host�ֶζ�Ӧ��IP��ַ������NamedSite���transfer��Ա�����б���ֵ */
    struct in_addr addr;

    bool isValid();

    /** @brief ���л�Url���󣬷��㱣�� */
    char *serialize();
 
    /** @brief �õ�URL�������� */
    inline char *getHost(){return host;}

    /** @brief �õ�URL��·���� */
    inline char *getFile(){return file;}

    /** @brief �õ�URL�Ķ˿ں� */
    inline  uint getPort(){return port;}

    inline int8_t getDepth(){ return depth;}

    Url *giveBase();

    /** @brief ��host name�õ�һ��hash code(����һ��uint��) */
    uint hostHashCode();

    /** @brief ������Url�õ�һ��hash code(����һ��uint��) */
    uint hashCode();
    


private:
    char *host; /**< ����URL������������ */
    char *file; /**< ����URL��·�������� */
    uint16_t port;  /**< ����URL�Ķ˿ں� */
    int8_t depth; /**< ����URL����� */

    /** @brief ����һ��URL��������host��port��file��д�뵽Url����Ӧ���ݳ�Ա�� */
    void parse(const char *s);

    void parseWithBase(const char * u,Url *base);

    bool isProtocol(const char *s);

};

#endif URL_H