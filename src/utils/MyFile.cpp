/**
* @file MyFile.cpp
* @brief 实现了三个类(MyFile,Html,robots)
* @author 10gic
* @date 2011/5/3
* @version
* @note
* Detailed description.
*/


#include "MyFile.h"
#include "../Global.h"
#include "Text.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <regex>
//使用C++ TR1中的正则表达式库，VS对TR1的支持从VS2008 SP1开始。
//TR1采用了ECMAScript语法，它不支持lookbehind assertion（逆序环视）。
using std::tr1::regex;
using std::tr1::regex_search;
using std::tr1::cmatch;
using namespace std::tr1::regex_constants;

#include <set>
using std::set;

#include <string>
using std::string;

#include <exception>
using std::exception;


MyFile::MyFile(Connection *conn)
{
    buffer = conn->pConnData->szMessage;
    pos = 0;
    //posParse = buffer;
}

MyFile::~MyFile()
{
}


Html::Html(Url *u, Connection *conn) :MyFile(conn)
{
    this->here = u;
    base = here->giveBase();
    state = ANSWER;
    respondCode = 0;

    lenthOfContent = 0;
    lenthOfHTTPHeader = 0;

    isInteresting = false;

    isRobots = false;
}

Html::~Html()
{
    //TODO
}

void Html::manageUrl(Url *u)
{
    if (u->isValid() && filter1(u->getHost(), u->getFile()) && u->getDepth() >= 0) //u->getDepth() >=0限制爬虫深度。
    {
        check(u);
    }
    else
    {
        delete u;
    }
}

//分析响应报文第一行。
int Html::parseCmdline()
{
    respondCode = (buffer[11] - '0') + (buffer[10] - '0') * 10 + (buffer[9] - '0') * 100;

    //switch(buffer[9] - '0')
    //{
    //case '2'://响应状态码为2开头
    //	state = HEADERS;
    //	break;
    //case '3'://响应状态码为3开头
    //	state = HEADERS30X;
    //	break;
    //default:
    //	return 1;
    //}
    return 0;
}

/*分析HTTP响应头。
*返回0为正常，返回1表明不要下载这个文件。
*/
int Html::parseHeader()
{
    verifLength();
    if (verifType())//verifType实现了大部分分析报文头的功能。
    {
        cerr << "忽略了文件：" << this->getUrl()->getHost() << ":" << this->getUrl()->getPort()
            << this->getUrl()->getFile() << endl;
        return 1;
    }
    else
    {
        if (isInteresting)
        {
            state = SPECIFIC;
        }
        else
        {
            state = HTML;
        }
        return 0;
    }
}

/**分析HTTP响应头。
* 返回0表示，继续下载，
* 返回1表示，内容不感兴趣，不再下载。
*/

int Html::inputHeaders()
{
    parseCmdline();
    if (parseHeader())
    {
        return 1;
    }

    return 0;
}

//返回0为网页，返回1为其它文件（如jpeg等等）。
int Html::endInput()
{
    //TODO
    if (SPECIFIC == this->state)
    {
        return 1;
    }
    if (HTML == this->state && (200 == this->respondCode))
    {
        parseHtml();
        return 0;
    }

}

//设置lenthOfContent和lenthOfHTTPHeader的值。
//出错返回-1。
int Html::verifLength()
{
    //在响应报文头中查找Content-Length。
    const char *p = strstr(buffer, "\r\n\r\n");
    if (NULL == p) //如果在响应报文中没有发现"\r\n\r\n"。
    {
        cerr << "error: HTTP响应报文中没有发现\"\\r\\n\\r\\n\"，可能是网络太拥塞还没收到。" << endl;
        return -1;
    }

    char * area = new char[p - buffer + 1];
    memcpy(area, buffer, p - buffer);
    area[p - buffer] = '\0';//最后一位置0。

    int i = 0;
    while (area[i] != 0)
    {
        if (area[i] >= 'A' && area[i] <= 'Z')
        {
            area[i] = (area[i] | 32); //能使大写转换为小写
        }
        i++;
    }
    //上面把ares里的大写字母都转换为小写字母！strstr()大小写敏感，这样Content-Length、content-ength、Content-length各种情况都能找到了。

    const char *p1 = strstr(area, "content-length: ");

    if (NULL == p1)
    {
        //访问新浪主页的网页时，它的响应报文里就不含Content-Length:字段。
        addlog("出现了不包含content-length:字段的HTTP响应头。");
        addlog(this->getUrl()->getHost());
        addlog(this->getUrl()->getFile());
        delete area;
        return -1;
    }

    int contenLen = 0;
    p1 = p1 + 16; //"Content-Length: "共16个字节（冒号后有个空格），跳过去。
    while (*p1 >= '0' && *p1 <= '9')
    {
        contenLen = contenLen * 10 + *p1 - '0';
        p1++;
    }
    this->lenthOfContent = contenLen;
    this->lenthOfHTTPHeader = p - buffer + 4;
    delete area;

    return 0;
}

