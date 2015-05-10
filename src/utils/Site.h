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
* �μ� Fifo<IPSite> *Global::okSites
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

//������ͬһվ���е�Url
class NamedSite
{
public:
    NamedSite();
    ~NamedSite();
    char name[maxSiteSize];//ĳ��վ���host name��
    uint16_t port;	//ĳ��վ��Ķ˿ںš�
    uint16_t number_urls;//�ڴ��б����URL����
    Url *Fifo[maxUrlsBySite];
    uint8_t inFifo;
    uint8_t outFifo;

    void putInFifo(Url *u);
    Url *getInFifo();
    short FifoLength();

    bool isInFifo; //��ʶ��������Ƿ���dnsSites�С�

    char dnsState; //����DNS״̬��

    struct in_addr addr; //����IP��ַ��

    uint ipHashCode;


    void putGenericUrl(Url *u,int limit,bool prio);

    inline void putUrl(Url *u){putGenericUrl(u,15,false);}

    void newQuery(); //��ʼDns����

    void scanQueue();

private:
    void transfer(Url *u);

    void forgetUrl(Url *u);

};

#endif