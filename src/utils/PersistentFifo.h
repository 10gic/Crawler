/**
* @file PersistentFifo.h
* @brief
* @author 10gic
* @date 2011/5/4
* @version
* @note
* 用于将优先级较低的urls存储在磁盘，文件名后六位是数字，
* 随着urls的增加而递增，每个文件最多url数由type.h中的urlByFile指定，
* 在从fifo中读出后如果输出文件已经读完，则会删除该文件，
* 在reload时，如果只有一个文件，则会忽略，对于文件名后
* 数字最大的文件作为输入文件（压入队列），最小的作为
* 输出文件（弹出队列），在初始化时，文件名后数字最大
* 的文件会被清空，这样保证url数是urlByFile的位数，可以容易的取得urls数
*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <fstream>
using std::ofstream;
using std::ifstream;


#include "../Types.h"
#include "Url.h"

#define BUF_SIZE 16384

class PersistentFifo
{
public:
    PersistentFifo(bool reload, const char *baseName);
    ~PersistentFifo();
    Url *tryGet();
    void put(Url *obj);



protected:
    uint in, out;

    //number of the file used for reading
    int fin, fout;
    //name of files
    uint fileNameLength;
    char *fileName;
    HANDLE hmtx;

    char buf1[BUF_SIZE];  //buf1为入队列(put)所用缓存区，buf1的内容是待写入Fifo(文件)的。

    uint buf1End;//buf1的有效内容为buf1开始的buf1End个字节。

    //buffer used for readLine
    char buf2[BUF_SIZE];   //buf2为出队列(get，tryGet)所用缓存区，buf2的内容是从Fifo(文件)里读出来的。

    uint buf2Pos, buf2End;//buf2的有效内容为buf2+buf2Pos开始的buf2End-buf2Pos个字节。

    ofstream wfile;
    ifstream rfile;

    void updateRead();

    void updateWrite();

    void makeName(uint nb);

    void writeUrl(const char *s);

    char *readLine();//从Fifo队列中读取一行内容，先从buf2里读，没有再读文件。

    void flushBuf1();//刷新buf1缓存区。

};