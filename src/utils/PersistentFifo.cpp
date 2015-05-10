/*
* �Լ�ʵ�ֻ�����Ƶ�Ŀ�������Ч�ʡ�
* read��write�õ���C++��׼����ĺ���������Unixϵͳ�������read��write�и���Ҫ�������Unix��
* ��ϵͳ����Ϊ�ǻ���IO����C++��׼��Ϊ����IO��C++��׼��IOӦע������״̬���統���������
* ��ʱ���Ȼָ�����״̬�����ܽ����´εĶ���
*/

#include <cstring>
#include <cassert>

#include <iostream>
using std::cerr;
using std::endl;

#include "PersistentFifo.h"

#define urlByFile 100000  //�ļ�������ٸ�URL��������µ��ļ���


//reload������˵���ǲ��ǽ����ϴ�û����ļ�������
//baseName�Ǳ���URL���ļ����ļ���ǰ׺������"fifo"��"fifowait"��
PersistentFifo::PersistentFifo(bool reload, const char *baseName)
{
    //�ļ�������������ɣ�baseName�ͺ�������֡�
    fileNameLength = strlen(baseName) + 6;//�趨�ļ���ǰ׺��ɽ�6�����֣�fifo000000��fifo000001�ȡ�
    fileName = new char[fileNameLength + 1];//�ַ����鳤�ȱ��ļ�������Ҫ��1�������λ�������\0��
    strcpy(fileName, baseName);//�������ǰstrlen(baseName)λ��ֵΪbaseName��
    fileName[fileNameLength] = 0;//����������һλ��Ϊ'\0'����д��fileName[fileNameLength + 1]='\0';Ч����ͬ��

    buf1End = 0;

    buf2Pos = 0;//ֻ�г�Ա����readLine��ʹbuf2Pos���ӡ�
    buf2End = 0;//ֻ�г�Ա����readLine��ʹbuf2End���ӡ�

    hmtx = CreateMutex(NULL, FALSE, NULL);

    if (reload)
    {
        //TODO
    }
    else//������ǽ����ϴε�����
    {
        //��ɾ����ǰĿ¼�µľ��ļ���fifo000000��fifo000001�ȡ�
        //TODO

        fin = 0;
        fout = 0;
        in = 0;
        out = 0;
        makeName(0);
        wfile.open(fileName);//���ļ�����������ݾ���ա�
        rfile.open(fileName);
        //���ļ�ʱ��û��ָ��binary��ʽ������Ĭ��Ϊ"�ı�"��ʽ���ļ���
        //Windowsƽ̨�������"�ı�"��ʽ��һ���ļ�����ô�ڶ��ַ���ʱ��ϵͳ������е�"\r\n"����ת��"\n"�����ڴ棬
        //��д��ʱ���ڴ����"\n"ת��"\r\n" �����ļ���
        if (!wfile)//������ļ�ʧ��
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

//���������nb��Ϊ�ļ�����벿�֣��ļ���ǰ�벿���ɹ��캯����baseName����ȷ���ġ�
//���磺�ٶ�baseNameΪ"fifo"��
//��nb==0����fileNameΪfifo000000��
//��nb==241����fileNameΪfifo000241��
//��nb=12832����fileNameΪfifo012832��
void PersistentFifo::makeName(uint nb)
{
    //����ѭ��������fileName�����һλ��ʼ������nbֵ����fileName����ĺ���λ��
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
    else//����������ʱ��
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
    if (in != out)//�����Ϊ��
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

//��ȡFifo���е�һ���м�¼���ȴ�buf2�ж�ȡ����
//ֱ����ȡ�����ݺ����Ż᷵�ء�
char *PersistentFifo::readLine()
{
    if (buf2Pos == buf2End) //���buf2Pos��buf2End��ͬ������buf2�������ȫ�����꣬����԰�buf2���á�
    {
        buf2Pos = 0; buf2End = 0; buf2[0] = 0;
    }

    char *posn = strchr(buf2 + buf2Pos, '\n'); //�ҵ���һ�γ���\n��λ�á�

    while (posn == NULL)
    {
        if (!(buf2End - buf2Pos < maxUrlSize + 40 + maxCookieSize))
        {
            //
        }
        if (buf2Pos * 2 > BUF_SIZE)//��buf2Posָ��λ�ö���buf2��һ��ʱ����buf2ǰ�ƣ����ܱ�֤buf2End���ᵽ��BUF_SIZEλ�á�
        {
            buf2End -= buf2Pos;
            memmove(buf2, buf2 + buf2Pos, buf2End);
            //�������д������д�ɸ���������ʽ��
            //memmove(buf,buf+bufPos,bufEnd-bufPos);
            //bufEnd = bufEnd-bufPos;
        }
        int postmp = buf2End;
        bool noRead = true; //bool����noRead����whileѭ�����˳���
        while (noRead)
        {
            int nb;
            if (rfile.bad())//�ж�����״̬��������ɻָ��Ĵ������˳�����
            {
                cerr << "file stream err in function PersistentFifo::readLine" << endl;
                exit(1);
            }
            if (rfile.fail()) //����������˿ɻָ��Ĵ��󣨰����������ļ�������ʱ������Ӧ�ûָ���Ϊ����״̬��
            {
                rfile.clear(); //�ָ�����״̬��
            }

            rfile.read(buf2 + buf2End, BUF_SIZE - 1 - buf2End);//��rfile���ȡ���ݵ�buf2�һ�ζ�ȡ�����ܶ���ֽڡ�
            nb = rfile.gcount(); //�����ȡ�˶��ٸ��ֽڡ�

            switch (nb)
            {
            case 0:
                //�����rfile��û�ж�ȡ�����ݣ��������ļ�����λ�÷��������´ε���ǰ�ָ�����״̬����
                //��ˢ��buf1������������д�뵽�ļ��С�
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
        rfile.close(); //�ر�ȫ���������ݵ��ļ���
        makeName(fout); //�õ����ļ�����
        remove(fileName); //ɾ�����ļ���
        makeName(++fout); //�������ļ������֡�
        rfile.open(fileName); //�����ļ���
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