/**
* @file Url.cpp
* @brief 本文件里实现了类Url的成员函数。
* @author 10gic
* @date 2011/4/28
* @version 
* @note
* Detailed description.
*/

#include <cstring>
#include <cstdio>

#include "..\Types.h"
#include "Url.h"
#include "Text.h"


//注意调用这个parse时，应保证URL前面没有http://等字样。
/** 
* @brief Url::parse 
* 
* Detailed description.
* @param[in] arg 
*/
void Url::parse(const char *arg)
{
    int fin = 0;
    // Find the end of host name
    while(arg[fin] != '/' && arg[fin] != ':' && arg[fin] != 0)
    {
        fin++;
    }
    if (fin == 0) return;

    // Get host name
    host = new char[fin+1];
    for (int i=0; i <= fin; i++)
    {
        host[i] = lowerCase(arg[i]);
    }
    host[fin] = 0;

    // Get port number
    if (arg[fin] == ':')
    {
        port = 0;
        fin++;
        while(arg[fin] >= '0' && arg[fin] <= '9')
        {
            port = port *10 + arg[fin] - '0';
            fin++;
        }
    }
    else  //这个else也可不写，因为在Url在构造函数里把port默认设为了80。
    {
        port = 80;
    }

    // Get file name
    if (arg[fin] != '/')
    {
        // www.baidu.com => www.baidu.com/
        file = newString("/");
    }
    else
    {
        file = newString(arg + fin);
    }
}

Url::Url(const char *u,int8_t depth, Url *base)
{
    this->depth = depth;
    this->host = NULL;
    this->port = 80;
    this->file = NULL;

    if (startWith("http://", u))
    {
        parse(u + 7);
    }
    else if (base != NULL)
    {
        if (startWith("http:",u))
        {
            parseWithBase(u+5,base);
        }
        else if (isProtocol(u))
        {
            // Other unknown protocols (mailto,ftp,news,file,gopher...)
        }
        else
        {
            parseWithBase(u,base);
        }
    }
}

//构造函数，在giveBase中用到。
Url::Url(char *host,uint port,char *file)
{
    //newUrl();
    
    this->host = host;
    this->port = port;
    this->file = file;
}

//由序列化后的字符串重新构造URL。
Url::Url(char *line)
{
    //
    int i = 0;
    depth = 0;
    while(line[i] >= '0' && line[i] <= '9')
    {
        depth = 10*depth + line[i] - '0';
        i++;
    }

    int deb = ++i;//deb保存着host的开始位置，深度后有个空格，所以deb=++i。
    while(line[i] != ':')
    {
        i++;
    }
    line[i] = 0;
    host = newString(line+deb);
    i++;

    port = 0;
    while(line[i]>='0' && line[i]<= '9')
    {
        port = 10*port + line[i] - '0';
        i++;
    }

    file = newString(line+i);

}


/* Is it a valid url ? */
bool Url::isValid()
{
    if (host == NULL) return false;
    int lh = strlen(host);
    return file!=NULL && lh < maxSiteSize	&& lh + strlen(file) + 18 < maxUrlSize;
}

char *Url::serialize()
{
    static char statstr[maxUrlSize + 40 + maxCookieSize];
    int pos = sprintf(statstr,"%u ",depth); //注意%u后有个空格，这是个约定，因为后面根据它再次构造对象时是按有空格构造的。

    pos += sprintf(statstr+pos,"%s:%u%s",host,port,file);

    statstr[pos] = '\n';
    statstr[pos+1] = 0;
    return statstr;
}

Url *Url::giveBase()
{
    int i = strlen(file);
    assert(file[0] == '/');
    while(file[i] != '/')	{i--;}
    char *newFile = new char[i+2];
    memcpy(newFile,file,i+1);
    newFile[i+1] = 0;
    return new Url(newString(host),port,newFile);
}

//一个简单的哈希算法，返回host name的哈希值（一个位于0—namedSiteListSize之间的uint数）
uint Url::hostHashCode()
{
    uint h = 0;
    uint i = 0;
    while(host[i] != 0)
    {
        h = 37*h + host[i];
        i++;
    }
    return h % namedSiteListSize;
}

//返回整个Url的哈希值
uint Url::hashCode()
{
    uint h = port; //h赋初值为port，这样如果host和file相同，port不同的话也能对应不同的哈希。
    uint i = 0;
    while(host[i] != 0)
    {
        h = 31*h + host[i];
        i++;
    }
    i = 0;
    while(file[i] != 0)
    {
        h = 31*h + file[i];
        i++;
    }
    return h % hashSize;
}


//在URL构造函数中被调用。
//功能：给file，port，host赋值。
void Url::parseWithBase(const char *u,Url *base)
{
    //cat filebase and file	
    if ('/' == u[0])
    {
        file = newString(u);
    }
    else
    {
        uint lenb = strlen(base->file);
        char *tmp = new char[lenb + strlen(u) +1];
        memcpy(tmp,base->file,lenb);
        strcpy(tmp + lenb,u);
        file = tmp;
    }
    //normalize
    host = newString(base->host);
    port = base->port;
}

bool Url::isProtocol(const char *s)
{
    uint i = 0;
    while (isalnum(s[i]))
    {
        i++;
    }
    return ':' == s[i];
}