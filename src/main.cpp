/**
* @file main.cpp
* @brief 主控程序，调度所有代码进行工作。（实际核心程序在Global.cpp里）
* @author 10gic
* @date 2011/5/4
* @version
* @note
* 这个程序移植自larbin。

larbin使用的I/O模型为poll，Windows下没有对应的模型，与之最相似的是select模型（这个模型Windows下有）。
但，这里并没有采用select，而是选择了Windows下最先进的I/O模型——完成端口模型。
完成端口与poll模型的差别是比较大的，如果我们把输入操作分为两个阶段：阶段一：等待数据准备好，阶段二：从内核向进程复制数据。
poll模型只能告诉进程，数据准备好了（socket可写或可读了），还得由自己去调用相应函数（recv或send或read或write等）去写或读。
完成端口模型则把两个阶段都完成后再通知进程。

这个项目于2011年在读研期间完成的。

用法说明：
Crawler -u url
如：
.\Crawler.exe -u www.baidu.com

配置文件说明：
可以在配置文件Crawler.txt(把它放入和Crawler.exe同一目录)中设置UserAgent，限制域名类别，限制爬取文件的后缀。
其格式和实例如下：
###############################################
UserAgent larbin_2.6.3


# limitDomainLevel用来限制域名。
# 0代表完全不限制，爬取所有域名下网址。
# 1代表限制比较宽，爬取相近域名下网址（爬取www.baidu.com时，也会爬取news.baidu.com等等下面网页）。
# 2代表限制最严，只爬取本个域名下网址（默认值）。
limitDomainLevel 1


# 限制爬取文件的后缀（forbiddenExtensions开始，end结束）。
forbiddenExtensions
.jpg .gif
.png
end

*/

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using std::string;

#include <exception>
using std::exception;

#include "Global.h"
#include "utils/Sequencer.h"
#include "fetch/fetchPipe.h"
#include "fetch/FetchOpen.h"
#include "fetch/CheckFinished.h"

#define TIMEOUT_IOCP INFINITE

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI WorkerThread(LPVOID lpCompletionPortID);
DWORD CreateWorkerThreadForCompletion(LPVOID lpCompletion);

int main(int argc, char *argv[])
{
    cout << "Welcome to this web crawler, it is based on Larbin." << endl;
    Global glob(argc, argv);

    //创建工作线程。现在句柄和完成端口还没有绑定在一起。
    CreateWorkerThreadForCompletion(Global::hCompletion);

    for (;;)
    {

        //发送请求。绑定到完成端口。
        sequencer();

        fetchDns();
        fetchOpen();

        checkAll();

        if (checkFinished())
        {
            cout << "All the work are finished, exit." << endl;
            return 0;
        }

    }

}


DWORD CreateWorkerThreadForCompletion(LPVOID lpCompletion)
{
    WSADATA                 wsaData;
    SYSTEM_INFO             systeminfo;
    int nRet;

    //初始化Winsock。
    if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
    {
        cerr << "WSAStartup() failed: " << nRet << endl;
        exit(1);
    }

    GetSystemInfo(&systeminfo);
    for (int i = 0; i < systeminfo.dwNumberOfProcessors - 1; i++) //*2 + 2; i++)
    {
        HANDLE hThread = INVALID_HANDLE_VALUE;
        DWORD dwThreadId = 0;
        hThread = CreateThread(NULL, 0, WorkerThread, lpCompletion, 0, &dwThreadId);
        if (hThread == NULL)
        {
            cerr << "CreateIoCompletionPort() failed to create I/O completion port: " << GetLastError() << endl;
            exit(1);
        }
    }

    return 0;
}


