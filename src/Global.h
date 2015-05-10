/**
* @file Global.h
* @brief �����������ṹ��(Global,Connection)
* @author 10gic
* @date 2011/5/4
* @version 
* @note
* Detailed description.
*/


#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
//���������󣬻��ų�һЩ��̫���õ�API����СWindowsͷ�ļ��Ĵ�С���ӿ�����ٶȣ�ͬʱ�ܽ��winsock.h��winsock2.h�ĳ�ͻ���⡣
#endif
//���û�ж���WIN32_LEAN_AND_MEAN��Windows.h�����winsock.h����������ٰ���winsock2.h�Ļ����ͻ������ض����ͻ��
#include<Windows.h>
#include<Winsock2.h>
#include<time.h>

#include"utils/SyncFifo.h"
#include"utils/PersistentFifo.h"
#include"utils/Url.h"
#include"utils/Fifo.h"
#include"utils/Site.h"
#include"utils/ConstantSizedFifo.h"
#include"utils/RequestString.h"
#include"utils/Interval.h"
#include"utils/Vector.h"
#include"utils/MyFile.h"

#include"utils/Debug.h"  //Debug.h������־����֧�֡�

#include"fetch/Checker.h"
#include"fetch/HashTable.h"
#include"Types.h"

#define addIPUrl()  Global::number_IPUrl++
#define delIPUrl()  Global::number_IPUrl--


#define MAX_BUFF_SIZE       8192


typedef struct //���������
{
    SOCKET Socket;
    sockaddr_in addr; 
    //char state;   //socket״̬��
    //int pos;   //
    //FetchError err;
    //int socketId;
    //int timeout;

    //char buffer[MAX_BUFF_SIZE];
}PER_SOCKET_DATA,*PPER_SOCKET_DATA;

typedef enum  //�������ö��������Ϊ��I/O���ݵ�һ���ֶΣ����㵥I/O���ݵĹ���
{
    IORead,
    IOWrite
}OPERATION_TYPE,*POPERATION_TYPE;

typedef enum  //�������ö��������Ϊ��I/O���ݵ�һ���ֶΣ����㵥I/O���ݵĹ���
{
    IONotSet,
    IOOtherError,
    IOFinished,
    IOCloseByOtherSide,
    IONotInterest,
    IOTooBig	
}OPERATION_STATE,*POPERATION_STATE;


typedef struct       // per-I/O����
{
    OVERLAPPED  overlap;
    char           szMessage[BufBig];
    OPERATION_TYPE OperationType;
    OPERATION_STATE state;
    MyFile *parser;
    int top;//������ָ�롣
}PER_IO_DATA, *PPER_IO_DATA;

struct Connection
{
    char state;   //socket״̬��
    //int pos;   //
    //FetchError err;
    SOCKET socket;
    //int timeout;

    RequestString request;
    //MyFile *parser;

    PPER_IO_DATA pConnData;

    //char buffer[MAX_BUFF_SIZE];

    Connection();
    ~Connection();

    void recycle();
};

struct Global
{
    Global(int argc,char *argv[]);
    ~Global();

#ifdef _DEBUG
    static LogFile gLog;
#endif

    static time_t now;

    /** List of pages allready seen (one bit per page) */
    static HashTable *seen;

    static SyncFifo<Url> *URLsPriority;
    static PersistentFifo *URLsDisk;


    //namedSiteList��IPSiteList��������ϣ����
    static NamedSite *namedSiteList; /**< ͬһվ���Url��namedSiteList����ֵΪNamedSite���飬����ÿ��Ԫ�ض�Ӧһ��վ�� */
    static IPSite *IPSiteList;//IPSiteList������ò��ȡ��ҳ����
    
    static Fifo<NamedSite> *dnsSites;//��û�л�ȡdns��Ϣ�ģ���Ҫ����dns��ȡ��
    static Fifo<IPSite> *okSites;//��ֱ�����ص���ַ��
    
    static Connection *connections;

    static ConstantSizedFifo<Connection> *freeConns;


    static Interval *inter;/**< ����һ�δ����URL�� */

    static int8_t depthInSite;


    static HANDLE hCompletion;/**< ��ɶ˿ڣ���Global���캯���г�ʼ�� */

    static int8_t limitDomainLevel;

    static char *startURLDomain; /**< ���濪ʼURL���ڵ����� */

    static Vector<char> forbExt; /**< ������ȡ���ļ���չ�� */

    /** ��ȡ��ֹ���ļ����� */
    static void manageExt(char **posParse);

    //static void manage();

    static uint number_conn; /**< �������ӵ����� */
    //static uint number_DnsCalls; /**< ��¼���ڷ�����ٸ�Dns���� */
    //static uint dnsConn; /**< ��������ͬʱ����Dns�������������number_DnsCalls<dnsConn���ܷ����µ�Dns���� */
    static int number_IPUrl; /**< IPSites��URL������ */


    static char *userAgent; 

    static char *headersCommon;/**< ��������ͷ�е���ͬ���� */


    static void parseFile(const char *file);/**< ��ȡ�����ļ������� */
};

#endif  //GLOBAL_H