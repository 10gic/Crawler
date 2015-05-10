#ifndef SITE_H
#define SITE_H

#include <time.h>
#include "Fifo.h"
#include "Url.h"

// define for the state of a connection
enum ConnState {
    emptyC,
    usedC,
};


enum DnsState
{
    waitDns,
    doneDns,
    errorDns
};


/**
* @class IPSite
* @brief 
* @note
* 参见 Fifo<IPSite> *Global::okSites
*/
class IPSite
{
public:
    IPSite();
    ~IPSite();

    Fifo<Url> tab;

    void putUrl(Url *u);

    int fetch();


private:
    time_t lastAccess;
    bool isInFifo;
    Url *getUrl();
};

//保存着同一站点中的Url
class NamedSite
{
public:
    NamedSite();
    ~NamedSite();
    char name[maxSiteSize];//某个站点的host name。
    uint16_t port;	//某个站点的端口号。
    uint16_t number_urls;//内存中保存的URL数，
    Url *Fifo[maxUrlsBySite];
    uint8_t inFifo;
    uint8_t outFifo;

    void putInFifo(Url *u);
    Url *getInFifo();
    short FifoLength();

    bool isInFifo; //标识这个域名是否在dnsSites中。

    char dnsState; //保存DNS状态。

    struct in_addr addr; //保存IP地址。

    uint ipHashCode;


    void putGenericUrl(Url *u,int limit,bool prio);

    inline void putUrl(Url *u){putGenericUrl(u,15,false);}

    void newQuery(); //开始Dns请求。

    void scanQueue();

private:
    void transfer(Url *u);

    void forgetUrl(Url *u);

};

#endif