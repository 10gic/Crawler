/*
来自：http://www.cppblog.com/tx7do/articles/5768.html
原作者未明确说明版权情况。:)

Use:
class LogFile;//用户定义日志文件名
class LogFileEx;//有日志文件名自动生成功能, 可分年月日频率生成文件名, 可指定日志存放的目录

LogFile gLog("My.Log");
gLog.Log("系统启动");

LogFileEx gLog(".", LogFileEx :: YEAR);//一年生成一个日志文件
LogFileEx gLog(".\\Log", LogFileEx :: MONTH);//一月生成一个日志文件
LogFileEx gLog(".\\Log", LogFileEx :: DAY);//一天生成一个日志文件
//注意日志所属目录创建失败会自动退出，请注意目录的合法性，文件生成频率看情况掌握
//24小时运行的程序可以每天生成一个日志文件，以免内容过多
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
    LogFile(const char *szFileName = "Log.log") //默认实参的构造函数，实参为日志文件名。
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
    void SetFileName(const char *szName)//修改文件名，同时关闭上一个日志文件
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

    void Log(const char *szText, DWORD dwLength)//追加日志内容
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

    void Log(const char *szText)//追加日志内容
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

    bool OpenFile()//打开文件，指针到文件尾
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

        if (!IsOpen() && GetLastError() == 2)//打开不成功，且因为文件不存在，则创建文件。
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

    virtual void WriteLog(LPCVOID lpBuffer, DWORD dwLength)//写日志, 可以扩展修改
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

private:   //屏蔽拷贝构造函数和拷贝构造赋值操作符。
    LogFile(const LogFile&);
    LogFile&operator = (const LogFile&);
};

#if 0 //下面类暂时没有，注释掉。
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

            if(strcmp(_szLastDate, temp) != 0)//更换文件名
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

private://屏蔽函数
    LogFileEx(const LogFileEx&);
    LogFileEx&operator = (const LogFileEx&);
};
#endif

#endif