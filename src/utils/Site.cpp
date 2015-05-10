#include "Site.h"
#include <cassert>

#include <iostream>
using std::cerr;
using std::endl;

#include "../Global.h"

#include <Ws2tcpip.h> //使用getaddrinfo需要包含的头文件。

#define BUFSMALL 1024  //一次接收数据的缓存区大小。

//建立socket连接。
static ConnState getFds(Connection *conn,in_addr addr,uint port)
{
    SOCKET s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        cerr<<"socket failed: "<<WSAGetLastError()<<endl;
        WSACleanup();
        exit (-1);
    }

    conn->socket = s;

    sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = addr.s_addr;
    servAddr.sin_port = htons(port);

    if (connect(s,(SOCKADDR *)&servAddr,sizeof(servAddr)) != SOCKET_ERROR)
    {
        //连接成功
        return usedC;
    }
    else
    {
        //连接失败。
        cerr<<"Connect failed!  Windows返回错误码: "<<WSAGetLastError()<<endl;
        return emptyC;
    }
    

}


IPSite::IPSite()
{
    lastAccess = 0;
    isInFifo = false;
}

IPSite::~IPSite()
{
    assert(false);
}

void IPSite::putUrl(Url *u)
{
    tab.put(u);
    addIPUrl();

    if (!isInFifo)
    {
        //TODO
        isInFifo = true;

        Global::okSites->put(this);

    }
}

inline Url *IPSite::getUrl()
{
    Url *u = tab.get();

    Global::namedSiteList[u->hostHashCode()].number_urls--;
    Global::inter->getOne();
    return u;
}


/**
* 得到okSites里的第一个URL。
* 绑定到完成端口。
*/
int IPSite::fetch()
{
    if (tab.isEmpty())
    {
        isInFifo = false;
        return 0;
    }
    else
    {
        Url *u = getUrl();//取出tab里的一个URL。

        //crash(u->getHost() << u->getPort() << u->getFile());

        Connection *conn = Global::freeConns->get();
        
        ConnState res = getFds(conn,u->addr,u->getPort());//建立socket连接。
        if (res == usedC)
        {
            conn->request.addString("GET ");
            conn->request.addString(u->getFile());
            conn->request.addString(" HTTP/1.0\r\nHost:");
            conn->request.addString(u->getHost());
            conn->request.addString(Global::headersCommon);//加入请求头里相同的部分。

            conn->state = res;
            //std::cout<<"发送的请求为："<<endl;
            //std::cout<<conn->request.getString()<<endl;

            int len = strlen(conn->request.getString());
            send(conn->socket,conn->request.getString(),len,0);
            //对send做错误处理！

            PPER_SOCKET_DATA pPerSocketData = NULL;
            pPerSocketData = (PPER_SOCKET_DATA)HeapAlloc(GetProcessHeap(),0,sizeof(PER_SOCKET_DATA));   
            //工作线程中调用GetQueuedCompletionStatus后，若发现本次socket通信已完成，则应把HeapAlloc的空间释放。

            pPerSocketData->Socket = conn->socket;
            CreateIoCompletionPort((HANDLE)pPerSocketData->Socket,Global::hCompletion,(DWORD)pPerSocketData,0);


            PPER_IO_DATA pPerIOData = (PPER_IO_DATA)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(PER_IO_DATA));

            conn->pConnData = pPerIOData; //单IO数据赋给conn。

            conn->pConnData->parser = new Html(u,conn);


            pPerIOData->OperationType = IORead;
            pPerIOData->top = 0;

            WSABUF recvBuffer;
            recvBuffer.buf = pPerIOData->szMessage; 
            recvBuffer.len = BufSmall;  //限制一次只能接收这么多数据。
            DWORD nFlags=0;
            DWORD NumberOfBytesRecvd=0;	

            int rc = WSARecv(conn->socket,
                &recvBuffer, //前面已赋值recvBuffer.buf = pPerIOData->szMessage; 这样调用WSARecv得到的数据在单IO数据里。
                1,
                &NumberOfBytesRecvd,
                &nFlags,
                &pPerIOData->overlap,
                NULL);

            if (rc == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if (err != WSA_IO_PENDING)
                {
                    cerr<<"WSARecv failed: "<<err<<endl;
                }
            }

            if(tab.isEmpty())
            {
                isInFifo = false;
            }
            else
            {
                Global::okSites->put(this);
            }
        }
        else if (res == emptyC)
        {
            cerr<<"不能连接到主机:  "<<u->getHost()<<endl;
        }

    }

    return 0;
}


