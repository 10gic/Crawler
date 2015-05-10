/**
* @file PersistentFifo.h
* @brief
* @author 10gic
* @date 2011/5/4
* @version
* @note
* ���ڽ����ȼ��ϵ͵�urls�洢�ڴ��̣��ļ�������λ�����֣�
* ����urls�����Ӷ�������ÿ���ļ����url����type.h�е�urlByFileָ����
* �ڴ�fifo�ж������������ļ��Ѿ����꣬���ɾ�����ļ���
* ��reloadʱ�����ֻ��һ���ļ��������ԣ������ļ�����
* ���������ļ���Ϊ�����ļ���ѹ����У�����С����Ϊ
* ����ļ����������У����ڳ�ʼ��ʱ���ļ������������
* ���ļ��ᱻ��գ�������֤url����urlByFile��λ�����������׵�ȡ��urls��
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

    char buf1[BUF_SIZE];  //buf1Ϊ�����(put)���û�������buf1�������Ǵ�д��Fifo(�ļ�)�ġ�

    uint buf1End;//buf1����Ч����Ϊbuf1��ʼ��buf1End���ֽڡ�

    //buffer used for readLine
    char buf2[BUF_SIZE];   //buf2Ϊ������(get��tryGet)���û�������buf2�������Ǵ�Fifo(�ļ�)��������ġ�

    uint buf2Pos, buf2End;//buf2����Ч����Ϊbuf2+buf2Pos��ʼ��buf2End-buf2Pos���ֽڡ�

    ofstream wfile;
    ifstream rfile;

    void updateRead();

    void updateWrite();

    void makeName(uint nb);

    void writeUrl(const char *s);

    char *readLine();//��Fifo�����ж�ȡһ�����ݣ��ȴ�buf2�����û���ٶ��ļ���

    void flushBuf1();//ˢ��buf1��������

};