DWORD WINAPI WorkerThread(LPVOID lpCompletionPortID)
{
    HANDLE hCompletion = (HANDLE)lpCompletionPortID; //得到完成端口的对象句柄。
    DWORD dwBytesTrans;
    PPER_SOCKET_DATA pPerSocketData = NULL;
    PPER_IO_DATA pPerIOData = NULL;

    while (TRUE)
    {
        BOOL ret = GetQueuedCompletionStatus(hCompletion,
            &dwBytesTrans,
            (PDWORD_PTR)&pPerSocketData,
            (LPOVERLAPPED *)&pPerIOData,
            TIMEOUT_IOCP);

        if (TRUE == ret)
        {
            //判断套节字是否被对方关闭(既网页是否下载完毕)
            if (dwBytesTrans == 0 &&
                (pPerIOData->OperationType == IORead || pPerIOData->OperationType == IOWrite))
            {
                //addlog("GetQueuedCompletionStatus返回里，传送的数据为0，说明网页已经下载完了！");
                pPerIOData->state = IOFinished;
                //释放单句柄数据。
                try
                {
                    if (pPerSocketData != NULL && pPerSocketData->Socket != INVALID_SOCKET)
                        closesocket(pPerSocketData->Socket);
                    if (pPerSocketData != NULL)
                        HeapFree(GetProcessHeap(), 0, pPerSocketData);
                    /*if(pPerIOData !=NULL)
                        HeapFree(GetProcessHeap(),0,pPerIOData);*/
                }
                catch (exception ex)
                {
                    cerr << ex.what() << endl;
                }

                continue;
            }

            pPerIOData->top += dwBytesTrans;

            //当至少收HTTP响应报文的第一行报文后，才开始分析。(第一行报文就是类似HTTP/1.1 200 OK的报文，
            //这里对长度用了足够的近似值20，如果这里不限制，后面得到的状态码可能出错。)
            if (pPerIOData->top > 20)
            {
                // inputHeaders()返回1，表示对内容没兴趣。文件过滤。
                if (1 == pPerIOData->parser->inputHeaders())
                {
                    pPerIOData->state = IONotInterest;
                    //释放单句柄数据。
                    try
                    {
                        if (pPerSocketData != NULL && pPerSocketData->Socket != INVALID_SOCKET)
                            closesocket(pPerSocketData->Socket);
                        if (pPerSocketData != NULL)
                            HeapFree(GetProcessHeap(), 0, pPerSocketData);
                    }
                    catch (exception ex)
                    {
                        cerr << ex.what() << endl;
                    }
                    continue;
                }
                else //对内容感兴趣。
                {
                    if ((0 != (((Html *)(pPerIOData->parser))->lenthOfHTTPHeader)) &&
                        (pPerIOData->top == ((Html *)(pPerIOData->parser))->lenthOfContent +
                        ((Html *)(pPerIOData->parser))->lenthOfHTTPHeader))
                    {
                        pPerIOData->state = IOFinished;
                        //释放单句柄数据。
                        try
                        {
                            if (pPerSocketData != NULL && pPerSocketData->Socket != INVALID_SOCKET)
                                closesocket(pPerSocketData->Socket);
                            if (pPerSocketData != NULL)
                                HeapFree(GetProcessHeap(), 0, pPerSocketData);
                        }
                        catch (exception ex)
                        {
                            cerr << ex.what() << endl;
                        }
                        continue;
                    }
                }
            }

            //当这次的dwBytesTrans不为0时，可能恰好完成了HTTP报文的传输，也可能还没有完成。
            //到底是哪种情况，可以通过分析得到的HTTP报文头的长度字段来区分。
            //为什么要区别这两种情况？如果不管它有没有完成，再提交一次请求，如果已经完成了，会使dwBytesTrans为0。
            //这样能少请求一次，更快结束。

            //if (pPerIOData->top == getResponseLength(pPerIOData->szMessage))
            //{
            //	pPerIOData->state = IOFinished;
            //	continue;//跳出本次while循环。
            //}

            //这次收到的数据不为0，再次提交请求，查看是否还有数据。
            DWORD nFlags = 0;
            DWORD NumberOfBytesRecvd = 0;

            switch (pPerIOData->OperationType)
            {
            case IORead:
                //防止网页过大而OverFlow，当pPerIOData里缓存区剩下的空间不能够保存一次WSARecv调用，
                //可能得到的最大数据(BufSmall)时，可能会OverFlow。
                if (pPerIOData->top > (BufBig - BufSmall))
                {
                    pPerIOData->state = IOTooBig;
                    //addlog("下载的内容太大，下载被忽略！");

                    //释放单句柄数据。
                    try
                    {
                        if (pPerSocketData != NULL && pPerSocketData->Socket != INVALID_SOCKET)
                            closesocket(pPerSocketData->Socket);
                        if (pPerSocketData != NULL)
                            HeapFree(GetProcessHeap(), 0, pPerSocketData);
                    }
                    catch (exception ex)
                    {
                        cerr << ex.what() << endl;
                    }
                    continue;
                }

                WSABUF recvBuffer;
                recvBuffer.buf = pPerIOData->szMessage + pPerIOData->top; //把接收buf定位到上次接收数据结尾处。
                recvBuffer.len = BufSmall;
                ZeroMemory(&(pPerIOData->overlap), sizeof(OVERLAPPED));

                //完成一个接收请求时，它只意味着一次WSARecv接收完成，一个网页可能要多次WSARecv才能接收完，
                //所以记录下这次数据接收了多少数据后，还得对同一个socket再次调用WSARecv。
                if (SOCKET_ERROR == WSARecv(pPerSocketData->Socket,
                    &recvBuffer,
                    1,
                    &NumberOfBytesRecvd,
                    &nFlags,
                    &pPerIOData->overlap,
                    NULL))
                {
                    int err = WSAGetLastError();
                    if (err != WSA_IO_PENDING)
                    {
                        pPerIOData->state = IOOtherError;
                        cerr << "WSARecv failed: " << err << endl;
                    }
                }

                break;
            case IOWrite: //完成一个发送请求，暂时还没用到。

                break;
            }

        }
        else  //TRUE != ret，说明GetQueuedCompletionStatus有错误发生。
        {
            int nErrCode = GetLastError();
            //释放单句柄数据。
            try
            {
                if (pPerSocketData != NULL && pPerSocketData->Socket != INVALID_SOCKET)
                    closesocket(pPerSocketData->Socket);
                if (pPerSocketData != NULL)
                    HeapFree(GetProcessHeap(), 0, pPerSocketData);
            }
            catch (exception ex)
            {
                cerr << ex.what() << endl;
            }

            if (&(pPerIOData->overlap) != NULL)
            {
                //被对方主动关闭。
                pPerIOData->state = IOCloseByOtherSide;
                cerr << "GetQueuedCompletionStatus返回false！ err: GetLastError():" << nErrCode << endl;
                addlog("GetQueuedCompletionStatus出现错误，可能连接被对方主动关闭！");
            }
            else
            {
                pPerIOData->state = IOOtherError;
                if (nErrCode == WAIT_TIMEOUT)
                {
                    cerr << "GetQueuedCompletionStatus返回false！ err: 调用超时" << endl;
                    addlog("GetQueuedCompletionStatus出现错误，调用超时！");
                }
                else
                {
                    cerr << "GetQueuedCompletionStatus返回false！ err: GetLastError():" << nErrCode << endl;
                }
            }

        }
    }

    return 0;
}


