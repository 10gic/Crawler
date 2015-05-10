/**
* @file MyFile.cpp
* @brief ʵ����������(MyFile,Html,robots)
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
//ʹ��C++ TR1�е�������ʽ�⣬VS��TR1��֧�ִ�VS2008 SP1��ʼ��
//TR1������ECMAScript�﷨������֧��lookbehind assertion�������ӣ���
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
    if (u->isValid() && filter1(u->getHost(), u->getFile()) && u->getDepth() >= 0) //u->getDepth() >=0����������ȡ�
    {
        check(u);
    }
    else
    {
        delete u;
    }
}

//������Ӧ���ĵ�һ�С�
int Html::parseCmdline()
{
    respondCode = (buffer[11] - '0') + (buffer[10] - '0') * 10 + (buffer[9] - '0') * 100;

    //switch(buffer[9] - '0')
    //{
    //case '2'://��Ӧ״̬��Ϊ2��ͷ
    //	state = HEADERS;
    //	break;
    //case '3'://��Ӧ״̬��Ϊ3��ͷ
    //	state = HEADERS30X;
    //	break;
    //default:
    //	return 1;
    //}
    return 0;
}

/*����HTTP��Ӧͷ��
*����0Ϊ����������1������Ҫ��������ļ���
*/
int Html::parseHeader()
{
    verifLength();
    if (verifType())//verifTypeʵ���˴󲿷ַ�������ͷ�Ĺ��ܡ�
    {
        cerr << "�������ļ���" << this->getUrl()->getHost() << ":" << this->getUrl()->getPort()
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

/**����HTTP��Ӧͷ��
* ����0��ʾ���������أ�
* ����1��ʾ�����ݲ�����Ȥ���������ء�
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

//����0Ϊ��ҳ������1Ϊ�����ļ�����jpeg�ȵȣ���
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

//����lenthOfContent��lenthOfHTTPHeader��ֵ��
//������-1��
int Html::verifLength()
{
    //����Ӧ����ͷ�в���Content-Length��
    const char *p = strstr(buffer, "\r\n\r\n");
    if (NULL == p) //�������Ӧ������û�з���"\r\n\r\n"��
    {
        cerr << "error: HTTP��Ӧ������û�з���\"\\r\\n\\r\\n\"������������̫ӵ����û�յ���" << endl;
        return -1;
    }

    char * area = new char[p - buffer + 1];
    memcpy(area, buffer, p - buffer);
    area[p - buffer] = '\0';//���һλ��0��

    int i = 0;
    while (area[i] != 0)
    {
        if (area[i] >= 'A' && area[i] <= 'Z')
        {
            area[i] = (area[i] | 32); //��ʹ��дת��ΪСд
        }
        i++;
    }
    //�����ares��Ĵ�д��ĸ��ת��ΪСд��ĸ��strstr()��Сд���У�����Content-Length��content-ength��Content-length������������ҵ��ˡ�

    const char *p1 = strstr(area, "content-length: ");

    if (NULL == p1)
    {
        //����������ҳ����ҳʱ��������Ӧ������Ͳ���Content-Length:�ֶΡ�
        addlog("�����˲�����content-length:�ֶε�HTTP��Ӧͷ��");
        addlog(this->getUrl()->getHost());
        addlog(this->getUrl()->getFile());
        delete area;
        return -1;
    }

    int contenLen = 0;
    p1 = p1 + 16; //"Content-Length: "��16���ֽڣ�ð�ź��и��ո񣩣�����ȥ��
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

/*��֤�ǲ�����Ҫ���ص��ļ�������������isInterestingΪtrue��ע�⣬isInteresting������ʶ�����ļ�����Ϊ��ҳ�϶�����Ȥ���ǲ��Ǹ���Ȥ�����ص�����ΪHTML�ļ�ʱ��isInteresting������û�����ã��ڹ��캯����������Ϊfalse�ģ���
*����0��ʾ����Ҫ���أ�����1��ʾ����Ҫ���ء�
*/
int Html::verifType()
{
    const char *p = strstr(buffer, "\r\n\r\n");
    if (NULL == p) //�������Ӧ������û�з���"\r\n\r\n"��
    {
        cerr << "error: HTTP��Ӧ������û�з���\"\\r\\n\\r\\n\"������������̫ӵ����û�յ���" << endl;
        return -1;
    }

    char * area = new char[p - buffer + 1];
    memcpy(area, buffer, p - buffer);
    area[p - buffer] = '\0';//���һλ��0��

    int i = 0;
    while (area[i] != 0)
    {
        if (area[i] >= 'A' && area[i] <= 'Z')
        {
            area[i] = (area[i] | 32); //�Ѵ�дת��ΪСд
        }
        i++;
    }

    const char *p1 = strstr(area, "content-type: ");  //����area��ת������Сд������strstr()�ڶ�������ҲҪΪСд��

    if (NULL == p1)
    {
        addlog("�����˲�����content-type:�ֶε�HTTP��Ӧͷ��"); //������content-type:�ֶε�����ܶ࣬�緵��״̬��Ϊ301ʱ��
        addlog(this->getUrl()->getHost());
        addlog(this->getUrl()->getFile());
        delete area;
        return -1;
    }

    if (!startWithIgnoreCase("text/html", p1 + 14))
    {
        //���ڷ���ҳ���з�����
        //TODO
        if (1)
        {
            //�������Ȥ��
            isInteresting = true;
        }
        else
        {
            //���������Ȥ��
            return 1;
        }
    }

    return 0;
}

//��������������ҵ��������Ӽ�������С�
//ע�⣺������������ʽ��֪����ֻ��ƥ���ǩhref��src���URL,�����URL������˫���Ż������ڡ�
int Html::parseHtml()
{
    set<string> UrlFinded;  //ʹ��insert�������Ԫ��ʱ����������Ԫ������UrlFinded�У���UrlFinded���治�䣬�������ܶ�URL����ȥ�ء��������ȥ��ֻ�Ƕ�ͬһ��ҳ����ĳ�����ȥ�أ���������ҳ�����ͬһ��URL�����������ӵ��������ʱ��ȥ�ػ��Ǳ���ģ�

    const char *page = getPage();
    const char* expr = "(?:\\bhref\\b|\\bsrc\\b)\\s*=\\s*[\"\']([^\"\'#>]+)[\"\']";//���������ģ�������֮��ģ���Ϊ�����Ӱ�쵽����ı�ţ�����ĳ�������Ϊmatch[1]����ΪURL��
    //������ʽ˵������б��\��ʾת�塣
    //\bƥ�䵥�ʵĿ�ʼ�ͽ�����\sƥ��ո�
    //()��ʾ���飬(?:exp)��ʾ���������ȡ����ţ�����������ʽ���б�ŵķ����ֻ��һ���ˣ�([^\"\'#>]+)����������ϣ���õ���URL��
    //������������ʽ��֪����ֻ��ƥ���ǩhref��src���URL,�����URL������˫���Ż������ڡ�

    try
    {
        regex rgx(expr, regex::icase);  //���Դ�Сд��
        cmatch match;   //����ƥ������match[0]��ʾ����ƥ�䣬match[1]��ʾ��һ������ƥ�������(������Ҫ��URL)��
        match_flag_type flags = match_default;

        const char* first = page; //regex_searchֻ�ܱ����һ��ƥ��Ľ�������Բ��ò�����һ��ָ��first���ƶ������ҵ����е�ƥ�䡣
        const char* last = page + strlen(page);

        for (;;)
        {
            if (regex_search(first, last, match, rgx, flags)) //���ƥ��ɹ���regex_search����true��
            {
                //cout<<"Hit at offset "<<(match[1].first - page)<<"  "<<match[1].str()<<'\n';
                first += match.position() + match.length();

                //�ҵ���ƥ�䣬����Ϊ����URL��Ҳ���������URL������û��ϵ������Url���ܴ�������������Ĺ��캯�������������.��ʼ�����URL��������
                UrlFinded.insert(match[1].str());
                //if ('.' != match[1].str().c_str()[0])//����ҵ���URL����.��ʼ��URL����ֱ�Ӽ��롣
                //{
                //	UrlFinded.insert(match[1].str());
                //}
                //else //����ҵ���URL��.��ʼ�����URL���������������ȥ��.���ټ��뵽�����С�
                //{
                //	//match[1].str().c_str()
                //	UrlFinded.insert(match[1].str().c_str()[1]);
                //}
            }
            else
                break;  //ƥ��ʧ�ܣ�����forѭ����
        } //end for(;;)
    }
    catch (exception e)  //���������ʽ���Ϲ淶�����ڴ��ܲ����������
    {
        cerr << e.what() << endl;
    }

    //��������UrlFinded
    set<string>::const_iterator set_it = UrlFinded.begin();
    while (set_it != UrlFinded.end())
    {
        if ('.' != (*set_it).c_str()[0])//����ҵ���URL����.��ʼ��URL����ֱ�Ӽ��롣
        {
            manageUrl(new Url((*set_it).c_str(), here->getDepth() - 1, base));//��URL���캯���У���URL�����˹��ˣ����mailto:��ftp:�ȹ��˵��ˡ�

        }
        else //����ҵ���URL��.��ʼ������֤һ�º����Ƿ����/�����URL����ʱ��./��ʼ����ͨ����֤��ȥ��.����������ŵ�/���ټ��롣
        {
            const char *temp = (*set_it).c_str();
            temp++; //����һλ������.��
            if ('/' == *temp)
            {
                temp++; //�ٺ���һλ������/��
                manageUrl(new Url(temp, here->getDepth() - 1, base));
            }
        }
        ++set_it;
    }
    return 0;
}

char *Html::getPage()
{
    //��contentStart��ֵ��
    char *p = "\r\n\r\n"; //"\r\n\r\n"����Ӧ��������Ӧ���Ŀ�ʼ�ط��ı�־��
    p = strstr(buffer, p); //�ҵ���һ�γ���"\r\n\r\n"��λ�á�
    contentStart = p + 4;

    return contentStart;
}