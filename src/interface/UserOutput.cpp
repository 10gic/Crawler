#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ofstream;

#include <cassert>

#include "UserOutput.h"

#include <direct.h> //新建路径（_mkdir）时会用到。

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

    //更新目录名。
    //uint d = u->hostHashCode() % nbDir;
    //for (int i=2; i<7; i++)
    //{
    //	fileName[endFileName - i] = d % 10 + '0';
    //	d /= 10;
    //}
    
    //设置文件名。
    uint len = endFileName;
    if (80 == p)
        len += sprintf(fileName+endFileName, "%s%s", h, f);
    else
        len += sprintf(fileName+endFileName, "%s_%u%s", h, p, f);

    //确认文件的目录存在。
    bool cont = true;
    while(cont)
    {
        len --;
        while(fileName[len] != '/') len --;
        fileName[len] = 0;
        //如果存在，cont就设为false。
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
                cerr<<"Error while creating dir: "<<fileName<<"  Windows返回错误码："<<GetLastError()<<endl;
                return;
            }
            fileName[len] = '/';
        }
        else
        {
            cont = false;
        }
    }
    if (fileName[len-1] == '/') //如果以/结尾，则加入一个默认文件名。
    {
        strcpy(fileName+len,indexFile);
    }

    if (SPECIFIC == page->state)
    {
        if ( 0 != page->lenthOfContent)
        {
            ofstream wfile;
            qualityFileName(fileName);
            wfile.open(fileName,ofstream::binary); //对于非网页文件应用以二进制模式打开。
            if (!wfile.is_open())
            {
                cerr<<"写入文件失败！ "<<fileName<<endl;
                cerr<<"Windows返回错误码： "<<GetLastError()<<endl;
                return;
            }
            wfile.write(page->getPage(),page->lenthOfContent);
            wfile.close();
            cout<<"已保存文件： "<<fileName<<endl;
        }
        else
        {
            addlog("下载文件（非网页）时，发现了没有Content-Length字段的响应头！");
        }
    }
    else if(HTML == page->state)
    {
        //把网页写入到文件。
        ofstream wfile;
        qualityFileName(fileName);
        wfile.open(fileName); //没有加binary设置文件模式，所以是以文本模式进行IO操作。
        if (!wfile.is_open())
        {
            cerr<<"写入文件失败！ "<<fileName<<endl;
            cerr<<"Windows返回错误码： "<<GetLastError()<<endl;
            return;
        }
        wfile<<page->getPage();
        wfile.close();
        cout<<"Saving file： "<<fileName<<endl;
    }
}

//URL中可能出现Windows下的不能做文件名的字符（如*<>:|?"等），把它们改为下划线。
void qualityFileName(char * name)
{
    assert(name);
    int i = 0;
    while (name[i] != 0)
    {
        if (name[i] == '*' || name[i] == '<' || name[i] == '>' || name[i] == '：' || name[i] == '|' || name[i] == '?')
        {
            name[i] = '_';
        }
        i++;
    }
}