/*
其它一些零散记录


修改了一些我认为larbin不合理的地方。
比如：Interval类用来控制一次处理的url数，是全局的，被放在了site.h中，放在global.h中更合适。我把Interval类写到了单独文件里。

larbin中PersistentFifo的实现用的是非缓存的Unix系统调用read、write（larbin中自己定义buf实现了缓存）。
而我们直接用C++标准库iostream中的read、write

file改名为BaseFile

larbin里分析响应报文，是边接收报文边分析的，现在改为了全部接收完再分析。

larbin怎么分析网页中的超链接的？
html::parseContent里分析网页内容。


fifo000000里的文件数达到宏urlByFile指定的数量后，会创建fifo000001，当fifo000000里的URL都被读取过后，fifo000000会自动删除。
fifowait000000也类似。


什么时候把URL放到文件fifowait000000里（也就是放到URLsDiskWait队列里）？
调用sequencer()时，会把URL从URLsDisk或URLsPriority中放到namedSiteList，
一旦超过namedSiteList的限制，就会把URL放到URLsDiskWait或URLsPriorityWait中。


larbin里的结构体Connexion相当于单IO数据。


备注：
larbin不支持HTTPS，本程序也不支持HTTPS。

限制了下载网页的大小。
完成端口单IO数据里buf的大小限制的。

larbin完成后不会自动退出，会一直等待。
这个程序增加了超时机制，爬虫结束会退出程序。

取消了爬虫过程中添加URL的功能。

可以改进的地方：
HTTP响应报文实现gzip解压，让请求头支持字段Accept-Encoding: gzip。
如果我们在HTTP请求报文加入字段Accept-Encoding: gzip，则Web服务器返回的响应可能是gzip压缩后的报文（www.baidu.com就会这样）。
由于没未实现gzip的解压，所以请求报文里还不能加入Accept-Encoding: gzip字段。
gzip报文压缩的报文里含有C字符串的结束字符\0，所以如果程序里以\0判断响应报文结束的话就要修改相应代码了。

*/