/*验证是不是想要下载的文件，若是则设置isInteresting为true（注意，isInteresting仅仅标识其它文件（因为网页肯定感兴趣）是不是感兴趣，下载的内容为HTML文件时，isInteresting在这里没有设置，在构造函数里是设置为false的）。
*返回0表示，想要下载；返回1表示，不要下载。
*/
int Html::verifType()
{
    const char *p = strstr(buffer, "\r\n\r\n");
    if (NULL == p) //如果在响应报文中没有发现"\r\n\r\n"。
    {
        cerr << "error: HTTP响应报文中没有发现\"\\r\\n\\r\\n\"，可能是网络太拥塞还没收到。" << endl;
        return -1;
    }

    char * area = new char[p - buffer + 1];
    memcpy(area, buffer, p - buffer);
    area[p - buffer] = '\0';//最后一位置0。

    int i = 0;
    while (area[i] != 0)
    {
        if (area[i] >= 'A' && area[i] <= 'Z')
        {
            area[i] = (area[i] | 32); //把大写转换为小写
        }
        i++;
    }

    const char *p1 = strstr(area, "content-type: ");  //由于area都转换成了小写，所以strstr()第二个参数也要为小写。

    if (NULL == p1)
    {
        addlog("出现了不包含content-type:字段的HTTP响应头。"); //不包含content-type:字段的情况很多，如返回状态码为301时。
        addlog(this->getUrl()->getHost());
        addlog(this->getUrl()->getFile());
        delete area;
        return -1;
    }

    if (!startWithIgnoreCase("text/html", p1 + 14))
    {
        //对于非网页进行分析。
        //TODO
        if (1)
        {
            //如果感兴趣。
            isInteresting = true;
        }
        else
        {
            //如果不感兴趣。
            return 1;
        }
    }

    return 0;
}

//用正则分析！把找到的新链接加入队列中。
//注意：由整个正则表达式可知，它只能匹配标签href或src后的URL,且这个URL必须在双引号或单引号内。
int Html::parseHtml()
{
    set<string> UrlFinded;  //使用insert往里插入元素时，如果插入的元素已在UrlFinded中，则UrlFinded保存不变，这样就能对URL进行去重。（这里的去重只是对同一个页面里的超链接去重，可能两个页面包含同一个URL情况，所以添加到爬虫队列时的去重还是必须的）

    const char *page = getPage();
    const char* expr = "(?:\\bhref\\b|\\bsrc\\b)\\s*=\\s*[\"\']([^\"\'#>]+)[\"\']";//不能随便更改，加括号之类的，因为这可能影响到分组的编号，后面的程序都是认为match[1]分组为URL。
    //正则表达式说明：反斜杠\表示转义。
    //\b匹配单词的开始和结束，\s匹配空格。
    //()表示分组，(?:exp)表示对这个分组取消编号，这样上面表达式中有编号的分组就只有一个了：([^\"\'#>]+)，它保存着希望得到的URL。
    //由整个正则表达式可知，它只能匹配标签href或src后的URL,且这个URL必须在双引号或单引号内。

    try
    {
        regex rgx(expr, regex::icase);  //忽略大小写。
        cmatch match;   //保存匹配结果，match[0]表示整个匹配，match[1]表示第一个分组匹配的内容(我们需要的URL)。
        match_flag_type flags = match_default;

        const char* first = page; //regex_search只能保存第一次匹配的结果，所以不得不设置一个指针first并移动它来找到所有的匹配。
        const char* last = page + strlen(page);

        for (;;)
        {
            if (regex_search(first, last, match, rgx, flags)) //如果匹配成功，regex_search返回true。
            {
                //cout<<"Hit at offset "<<(match[1].first - page)<<"  "<<match[1].str()<<'\n';
                first += match.position() + match.length();

                //找到的匹配，可能为绝对URL，也可能是相对URL，不过没关系，在类Url有能处理这两种情况的构造函数（但它针对以.开始的相对URL做处理）。
                UrlFinded.insert(match[1].str());
                //if ('.' != match[1].str().c_str()[0])//如果找到的URL不以.开始的URL，则直接加入。
                //{
                //	UrlFinded.insert(match[1].str());
                //}
                //else //如果找到的URL以.开始（相对URL有这种情况），则去掉.后再加入到集合中。
                //{
                //	//match[1].str().c_str()
                //	UrlFinded.insert(match[1].str().c_str()[1]);
                //}
            }
            else
                break;  //匹配失败，跳出for循环。
        } //end for(;;)
    }
    catch (exception e)  //如果正则表达式不合规范，会在此能捕获这个错误。
    {
        cerr << e.what() << endl;
    }

    //遍历集合UrlFinded
    set<string>::const_iterator set_it = UrlFinded.begin();
    while (set_it != UrlFinded.end())
    {
        if ('.' != (*set_it).c_str()[0])//如果找到的URL不以.开始的URL，则直接加入。
        {
            manageUrl(new Url((*set_it).c_str(), here->getDepth() - 1, base));//在URL构造函数中，对URL进行了过滤，如把mailto:、ftp:等过滤掉了。

        }
        else //如果找到的URL以.开始，则验证一下后面是否紧跟/（相对URL中有时用./开始），通过验证则去掉.及后面紧跟着的/，再加入。
        {
            const char *temp = (*set_it).c_str();
            temp++; //后移一位，跳过.。
            if ('/' == *temp)
            {
                temp++; //再后移一位，跳过/。
                manageUrl(new Url(temp, here->getDepth() - 1, base));
            }
        }
        ++set_it;
    }
    return 0;
}

char *Html::getPage()
{
    //给contentStart赋值。
    char *p = "\r\n\r\n"; //"\r\n\r\n"是响应报文中响应正文开始地方的标志。
    p = strstr(buffer, p); //找到第一次出现"\r\n\r\n"的位置。
    contentStart = p + 4;

    return contentStart;
}