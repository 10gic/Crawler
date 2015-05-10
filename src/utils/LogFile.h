/*
���ԣ�http://www.cppblog.com/tx7do/articles/5768.html
ԭ����δ��ȷ˵����Ȩ�����:)

Use:
class LogFile;//�û�������־�ļ���
class LogFileEx;//����־�ļ����Զ����ɹ���, �ɷ�������Ƶ�������ļ���, ��ָ����־��ŵ�Ŀ¼

LogFile gLog("My.Log");
gLog.Log("ϵͳ����");

LogFileEx gLog(".", LogFileEx :: YEAR);//һ������һ����־�ļ�
LogFileEx gLog(".\\Log", LogFileEx :: MONTH);//һ������һ����־�ļ�
LogFileEx gLog(".\\Log", LogFileEx :: DAY);//һ������һ����־�ļ�
//ע����־����Ŀ¼����ʧ�ܻ��Զ��˳�����ע��Ŀ¼�ĺϷ��ԣ��ļ�����Ƶ�ʿ��������
//24Сʱ���еĳ������ÿ������һ����־�ļ����������ݹ���
*/


#pragma once

#ifndef LOGFILE_H
#define LOGFILE_H

#include <assert.h>
#include <time.h>
#include <stdio.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

class LogFile
{
public:
    LogFile(const char *szFileName = "Log.log") //Ĭ��ʵ�εĹ��캯����ʵ��Ϊ��־�ļ�����
    {
        _szFileName = NULL;
        _hFile = INVALID_HANDLE_VALUE;
        ::InitializeCriticalSection(&_csLock);

        SetFileName(szFileName);
    }

    virtual ~LogFile()
    {
        ::DeleteCriticalSection(&_csLock);
        Close();
        if (_szFileName)
            delete[]_szFileName;
    }

    const char *GetFileName()	{ return _szFileName; }
    void SetFileName(const char *szName)//�޸��ļ�����ͬʱ�ر���һ����־�ļ�
    {
        assert(szName);
        if (_szFileName)
            delete[]_szFileName;

        Close();
        _szFileName = new char[strlen(szName) + 1];
        assert(_szFileName);
        strcpy(_szFileName, szName);
    }

    bool IsOpen()	{ return _hFile != INVALID_HANDLE_VALUE; }

    void Log(const char *szText, DWORD dwLength)//׷����־����
    {
        assert(szText);
        __try
        {
            Lock();

            if (!OpenFile())
                return;

            WriteLog(szText, dwLength);
        }
        __finally
        {
            Unlock();
        }
    }

    void Log(const char *szText)//׷����־����
    {
        Log(szText, strlen(szText));
    }

    void Close()
    {
        if (IsOpen())
        {
            CloseHandle(_hFile);
            _hFile = INVALID_HANDLE_VALUE;
        }
    }


protected:
    CRITICAL_SECTION _csLock;
    char * _szFileName;
    HANDLE _hFile;

    bool OpenFile()//���ļ���ָ�뵽�ļ�β
    {
        if (IsOpen())
            return true;

        if (!_szFileName)
            return false;

        _hFile = CreateFileA(
            _szFileName,
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

        if (!IsOpen() && GetLastError() == 2)//�򿪲��ɹ�������Ϊ�ļ������ڣ��򴴽��ļ���
            _hFile = CreateFileA(
            _szFileName,
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

        if (IsOpen())
            SetFilePointer(_hFile, 0, NULL, FILE_END);

        return IsOpen();
    }

    virtual void WriteLog(LPCVOID lpBuffer, DWORD dwLength)//д��־, ������չ�޸�
    {
        time_t now;
        char temp[21];
        DWORD dwWriteLength;

        if (IsOpen())
        {
            time(&now);
            strftime(temp, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
            //WriteFile(_hFile, "\xd\xa#-----------------------------", 32, &dwWriteLength, NULL);
            WriteFile(_hFile, temp, 19, &dwWriteLength, NULL);
            //WriteFile(_hFile, "-----------------------------#\xd\xa", 32, &dwWriteLength, NULL);
            WriteFile(_hFile, lpBuffer, dwLength, &dwWriteLength, NULL);
            WriteFile(_hFile, " ", 1, &dwWriteLength, NULL);
            WriteFile(_hFile, "\xd\xa", 2, &dwWriteLength, NULL);
            FlushFileBuffers(_hFile);
        }
    }

    void Lock()   { ::EnterCriticalSection(&_csLock); }
    void Unlock() { ::LeaveCriticalSection(&_csLock); }

private:   //���ο������캯���Ϳ������츳ֵ��������
    LogFile(const LogFile&);
    LogFile&operator = (const LogFile&);
};

#if 0 //��������ʱû�У�ע�͵���
class LogFileEx : public LogFile
{
public:
    enum LOG_TYPE{YEAR = 0, MONTH = 1, DAY = 2};

    LogFileEx(const char *szPath = ".", LOG_TYPE iType = MONTH)
    {
        _szPath = NULL;
        SetPath(szPath);
        _iType = iType;
        memset(_szLastDate, 0, 9);
    }

    ~LogFileEx()
    {
        if(_szPath)
            delete []_szPath;
    }

    const char * GetPath()
    {
        return _szPath;
    }

    void Log(const char *szText)
    {
        assert(szText);

        char temp[10];
        static const char format[3][10] = {"%Y", "%Y-%m", "%Y%m%d"};

        __try
        {
            Lock();

            time_t now = time(NULL);

            strftime(temp, 9, format[_iType], localtime(&now));

            if(strcmp(_szLastDate, temp) != 0)//�����ļ���
            {
                strcat(strcpy(_szFileName, _szPath), "\\");
                strcat(strcat(_szFileName, temp), ".log");
                strcpy(_szLastDate, temp);
                Close();
            }

            if(!OpenFile())
                return;

            WriteLog(szText, strlen(szText));
        }
        __finally
        {
            Unlock();
        }
    }

protected:
    char *_szPath;
    char _szLastDate[9];
    int _iType;

    void SetPath(const char *szPath)
    {
        assert(szPath);

        WIN32_FIND_DATAA wfd;
        char temp[MAX_PATH + 1] = {0};

        if(FindFirstFileA(szPath, &wfd) == INVALID_HANDLE_VALUE && CreateDirectoryA(szPath, NULL) == 0)
        {
            strcat(strcpy(temp, szPath), " Create Fail. Exit Now! Error ID :");
            //ltoa(GetLastError(), temp + strlen(temp), 10);
            MessageBoxA(NULL, temp, "Class LogFileEx", MB_OK);
            //exit(1);
        }
        else
        {
            GetFullPathNameA(szPath, MAX_PATH, temp, NULL);
            _szPath = new char[strlen(temp) + 1];
            assert(_szPath);
            strcpy(_szPath, temp);
        }
    }

private://���κ���
    LogFileEx(const LogFileEx&);
    LogFileEx&operator = (const LogFileEx&);
};
#endif

#endif