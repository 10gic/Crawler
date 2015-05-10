/*
* 自己实现缓存机制的目的是提高效率。
* read、write用的是C++标准库里的函数，它与Unix系统调用里的read、write有个重要区别就是Unix里
* 的系统调用为非缓存IO，而C++标准库为缓存IO，C++标准库IO应注意流的状态，如当流到达结束
* 符时，先恢复流的状态，才能进行下次的读。
*/

#include <cstring>
#include <cassert>

#include <iostream>
using std::cerr;
using std::endl;

#include "PersistentFifo.h"

#define urlByFile 100000  //文件保存多少个URL后才生成新的文件。


//reload是用来说明是不是接着上次没爬完的继续爬。
//baseName是保存URL的文件的文件名前缀，比如"fifo"或"fifowait"。
PersistentFifo::PersistentFifo(bool reload, const char *baseName)
{
    //文件名由两部分组成，baseName和后面的数字。
    fileNameLength = strlen(baseName) + 6;//设定文件名前缀后可接6个数字，fifo000000、fifo000001等。
    fileName = new char[fileNameLength + 1];//字符数组长度比文件名长度要多1，多的那位用来存放\0。
    strcpy(fileName, baseName);//把数组的前strlen(baseName)位赋值为baseName。
    fileName[fileNameLength] = 0;//把数组的最后一位置为'\0'，和写成fileName[fileNameLength + 1]='\0';效果相同。

    buf1End = 0;

    buf2Pos = 0;//只有成员函数readLine才使buf2Pos增加。
    buf2End = 0;//只有成员函数readLine才使buf2End增加。

    hmtx = CreateMutex(NULL, FALSE, NULL);

    if (reload)
    {
        //TODO
    }
    else//如果不是接着上次的任务。
    {
        //先删除当前目录下的旧文件，fifo000000、fifo000001等。
        //TODO

        fin = 0;
        fout = 0;
        in = 0;
        out = 0;
        makeName(0);
        wfile.open(fileName);//打开文件，如果有内容就清空。
        rfile.open(fileName);
        //打开文件时都没有指定binary方式，所以默认为"文本"方式打开文件。
        //Windows平台下如果以"文本"方式打开一个文件，那么在读字符的时候，系统会把所有的"\r\n"序列转成"\n"载入内存，
        //在写入时把内存里的"\n"转成"\r\n" 保存文件。
        if (!wfile)//如果打开文件失败
        {
            cerr << "unable to creat file: " << fileName << endl;
            exit(1);
        }
        if (!rfile)
        {
            cerr << "unable to open file: " << fileName << endl;
            exit(1);
        }
    }
}

PersistentFifo::~PersistentFifo()
{

}

//把输入参数nb作为文件名后半部分，文件名前半部分由构造函数的baseName参数确定的。
//例如：假定baseName为"fifo"，
//若nb==0，则fileName为fifo000000；
//若nb==241，则fileName为fifo000241；
//若nb=12832，则fileName为fifo012832。
void PersistentFifo::makeName(uint nb)
{
    //下面循环从数组fileName的最后一位开始，根据nb值设置fileName数组的后六位。
    for (uint i = fileNameLength; i > fileNameLength - 6; i--)
    {
        fileName[i - 1] = (nb % 10) + '0';
        nb /= 10;
    }
}


void PersistentFifo::writeUrl(const char *s)
{
    size_t len = strlen(s);
    assert(len < maxUrlSize + 40 + maxCookieSize);
    if (buf1End + len < BUF_SIZE)
    {
        memcpy(buf1 + buf1End, s, len);
        buf1End += len;
    }
    else//当缓存区满时。
    {
        flushBuf1();
        memcpy(buf1 + buf1End, s, len);
        buf1End = len;
    }
}

Url *PersistentFifo::tryGet()
{
    Url *tmp = NULL;
    WaitForSingleObject(hmtx, INFINITE);
    if (in != out)//如果不为空
    {
        char *line = readLine();
        tmp = new Url(line);
        out++;

    }
    ReleaseMutex(hmtx);
    return tmp;
}

void PersistentFifo::put(Url *obj)
{
    WaitForSingleObject(hmtx, INFINITE);
    char *s = obj->serialize();
    writeUrl(s);
    in++;
    updateWrite();
    ReleaseMutex(hmtx);
    delete obj;
}

//读取Fifo队列的一个行记录，先从buf2中读取，。
//直到读取到数据函数才会返回。
char *PersistentFifo::readLine()
{
    if (buf2Pos == buf2End) //如果buf2Pos和buf2End相同，表明buf2里的内容全部读完，则可以把buf2重置。
    {
        buf2Pos = 0; buf2End = 0; buf2[0] = 0;
    }

    char *posn = strchr(buf2 + buf2Pos, '\n'); //找到第一次出现\n的位置。

    while (posn == NULL)
    {
        if (!(buf2End - buf2Pos < maxUrlSize + 40 + maxCookieSize))
        {
            //
        }
        if (buf2Pos * 2 > BUF_SIZE)//当buf2Pos指向位置多于buf2的一半时，把buf2前移，这能保证buf2End不会到达BUF_SIZE位置。
        {
            buf2End -= buf2Pos;
            memmove(buf2, buf2 + buf2Pos, buf2End);
            //上面两行代码可以写成更好理解的形式：
            //memmove(buf,buf+bufPos,bufEnd-bufPos);
            //bufEnd = bufEnd-bufPos;
        }
        int postmp = buf2End;
        bool noRead = true; //bool变量noRead控制while循环的退出。
        while (noRead)
        {
            int nb;
            if (rfile.bad())//判断流的状态，如果不可恢复的错误，则退出程序。
            {
                cerr << "file stream err in function PersistentFifo::readLine" << endl;
                exit(1);
            }
            if (rfile.fail()) //如果流出现了可恢复的错误（包括流遇到文件结束符时），则应该恢复流为正常状态。
            {
                rfile.clear(); //恢复流的状态。
            }

            rfile.read(buf2 + buf2End, BUF_SIZE - 1 - buf2End);//从rfile里读取内容到buf2里，一次读取尽可能多的字节。
            nb = rfile.gcount(); //计算读取了多少个字节。

            switch (nb)
            {
            case 0:
                //如果从rfile里没有读取到内容（到达了文件结束位置符，就在下次调用前恢复流的状态），
                //则刷新buf1，把其中内容写入到文件中。
                flushBuf1();
                break;
            default:
                noRead = false;
                buf2End += nb;
                buf2[buf2End] = 0;
                break;
            }
        }//end while (noRead)
        posn = strchr(buf2 + postmp, '\n');
    }//end while(posn == NULL)
    *posn = 0;
    char *res = buf2 + buf2Pos;
    buf2Pos = posn + 1 - buf2;
    return res;
}

void PersistentFifo::flushBuf1()
{
    wfile.write(buf1, buf1End);
    wfile.flush();
    buf1End = 0;
}

void PersistentFifo::updateRead()
{
    if (out % urlByFile == 0)
    {
        rfile.close(); //关闭全部读完内容的文件。
        makeName(fout); //得到旧文件名。
        remove(fileName); //删除旧文件。
        makeName(++fout); //计算新文件的名字。
        rfile.open(fileName); //打开新文件。
        in -= out;
        out = 0;
        assert(buf2Pos == buf2End);
    }
}

void PersistentFifo::updateWrite()
{
    if (in % urlByFile == 0)
    {
        flushBuf1();
        wfile.close();
        makeName(++fin);
        wfile.open(fileName);
        //
    }
}