NamedSite::NamedSite()
{
    name[0] = 0;
    number_urls = 0;

    inFifo = 0;
    outFifo = 0;

    isInFifo = false;

    dnsState = waitDns;
    //
}

//析构函数永不会被调用。
NamedSite::~NamedSite()
{
    assert(false);
}

/* Management of the Fifo */
void NamedSite::putInFifo(Url *u)
{
    Fifo[inFifo] = u;
    inFifo = (inFifo +1) % maxUrlsBySite;
    assert(inFifo != outFifo);
}

Url *NamedSite::getInFifo()
{
    assert (inFifo != outFifo);
    Url *tmp = Fifo[outFifo];
    outFifo = (outFifo + 1) % maxUrlsBySite;
    return tmp;
}

short NamedSite::FifoLength()
{
    return (inFifo + maxUrlsBySite -outFifo) % maxUrlsBySite;
}

void NamedSite::transfer(Url *u)
{
    //TODO

    ipHashCode=0;
    char *s = (char *) &addr;
    for (uint i=0; i<sizeof(struct in_addr); i++) 
    {
        ipHashCode = ipHashCode*31 + s[i];
    }
    ipHashCode %= IPSiteListSize; //保证ipHashCode在0到IPSiteListSize-1之间，这样才能调用Global::IPSiteList[ipHashCode].putUrl(u);

    memcpy(&u->addr,&addr,sizeof(struct in_addr)); //把Dns请求得到的信息复制到u->addr中，利用Dns缓存的结果。
    Global::IPSiteList[ipHashCode].putUrl(u); //在该函数里实现了把u放入okSites里。
}

void NamedSite::forgetUrl(Url *u)
{
    this->number_urls--;
    delete u;
    Global::inter->getOne();
}

void NamedSite::putGenericUrl(Url *u,int limit,bool prio)
{

    //TODO
    
    number_urls++;

    if (waitDns == dnsState
        ||strcmp(name,u->getHost())
        ||this->port != u->getPort()) //如果dns还没请求，或是一个新的域名。
    {
        this->putInFifo(u);
        if (!isInFifo)
        {
            isInFifo = true;
            Global::dnsSites->put(this);
        }
    }
    else switch(dnsState)
    {
    case doneDns:
        transfer(u);  //只有当DNS请求完成后才能调用transfer!!
        break;

        //TODO
    }
    
}

void NamedSite::newQuery()
{
    if (inFifo == outFifo)
        return;
    Url *u = Fifo[outFifo];  //拿出一个URL来请求Dns。
    strcpy(name,u->getHost()); //尝试请求时会把URL的host复制到name数组里。
    port = u->getPort();
    dnsState = waitDns;
    
    char portStr[16];
    itoa(port,portStr,10);


    DWORD dwRetval;

    int i = 1;
    
    struct addrinfo *result = NULL;
    struct addrinfo hints;

    //--------------------------------
    // Setup the hints address info structure
    // which is passed to the getaddrinfo() function
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;  //告诉getaddrinfo，只想得到IPv4的地址。
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

//--------------------------------
// Call getaddrinfo(). If the call succeeds,
// the result variable will hold a linked list
// of addrinfo structures containing response
// information
    dwRetval = getaddrinfo(name, portStr, &hints, &result);
    if ( dwRetval != 0 ) {
        printf("getaddrinfo failed with error: %d\n", dwRetval);
        this->dnsState = errorDns;
        return;
    }

    this->addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr;  	
    //关于getaddrinfo的用法详见MSDN，得到的结果result里可能不止包含一个IP，这里我们只取一个合适的。

    this->dnsState = doneDns;

    freeaddrinfo(result);
}

void NamedSite::scanQueue()
{
    //扫描this->Fifo中的URL，如果发现有已经完成Dns请求的URL，则把它transfer（调用transfer最终会把它放入okSites里）。
    int ss = FifoLength();
    for (int i = 0; i < ss; i++)
    {
        Url *u1 = getInFifo();
        if (!strcmp(name,u1->getHost())) //如果以前尝试过Dns请求。（尝试请求时会把URL的host复制到name数组里。）
        {
            if (doneDns == this->dnsState) //尝试过的请求成功。
            {
                if (u1->getPort() == this->port)
                    transfer(u1);
                else
                    putInFifo(u1);
            }
            else //尝试过的请求失败。
            {
                forgetUrl(u1);
            }
        }
        else //如果还没尝试Dns请求。
        {
            putInFifo(u1);
        }
    }
}