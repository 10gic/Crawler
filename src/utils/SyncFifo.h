#pragma once

#ifndef SYNCFIFO_H
#define SYNCFIFO_H

#define StdSyncFifoSize 100

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "../Types.h"

template <class T>
class SyncFifo
{
protected:
    uint in;
    uint out;
    uint size;
    T **tab;
    HANDLE hmtx;

public:
    SyncFifo(uint size = StdSyncFifoSize);
    ~SyncFifo();
    T *get();
    T *tryGet();
    void put(T *obj);
    int getLength();
};


template <class T>
SyncFifo<T>::SyncFifo(uint size)
{
    tab = new T *[size];
    this->size = size;
    in = 0;
    out = 0;
    hmtx = CreateMutex(NULL,FALSE,NULL);
}

template <class T>
SyncFifo<T>::~SyncFifo()
{
    delete [] tab;
    CloseHandle(hmtx);
}

template <class T>
T *SyncFifo<T>::get()
{
    T *tmp;
    WaitForSingleObject(hmtx,INFINITE);
    tmp = tab[out];
    out = (out + 1)%size;
    ReleaseMutex(hmtx);
    return tmp;
}

template <class T>
T *SyncFifo<T>::tryGet()
{
    T *tmp = NULL;
    WaitForSingleObject(hmtx,INFINITE);
    if (in != out)
    {
        tmp = tab[out];
        out = (out + 1)%size;
    }
    ReleaseMutex(hmtx);
    return tmp;
}

template <class T>
void SyncFifo<T>::put(T *obj)
{
    WaitForSingleObject(hmtx,INFINITE);
    tab[in] = obj;
    in = (in+1)%size;
    if (in == out)
    {
        T **tmp;
        tmp = new T*[2*size];
        for (uint i=out; i<size; i++) {
            tmp[i] = tab[i];
        }
        for (uint i=0; i<in; i++) {
            tmp[i+size] = tab[i];
        }
        in += size;
        size *= 2;
        delete [] tab;
        tab = tmp;
    }
    ReleaseMutex(hmtx);	
}

template <class T>
int SyncFifo<T>::getLength()
{
    int tmp;
    WaitForSingleObject(hmtx);
    tmp = (in + size - out)%size;
    ReleaseMutex(hmtx);
    return tmp;
}

#endif