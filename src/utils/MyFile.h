/**
* @file MyFile.h
* @brief �����˳������(File)��������������(Html��robots)��
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
#define SPECIFIC 4  //�����������Ϊһ������Ȥ���ļ�ʱ(��jpeg,gif,js�ȵ�)��

struct Connection;//�ṹ��(��)��ǰ��������ǰ���������ڴ����໥�������ࡣ������Connection��MyFile�໥������


//���Ǹ�������࣬��Ϊ���������������麯����
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

    virtual int inputHeaders() = 0;//���麯��
    virtual int endInput() = 0;//���麯��
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

    //��Ӧ���ĵĳ��ȡ�����HTTTP��Ӧͷ�е�Content-Length:�ֶζ�Ӧ��ֵ��
    //ע�⣺��Щ��վ(��������ҳ)���ص���Ӧ���Ŀ��ܲ�����Content-Length:�ֶΡ�
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

    int parseCmdline();//������Ӧ���ĵ�һ�С�

    int parseHeader();

    int verifLength();//����lenthOfContent��lenthOfHTTPHeader��ֵ��
    int verifType();//����isInteresting��ֵ��

    int parseHtml();

};

#endif