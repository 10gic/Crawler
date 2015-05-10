/**
* @file main.cpp
* @brief ���س��򣬵������д�����й�������ʵ�ʺ��ĳ�����Global.cpp�
* @author 10gic
* @date 2011/5/4
* @version
* @note
* ���������ֲ��larbin��

larbinʹ�õ�I/Oģ��Ϊpoll��Windows��û�ж�Ӧ��ģ�ͣ���֮�����Ƶ���selectģ�ͣ����ģ��Windows���У���
�������ﲢû�в���select������ѡ����Windows�����Ƚ���I/Oģ�͡�����ɶ˿�ģ�͡�
��ɶ˿���pollģ�͵Ĳ���ǱȽϴ�ģ�������ǰ����������Ϊ�����׶Σ��׶�һ���ȴ�����׼���ã��׶ζ������ں�����̸������ݡ�
pollģ��ֻ�ܸ��߽��̣�����׼�����ˣ�socket��д��ɶ��ˣ����������Լ�ȥ������Ӧ������recv��send��read��write�ȣ�ȥд�����
��ɶ˿�ģ����������׶ζ���ɺ���֪ͨ���̡�

�����Ŀ��2011���ڶ����ڼ���ɵġ�

�÷�˵����
Crawler -u url
�磺
.\Crawler.exe -u www.baidu.com

�����ļ�˵����
�����������ļ�Crawler.txt(���������Crawler.exeͬһĿ¼)������UserAgent�������������������ȡ�ļ��ĺ�׺��
���ʽ��ʵ�����£�
###############################################
UserAgent larbin_2.6.3


# limitDomainLevel��������������
# 0������ȫ�����ƣ���ȡ������������ַ��
# 1�������ƱȽϿ���ȡ�����������ַ����ȡwww.baidu.comʱ��Ҳ����ȡnews.baidu.com�ȵ�������ҳ����
# 2�����������ϣ�ֻ��ȡ������������ַ��Ĭ��ֵ����
limitDomainLevel 1


# ������ȡ�ļ��ĺ�׺��forbiddenExtensions��ʼ��end��������
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

    //���������̡߳����ھ������ɶ˿ڻ�û�а���һ��
    CreateWorkerThreadForCompletion(Global::hCompletion);

    for (;;)
    {

        //�������󡣰󶨵���ɶ˿ڡ�
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

    //��ʼ��Winsock��
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
    HANDLE hCompletion = (HANDLE)lpCompletionPortID; //�õ���ɶ˿ڵĶ�������
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
            //�ж��׽����Ƿ񱻶Է��ر�(����ҳ�Ƿ��������)
            if (dwBytesTrans == 0 &&
                (pPerIOData->OperationType == IORead || pPerIOData->OperationType == IOWrite))
            {
                //addlog("GetQueuedCompletionStatus��������͵�����Ϊ0��˵����ҳ�Ѿ��������ˣ�");
                pPerIOData->state = IOFinished;
                //�ͷŵ�������ݡ�
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

            //��������HTTP��Ӧ���ĵĵ�һ�б��ĺ󣬲ſ�ʼ������(��һ�б��ľ�������HTTP/1.1 200 OK�ı��ģ�
            //����Գ��������㹻�Ľ���ֵ20��������ﲻ���ƣ�����õ���״̬����ܳ���)
            if (pPerIOData->top > 20)
            {
                // inputHeaders()����1����ʾ������û��Ȥ���ļ����ˡ�
                if (1 == pPerIOData->parser->inputHeaders())
                {
                    pPerIOData->state = IONotInterest;
                    //�ͷŵ�������ݡ�
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
                else //�����ݸ���Ȥ��
                {
                    if ((0 != (((Html *)(pPerIOData->parser))->lenthOfHTTPHeader)) &&
                        (pPerIOData->top == ((Html *)(pPerIOData->parser))->lenthOfContent +
                        ((Html *)(pPerIOData->parser))->lenthOfHTTPHeader))
                    {
                        pPerIOData->state = IOFinished;
                        //�ͷŵ�������ݡ�
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

            //����ε�dwBytesTrans��Ϊ0ʱ������ǡ�������HTTP���ĵĴ��䣬Ҳ���ܻ�û����ɡ�
            //�������������������ͨ�������õ���HTTP����ͷ�ĳ����ֶ������֡�
            //ΪʲôҪ��������������������������û����ɣ����ύһ����������Ѿ�����ˣ���ʹdwBytesTransΪ0��
            //������������һ�Σ����������

            //if (pPerIOData->top == getResponseLength(pPerIOData->szMessage))
            //{
            //	pPerIOData->state = IOFinished;
            //	continue;//��������whileѭ����
            //}

            //����յ������ݲ�Ϊ0���ٴ��ύ���󣬲鿴�Ƿ������ݡ�
            DWORD nFlags = 0;
            DWORD NumberOfBytesRecvd = 0;

            switch (pPerIOData->OperationType)
            {
            case IORead:
                //��ֹ��ҳ�����OverFlow����pPerIOData�ﻺ����ʣ�µĿռ䲻�ܹ�����һ��WSARecv���ã�
                //���ܵõ����������(BufSmall)ʱ�����ܻ�OverFlow��
                if (pPerIOData->top > (BufBig - BufSmall))
                {
                    pPerIOData->state = IOTooBig;
                    //addlog("���ص�����̫�����ر����ԣ�");

                    //�ͷŵ�������ݡ�
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
                recvBuffer.buf = pPerIOData->szMessage + pPerIOData->top; //�ѽ���buf��λ���ϴν������ݽ�β����
                recvBuffer.len = BufSmall;
                ZeroMemory(&(pPerIOData->overlap), sizeof(OVERLAPPED));

                //���һ����������ʱ����ֻ��ζ��һ��WSARecv������ɣ�һ����ҳ����Ҫ���WSARecv���ܽ����꣬
                //���Լ�¼��������ݽ����˶������ݺ󣬻��ö�ͬһ��socket�ٴε���WSARecv��
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
            case IOWrite: //���һ������������ʱ��û�õ���

                break;
            }

        }
        else  //TRUE != ret��˵��GetQueuedCompletionStatus�д�������
        {
            int nErrCode = GetLastError();
            //�ͷŵ�������ݡ�
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
                //���Է������رա�
                pPerIOData->state = IOCloseByOtherSide;
                cerr << "GetQueuedCompletionStatus����false�� err: GetLastError():" << nErrCode << endl;
                addlog("GetQueuedCompletionStatus���ִ��󣬿������ӱ��Է������رգ�");
            }
            else
            {
                pPerIOData->state = IOOtherError;
                if (nErrCode == WAIT_TIMEOUT)
                {
                    cerr << "GetQueuedCompletionStatus����false�� err: ���ó�ʱ" << endl;
                    addlog("GetQueuedCompletionStatus���ִ��󣬵��ó�ʱ��");
                }
                else
                {
                    cerr << "GetQueuedCompletionStatus����false�� err: GetLastError():" << nErrCode << endl;
                }
            }

        }
    }

    return 0;
}


/*
����һЩ��ɢ��¼


�޸���һЩ����Ϊlarbin������ĵط���
���磺Interval����������һ�δ����url������ȫ�ֵģ���������site.h�У�����global.h�и����ʡ��Ұ�Interval��д���˵����ļ��

larbin��PersistentFifo��ʵ���õ��Ƿǻ����Unixϵͳ����read��write��larbin���Լ�����bufʵ���˻��棩��
������ֱ����C++��׼��iostream�е�read��write

file����ΪBaseFile

larbin�������Ӧ���ģ��Ǳ߽��ձ��ı߷����ģ����ڸ�Ϊ��ȫ���������ٷ�����

larbin��ô������ҳ�еĳ����ӵģ�
html::parseContent�������ҳ���ݡ�


fifo000000����ļ����ﵽ��urlByFileָ���������󣬻ᴴ��fifo000001����fifo000000���URL������ȡ����fifo000000���Զ�ɾ����
fifowait000000Ҳ���ơ�


ʲôʱ���URL�ŵ��ļ�fifowait000000�Ҳ���Ƿŵ�URLsDiskWait�������
����sequencer()ʱ�����URL��URLsDisk��URLsPriority�зŵ�namedSiteList��
һ������namedSiteList�����ƣ��ͻ��URL�ŵ�URLsDiskWait��URLsPriorityWait�С�


larbin��Ľṹ��Connexion�൱�ڵ�IO���ݡ�


��ע��
larbin��֧��HTTPS��������Ҳ��֧��HTTPS��

������������ҳ�Ĵ�С��
��ɶ˿ڵ�IO������buf�Ĵ�С���Ƶġ�

larbin��ɺ󲻻��Զ��˳�����һֱ�ȴ���
������������˳�ʱ���ƣ�����������˳�����

ȡ����������������URL�Ĺ��ܡ�

���ԸĽ��ĵط���
HTTP��Ӧ����ʵ��gzip��ѹ��������ͷ֧���ֶ�Accept-Encoding: gzip��
���������HTTP�����ļ����ֶ�Accept-Encoding: gzip����Web���������ص���Ӧ������gzipѹ����ı��ģ�www.baidu.com�ͻ���������
����ûδʵ��gzip�Ľ�ѹ�������������ﻹ���ܼ���Accept-Encoding: gzip�ֶΡ�
gzip����ѹ���ı����ﺬ��C�ַ����Ľ����ַ�\0�����������������\0�ж���Ӧ���Ľ����Ļ���Ҫ�޸���Ӧ�����ˡ�

*/