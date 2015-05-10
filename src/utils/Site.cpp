#include "Site.h"
#include <cassert>

#include <iostream>
using std::cerr;
using std::endl;

#include "../Global.h"

#include <Ws2tcpip.h> //ʹ��getaddrinfo��Ҫ������ͷ�ļ���

#define BUFSMALL 1024  //һ�ν������ݵĻ�������С��

//����socket���ӡ�
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
        //���ӳɹ�
        return usedC;
    }
    else
    {
        //����ʧ�ܡ�
        cerr<<"Connect failed!  Windows���ش�����: "<<WSAGetLastError()<<endl;
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
* �õ�okSites��ĵ�һ��URL��
* �󶨵���ɶ˿ڡ�
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
        Url *u = getUrl();//ȡ��tab���һ��URL��

        //crash(u->getHost() << u->getPort() << u->getFile());

        Connection *conn = Global::freeConns->get();
        
        ConnState res = getFds(conn,u->addr,u->getPort());//����socket���ӡ�
        if (res == usedC)
        {
            conn->request.addString("GET ");
            conn->request.addString(u->getFile());
            conn->request.addString(" HTTP/1.0\r\nHost:");
            conn->request.addString(u->getHost());
            conn->request.addString(Global::headersCommon);//��������ͷ����ͬ�Ĳ��֡�

            conn->state = res;
            //std::cout<<"���͵�����Ϊ��"<<endl;
            //std::cout<<conn->request.getString()<<endl;

            int len = strlen(conn->request.getString());
            send(conn->socket,conn->request.getString(),len,0);
            //��send��������

            PPER_SOCKET_DATA pPerSocketData = NULL;
            pPerSocketData = (PPER_SOCKET_DATA)HeapAlloc(GetProcessHeap(),0,sizeof(PER_SOCKET_DATA));   
            //�����߳��е���GetQueuedCompletionStatus�������ֱ���socketͨ������ɣ���Ӧ��HeapAlloc�Ŀռ��ͷš�

            pPerSocketData->Socket = conn->socket;
            CreateIoCompletionPort((HANDLE)pPerSocketData->Socket,Global::hCompletion,(DWORD)pPerSocketData,0);


            PPER_IO_DATA pPerIOData = (PPER_IO_DATA)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(PER_IO_DATA));

            conn->pConnData = pPerIOData; //��IO���ݸ���conn��

            conn->pConnData->parser = new Html(u,conn);


            pPerIOData->OperationType = IORead;
            pPerIOData->top = 0;

            WSABUF recvBuffer;
            recvBuffer.buf = pPerIOData->szMessage; 
            recvBuffer.len = BufSmall;  //����һ��ֻ�ܽ�����ô�����ݡ�
            DWORD nFlags=0;
            DWORD NumberOfBytesRecvd=0;	

            int rc = WSARecv(conn->socket,
                &recvBuffer, //ǰ���Ѹ�ֵrecvBuffer.buf = pPerIOData->szMessage; ��������WSARecv�õ��������ڵ�IO�����
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
            cerr<<"�������ӵ�����:  "<<u->getHost()<<endl;
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

//�������������ᱻ���á�
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
    ipHashCode %= IPSiteListSize; //��֤ipHashCode��0��IPSiteListSize-1֮�䣬�������ܵ���Global::IPSiteList[ipHashCode].putUrl(u);

    memcpy(&u->addr,&addr,sizeof(struct in_addr)); //��Dns����õ�����Ϣ���Ƶ�u->addr�У�����Dns����Ľ����
    Global::IPSiteList[ipHashCode].putUrl(u); //�ڸú�����ʵ���˰�u����okSites�
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
        ||this->port != u->getPort()) //���dns��û���󣬻���һ���µ�������
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
        transfer(u);  //ֻ�е�DNS������ɺ���ܵ���transfer!!
        break;

        //TODO
    }
    
}

void NamedSite::newQuery()
{
    if (inFifo == outFifo)
        return;
    Url *u = Fifo[outFifo];  //�ó�һ��URL������Dns��
    strcpy(name,u->getHost()); //��������ʱ���URL��host���Ƶ�name�����
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
    hints.ai_family = AF_INET;  //����getaddrinfo��ֻ��õ�IPv4�ĵ�ַ��
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
    //����getaddrinfo���÷����MSDN���õ��Ľ��result����ܲ�ֹ����һ��IP����������ֻȡһ�����ʵġ�

    this->dnsState = doneDns;

    freeaddrinfo(result);
}

void NamedSite::scanQueue()
{
    //ɨ��this->Fifo�е�URL������������Ѿ����Dns�����URL�������transfer������transfer���ջ��������okSites���
    int ss = FifoLength();
    for (int i = 0; i < ss; i++)
    {
        Url *u1 = getInFifo();
        if (!strcmp(name,u1->getHost())) //�����ǰ���Թ�Dns���󡣣���������ʱ���URL��host���Ƶ�name�������
        {
            if (doneDns == this->dnsState) //���Թ�������ɹ���
            {
                if (u1->getPort() == this->port)
                    transfer(u1);
                else
                    putInFifo(u1);
            }
            else //���Թ�������ʧ�ܡ�
            {
                forgetUrl(u1);
            }
        }
        else //�����û����Dns����
        {
            putInFifo(u1);
        }
    }
}