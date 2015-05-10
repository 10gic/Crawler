#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ofstream;

#include <cassert>

#include "UserOutput.h"

#include <direct.h> //�½�·����_mkdir��ʱ���õ���

#include "../Global.h"

#define saveDir "save/"
#define indexFile "index.html"
#define nbDir 1000

static char *fileName;
static uint endFileName;

void qualityFileName(char *);

void initUserOutput()
{
    _mkdir(saveDir);
    endFileName = strlen(saveDir);
    fileName = new char[endFileName + maxUrlSize + 50];
    strcpy(fileName,saveDir);
    if (fileName[endFileName - 1] != '/')
    {
        fileName[endFileName++] = '/';
    }
    strcpy(fileName + endFileName, "d00000/");
    endFileName += 7;
}

void loaded(Html *page)
{
    Url *u = page->getUrl();
    uint p = u->getPort();
    char *h = u->getHost();
    char *f = u->getFile();

    //����Ŀ¼����
    //uint d = u->hostHashCode() % nbDir;
    //for (int i=2; i<7; i++)
    //{
    //	fileName[endFileName - i] = d % 10 + '0';
    //	d /= 10;
    //}
    
    //�����ļ�����
    uint len = endFileName;
    if (80 == p)
        len += sprintf(fileName+endFileName, "%s%s", h, f);
    else
        len += sprintf(fileName+endFileName, "%s_%u%s", h, p, f);

    //ȷ���ļ���Ŀ¼���ڡ�
    bool cont = true;
    while(cont)
    {
        len --;
        while(fileName[len] != '/') len --;
        fileName[len] = 0;
        //������ڣ�cont����Ϊfalse��
        if (INVALID_FILE_ATTRIBUTES != GetFileAttributesA (fileName))
        {
            cont = false;
        }		
        fileName[len] = '/';
    }
    cont = true;
    while(cont)
    {
        len ++;
        while(fileName[len] != '/' && fileName[len] != 0) len++;
        if (fileName[len] == '/')
        {
            fileName[len] = 0;
            qualityFileName(fileName); //
            if(0 != _mkdir(fileName))
            {
                cerr<<"Error while creating dir: "<<fileName<<"  Windows���ش����룺"<<GetLastError()<<endl;
                return;
            }
            fileName[len] = '/';
        }
        else
        {
            cont = false;
        }
    }
    if (fileName[len-1] == '/') //�����/��β�������һ��Ĭ���ļ�����
    {
        strcpy(fileName+len,indexFile);
    }

    if (SPECIFIC == page->state)
    {
        if ( 0 != page->lenthOfContent)
        {
            ofstream wfile;
            qualityFileName(fileName);
            wfile.open(fileName,ofstream::binary); //���ڷ���ҳ�ļ�Ӧ���Զ�����ģʽ�򿪡�
            if (!wfile.is_open())
            {
                cerr<<"д���ļ�ʧ�ܣ� "<<fileName<<endl;
                cerr<<"Windows���ش����룺 "<<GetLastError()<<endl;
                return;
            }
            wfile.write(page->getPage(),page->lenthOfContent);
            wfile.close();
            cout<<"�ѱ����ļ��� "<<fileName<<endl;
        }
        else
        {
            addlog("�����ļ�������ҳ��ʱ��������û��Content-Length�ֶε���Ӧͷ��");
        }
    }
    else if(HTML == page->state)
    {
        //����ҳд�뵽�ļ���
        ofstream wfile;
        qualityFileName(fileName);
        wfile.open(fileName); //û�м�binary�����ļ�ģʽ�����������ı�ģʽ����IO������
        if (!wfile.is_open())
        {
            cerr<<"д���ļ�ʧ�ܣ� "<<fileName<<endl;
            cerr<<"Windows���ش����룺 "<<GetLastError()<<endl;
            return;
        }
        wfile<<page->getPage();
        wfile.close();
        cout<<"Saving file�� "<<fileName<<endl;
    }
}

//URL�п��ܳ���Windows�µĲ������ļ������ַ�����*<>:|?"�ȣ��������Ǹ�Ϊ�»��ߡ�
void qualityFileName(char * name)
{
    assert(name);
    int i = 0;
    while (name[i] != 0)
    {
        if (name[i] == '*' || name[i] == '<' || name[i] == '>' || name[i] == '��' || name[i] == '|' || name[i] == '?')
        {
            name[i] = '_';
        }
        i++;
    }
}