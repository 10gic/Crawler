/**
* @file Fifo.h
* @brief 本文件定义和实现了类模块Fifo，它的作用就是模拟队列。
* @author 10gic
* @date 2011/4/28
* @version
* @note
* Detailed description.
*/

#ifndef FIFO_H
#define FIFO_H

#include <cassert>
#include "../Types.h"

template<class T>
class Fifo
{
public:
    Fifo(uint size = maxUrlSize);

    ~Fifo();

    inline T *read(){ return tab[out]; }

    T *tryRead();

    T *get();

    T *tryGet();

    void put(T *obj);

    void rePut(T *obj);

    int getLength();

    inline bool isEmpty(){ return in == out; }

protected:
    uint in;
    uint out;
    uint size;
    T **tab;

};

template<class T>
Fifo<T>::Fifo(uint size)
{
    tab = new T *[size];
    this->size = size;
    in = 0;
    out = 0;
}

template<class T>
Fifo<T>::~Fifo()
{
    delete[] tab;
}

template<class T>
T *Fifo<T>::tryRead()
{
    if (in == out)
    {
        return NULL;
    }
    else
    {
        return tab[out];
    }
}

template<class T>
T *Fifo<T>::get()
{
    T *tmp;
    assert(in != out);
    tmp = tab[out];
    out = (out + 1) % size;
    return tmp;
}

template<class T>
T *Fifo<T>::tryGet()
{
    T *tmp = NULL;
    if (in != out)
    {
        tmp = tab[out];
        out = (out + 1) % size;
    }
    return tmp;
}

template<class T>
void Fifo<T>::put(T *obj)
{
    tab[in] = obj;
    in = (in + 1) % size;
    if (in == out)
    {
        T **tmp;
        tmp = new T *[2 * size];
        for (uint i = out; i < size; i++)
        {
            tmp[i] = tab[i];
        }
        for (uint i = 0; i < in; i++)
        {
            tmp[i + size] = tab[i];
        }
        in += size;
        size *= 2;
        delete[] tab;
        tab = tmp;
    }
}

template<class T>
void Fifo<T>::rePut(T *obj)
{
    out = (out + size - 1) % size;
    tab[out] = obj;
}

template<class T>
int Fifo<T>::getLength()
{
    return (in + size - out) % size;
}

#endif