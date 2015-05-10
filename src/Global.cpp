#include <iostream>
using std::cerr;
using std::endl;
using std::ios;

#include <fstream>
using std::ifstream;

#include <cstring>
#ifdef _MSC_VER
#define strcasecmp stricmp  //no strcasecmp in vs, use stricmp.
#define strncasecmp strnicmp
#endif

#include <string>
using std::string;

#include <iterator>
using std::istreambuf_iterator;

#include <cstdlib>
using std::exit;

#include <exception>
using std::exception;

#include "Global.h"
#include "utils/Text.h"
#include "interface/Output.h"

#include "utils/WinGetOpt.h"

#define AppConfigFile "Crawler.txt"
#define fifoFile "fifo"

#ifdef _DEBUG
LogFile Global::gLog("log.txt");
#endif

time_t Global::now;
HashTable *Global::seen;
PersistentFifo *Global::URLsDisk;
SyncFifo<Url>  *Global::URLsPriority;

NamedSite *Global::namedSiteList;
IPSite *Global::IPSiteList;

Fifo<NamedSite> *Global::dnsSites;
Fifo<IPSite> *Global::okSites;

ConstantSizedFifo<Connection> *Global::freeConns;

Connection *Global::connections;

Interval *Global::inter;
int8_t Global::depthInSite;

HANDLE Global::hCompletion;

//Vector<char> *Global::domains;
char *Global::startURLDomain;
int8_t Global::limitDomainLevel = 2;
Vector<char> Global::forbExt;

uint Global::number_conn;
//uint Global::number_DnsCalls = 0;
//uint Global::dnsConn;
int Global::number_IPUrl;


char *Global::userAgent = "larbin_2.6.3";  //" Mozilla / 5.0 (Windows NT 6.1; rv:2.0) Firefox / 3.0";
char *Global::headersCommon;


Global::Global(int argc, char *argv[])
{
    //���������в�����
    const char *uflag = NULL;
    int ch;
    while (1)
    {
        ch = getopt(argc, argv, "hu:");
        if (-1 == ch)
        {
            break;
        }
        switch (ch)
        {
        case 'h':
            printf("Usage: Crawler -u starturl.\n");
            exit(0);
        case 'u':
            //printf("The argument of -u is %s\n", optarg);
            uflag = optarg;
            break;
        case '?':
            //printf("Unknown option: %c\n",(char)optopt);
            printf("Incorrected Or Incompleted Arguments!\n");
            exit(-1);
        }
    }

    string url("http://");
    if (!startWith("http://", uflag))
    {
        url.append(uflag);
        uflag = url.c_str(); //Normalize it.
    }

    if (NULL == uflag)
    {
        cerr << "Start url is not specified." << endl;
        exit(-1);
    }

    const char *configFile = AppConfigFile;
#ifdef RELOAD
    bool reload = true;
#else
    bool reload = false;
#endif

    URLsDisk = new PersistentFifo(reload, fifoFile); //����PersistentFifo���캯���ᴴ�����򿪣��ļ�fifo000000��
    URLsPriority = new SyncFifo < Url > ;


    namedSiteList = new NamedSite[namedSiteListSize]; //�Ƚϴ�����飬������ϣ��
    IPSiteList = new IPSite[IPSiteListSize];  //�Ƚϴ�����飬������ϣ��
    //��ȡwww.baidu.comʱ����õ�mp3.baidu.com��news.baidu.com�ȵ������µĺܶ�URL��
    //��õ�news.baidu.com�µ�URL�󣬻�������ϣֵ��0��namedSiteListSize-1���һ��ֵ����������namedSiteList[��ϣֵ]�С�

    okSites = new Fifo<IPSite>(2000);
    dnsSites = new Fifo<NamedSite>(2000);

    inter = new Interval(ramUrls);

    seen = new HashTable();

    depthInSite = 5;
    number_conn = 3;//20;

    //dnsConn = 4;

    hCompletion = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hCompletion == NULL)
    {
        cerr << "CreateIoCompletionPort() failed to create I/O completion port: " << GetLastError() << endl;
        exit(-1);
    }

    parseFile(configFile);

    Url *u = new Url(uflag, Global::depthInSite, (Url *)NULL);
    if (u->isValid())
    {
        startURLDomain = new char[maxSiteSize];
        int len = (strlen(u->getHost()) < maxSiteSize - 1) ? strlen(u->getHost()) : (maxSiteSize - 1);
        strncpy(startURLDomain, u->getHost(), len);
        startURLDomain[len] = '\0';
        check(u);  //����ʼURL���뵽Global::URLsDisk
    }
    else
    {
        cerr << "The start url is invalid" << endl;
        exit(1);
    }

    //�����ύ����ʱ����ͬ���֡�
    RequestString strtmp;
    strtmp.addString("\r\nUser-Agent: ");
    strtmp.addString(userAgent);
    strtmp.addString("\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    strtmp.addString("\r\nAccept-Language: zh-cn,zh;q=0.5");
    //strtmp.addString("\r\nAccept-Encoding: gzip, deflate");
    //���ѡ����Խ���gzip������������ܷ���gzipѹ�����http���ģ������ڴ�������ʱ��Ӧ�ý�ѹ��
    //��������û��ʵ�ֽ�ѹgzip�������������ﲻ��ָ������gzip
    strtmp.addString("\r\nAccept-Charset: GB2312,utf-8;q=0.7,*;q=0.7");
    strtmp.addString("\r\nKeep-Alive: 115");
    strtmp.addString("\r\nConnection: keep-alive");
    strtmp.addString("\r\n\r\n");
    headersCommon = strtmp.giveString(); //����ͷ����ͬ�Ĳ��ֱ��浽headersCommon�

    freeConns = new ConstantSizedFifo<Connection>(number_conn);
    connections = new Connection[number_conn];
    for (uint i = 0; i < number_conn; i++)
    {
        freeConns->put(connections + i);
    }

    initOutput();

}

