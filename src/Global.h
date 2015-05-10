/**
* @file Global.h
* @brief 定义了两个结构体(Global,Connection)
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
//定义这个宏后，会排除一些不太常用的API，减小Windows头文件的大小，加快编译速度，同时能解决winsock.h和winsock2.h的冲突问题。
#endif
//如果没有定义WIN32_LEAN_AND_MEAN，Windows.h会包含winsock.h，程序后面再包含winsock2.h的话，就会有有重定义冲突。
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

#include"utils/Debug.h"  //Debug.h里有日志调试支持。

#include"fetch/Checker.h"
#include"fetch/HashTable.h"
#include"Types.h"

#define addIPUrl()  Global::number_IPUrl++
#define delIPUrl()  Global::number_IPUrl--


#define MAX_BUFF_SIZE       8192


typedef struct //单句柄数据
{
    SOCKET Socket;
    sockaddr_in addr; 
    //char state;   //socket状态。
    //int pos;   //
    //FetchError err;
    //int socketId;
    //int timeout;

    //char buffer[MAX_BUFF_SIZE];
}PER_SOCKET_DATA,*PPER_SOCKET_DATA;

typedef enum  //定义这个枚举类型作为单I/O数据的一个字段，方便单I/O数据的管理。
{
    IORead,
    IOWrite
}OPERATION_TYPE,*POPERATION_TYPE;

typedef enum  //定义这个枚举类型作为单I/O数据的一个字段，方便单I/O数据的管理。
{
    IONotSet,
    IOOtherError,
    IOFinished,
    IOCloseByOtherSide,
    IONotInterest,
    IOTooBig	
}OPERATION_STATE,*POPERATION_STATE;


typedef struct       // per-I/O数据
{
    OVERLAPPED  overlap;
    char           szMessage[BufBig];
    OPERATION_TYPE OperationType;
    OPERATION_STATE state;
    MyFile *parser;
    int top;//缓存区指针。
}PER_IO_DATA, *PPER_IO_DATA;

struct Connection
{
    char state;   //socket状态。
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


    //namedSiteList和IPSiteList是两个哈希表！！
    static NamedSite *namedSiteList; /**< 同一站点的Url，namedSiteList被赋值为NamedSite数组，数组每个元素对应一个站点 */
    static IPSite *IPSiteList;//IPSiteList控制礼貌爬取网页！！
    
    static Fifo<NamedSite> *dnsSites;//还没有获取dns信息的，需要进行dns获取。
    static Fifo<IPSite> *okSites;//可直接下载的网址。
    
    static Connection *connections;

    static ConstantSizedFifo<Connection> *freeConns;


    static Interval *inter;/**< 控制一次处理的URL数 */

    static int8_t depthInSite;


    static HANDLE hCompletion;/**< 完成端口，在Global构造函数中初始化 */

    static int8_t limitDomainLevel;

    static char *startURLDomain; /**< 保存开始URL所在的域名 */

    static Vector<char> forbExt; /**< 限制爬取的文件扩展名 */

    /** 读取禁止的文件类型 */
    static void manageExt(char **posParse);

    //static void manage();

    static uint number_conn; /**< 并行连接的数量 */
    //static uint number_DnsCalls; /**< 记录正在发起多少个Dns请求 */
    //static uint dnsConn; /**< 用来限制同时发起Dns请求的数量，当number_DnsCalls<dnsConn才能发起新的Dns请求 */
    static int number_IPUrl; /**< IPSites中URL的数量 */


    static char *userAgent; 

    static char *headersCommon;/**< 保存请求头中的相同部分 */


    static void parseFile(const char *file);/**< 读取配置文件里内容 */
};

#endif  //GLOBAL_H