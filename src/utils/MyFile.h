/**
* @file MyFile.h
* @brief 定义了抽象基类(File)和它的两个子类(Html和robots)。
* @author 10gic
* @date 2011/5/3
* @version
* @note
* Detailed description.
*/

#ifndef MYFILE_H
#define MYFILE_H

#include"../Types.h"

#include"Url.h"

#define ANSWER 0
//#define HEADERS 1
//#define HEADERS30X 2
#define HTML 3
#define SPECIFIC 4  //当请求的内容为一个感兴趣的文件时(如jpeg,gif,js等等)。

struct Connection;//结构体(类)的前向声明，前向声明用于处理相互依赖的类。（这里Connection和MyFile相互依赖）


//这是个抽象基类，因为它包含了两个纯虚函数。
class MyFile
{
protected:
    char *buffer;
    //char *posParse;

public:
    MyFile(Connection *conn);

    virtual ~MyFile();

    bool isRobots;

    uint pos;

    virtual int inputHeaders() = 0;//纯虚函数
    virtual int endInput() = 0;//纯虚函数
};


class Html :public MyFile
{
public:
    Html(Url *u, Connection *conn);
    ~Html();

    int inputHeaders();
    int endInput();

    // State of our read : answer, headers, tag, html...
    int state;

    int respondCode;

    //响应报文的长度。它是HTTTP响应头中的Content-Length:字段对应的值。
    //注意：有些网站(如新浪网页)返回的响应报文可能不含有Content-Length:字段。
    int lenthOfContent;

    int lenthOfHTTPHeader;

    inline Url *getUrl(){ return here; }

    bool isInteresting;

    char *getPage();


private:
    Url *here;
    char *area;
    char *contentStart;//begining of the real content (end of the headers + 1)
    Url *base;
    void manageUrl(Url *u);

    int parseCmdline();//分析响应报文第一行。

    int parseHeader();

    int verifLength();//设置lenthOfContent和lenthOfHTTPHeader的值。
    int verifType();//设置isInteresting的值。

    int parseHtml();

};

#endif