Global::~Global()
{
}

void Global::parseFile(const char *file)
{
    char szPath[MAX_PATH];
    GetModuleFileNameA(NULL, szPath, MAX_PATH);  //�õ���������·����

    //����ȥ�������ļ������õ���������Ŀ¼��
    char *p;
    p = szPath;
    while (strchr(p, '\\'))
    {
        p = strchr(p, '\\');
        p++;
    }
    *p = '\0';
    strcat(szPath, file);

    //FILE *fp = fopen(szPath,"r");
    ifstream inFile(szPath, ios::in);
    if (!inFile)
    {
        cerr << "Cannot open configuration file :  "<< endl;
        cerr << szPath << endl;
        cerr << "Skip configuration file. Using default settings." << endl;
        return;
    }

    //�����������ļ������ݶ���string����fileData�С�
    string fileData((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    int flen = fileData.length();
    char *tmp = new char[flen + 1];
    memcpy(tmp, fileData.c_str(), flen + 1);
    inFile.close();

    //����ע���У���ע�ͷ���#��ʼ����ȫ�����ɿո񣨻��з���������
    bool eff = false;
    for (int i = 0; tmp[i] != 0; i++)
    {
        switch (tmp[i])
        {
        case '\n':
            eff = false;
            break;
        case '#':
            eff = true;
            //no break!!!
        default:
            if (eff)
                tmp[i] = ' ';
        }
    }

    char *posParse = tmp;
    char *tok = nextToken(&posParse);
    while (tok != NULL)
    {
        if (!strcasecmp(tok, "UserAgent"))
        {
            userAgent = newString(nextToken(&posParse));
        }
        else if (!strcasecmp(tok, "limitDomainLevel"))
        {
            tok = nextToken(&posParse);
            limitDomainLevel = atoi(tok);
        }
        else if (!strcasecmp(tok, "forbiddenExtensions"))
        {
            manageExt(&posParse);
        }
        else
        {
            cerr << "Error: Bad configuration file : " << tok << endl;
            exit(1);
        }
        tok = nextToken(&posParse);

    }

    delete[] tmp;
}

void Global::manageExt(char **posParse)
{
    char * tok = nextToken(posParse);
    while (tok != NULL && strcasecmp(tok, "end"))
    {
        int len = strlen(tok);
        int i;
        for (i = 0; i < len; i++)
        {
            tok[i] = tolower(tok[i]);
        }
        forbExt.addElement(newString(tok)); //��forbExt����������ļ������õ�ֵ��
        tok = nextToken(posParse);
    }
    if (tok == NULL)
    {
        cerr << "Bad configuration file:  no end to forbiddenExtensions" << endl;
        exit(1);
    }
}


/** put Connection in a coherent state
 */
Connection::Connection()
{
    state = emptyC;
}

/** Destructor : never used : we recycle !!!
 */
Connection::~Connection()
{
    assert(false);
}

void Connection::recycle()
{
    //delete parser;
    try
    {
        if (this->pConnData != NULL)
            HeapFree(GetProcessHeap(), 0, this->pConnData);
    }
    catch (exception ex)
    {
        cerr << ex.what() << endl;
    }
    request.recycle();
}