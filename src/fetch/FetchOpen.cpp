#include "FetchOpen.h"
#include "../Global.h"


void fetchDns()
{
    //Submit queries
    //larbin中对DNS请求做了限制。这里的DNS请求不是异步的，是阻塞的，对DNS请求没做限制。
    NamedSite *site = Global::dnsSites->tryGet();
    if (NULL == site)
    {
        return;
    }
    else
    {
        site->newQuery();
        site->scanQueue();
    }
}


void fetchOpen()
{

    int cont = 1;

    //当okSites里有元素且freeConns里有连接。
    while (cont && Global::freeConns->isNonEmpty())
    {
        //从可以下载的Url队列(okSites)中获取Url。
        IPSite *s = Global::okSites->tryGet();
        if (s == NULL)
        {
            cont = 0;
        }
        else
        {
            s->fetch();
        }
    }

}