/**
* @file Url.h
* @brief 本文件里定义了类Url
* @author 10gic
* @date 2011/4/28
* @version 1.0
* @note
* 头文件里只有类的定义，类成员函数的实现在其相应的源文件里。
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
    /** @brief Url的构造函数 */
    Url(const char *u,int8_t depth, Url *base);

    Url(char *host,uint port,char *file);

    Url(char *line);

    /** @brief 保存着Url的host字段对应的IP地址，它在NamedSite类的transfer成员函数中被赋值 */
    struct in_addr addr;

    bool isValid();

    /** @brief 序列化Url对象，方便保存 */
    char *serialize();
 
    /** @brief 得到URL的主机名 */
    inline char *getHost(){return host;}

    /** @brief 得到URL的路径名 */
    inline char *getFile(){return file;}

    /** @brief 得到URL的端口号 */
    inline  uint getPort(){return port;}

    inline int8_t getDepth(){ return depth;}

    Url *giveBase();

    /** @brief 由host name得到一个hash code(就是一个uint数) */
    uint hostHashCode();

    /** @brief 由整个Url得到一个hash code(就是一个uint数) */
    uint hashCode();
    


private:
    char *host; /**< 保存URL的主机名部分 */
    char *file; /**< 保存URL的路径名部分 */
    uint16_t port;  /**< 保存URL的端口号 */
    int8_t depth; /**< 保存URL的深度 */

    /** @brief 分析一个URL，把它的host、port、file等写入到Url的相应数据成员里 */
    void parse(const char *s);

    void parseWithBase(const char * u,Url *base);

    bool isProtocol(const char *s);

};

#endif URL_H