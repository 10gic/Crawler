/**
* @file ConstantSizedFifo.h
* @brief 定义和实现了一个类模板ConstantSizedFifo（模板类的定义和实现应该放在一个文件里）
* @author 10gic
* @date 2011/5/4
* @version
* @note
* 这个队列不能自动增加大小，它支持同步。
*/

#ifndef CONSTANTFIFO_H
#define CONSTANTFIFO_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include<Windows.h>

#include<cassert>

template<class T>
class ConstantSizedFifo
{
public:
    ConstantSizedFifo(uint size);
    ~ConstantSizedFifo();

    T *get(); /**< 得到第一个对象 */

    T *tryGet(); /**< 得到第一个对象，如果没有对象，则返回NULL */

    void put(T *obj);

    int getLength();

    bool isNonEmpty();

protected:
    uint in, out;
    uint size;
    T **tab;
    HANDLE hmtx;
};


template<class T>
ConstantSizedFifo<T>::ConstantSizedFifo(uint size)
{
    this->size = size + 1;
    tab = new T *[this->size];
    in = 0;
    out = 0;
    hmtx = CreateMutex(NULL, FALSE, NULL);
}

template<class T>
ConstantSizedFifo<T>::~ConstantSizedFifo()
{
    delete[] tab;
}

template<class T>
T *ConstantSizedFifo<T>::get()
{
    T *tmp;
    WaitForSingleObject(hmtx, INFINITE);
    tmp = tab[out];
    out = (out + 1) % size;
    ReleaseMutex(hmtx);
    return tmp;
}

template<class T>
T *ConstantSizedFifo<T>::tryGet()
{
    T *tmp = NULL;
    WaitForSingleObject(hmtx, INFINITE);
    if (in != out)
    {
        tmp = tab[out];
        out = (out + 1) % size;
    }
    ReleaseMutex(hmtx);
    return tmp;
}

template<class T>
void ConstantSizedFifo<T>::put(T *obj)
{
    WaitForSingleObject(hmtx, INFINITE);
    tab[in] = obj;
    in = (in + 1) % size;
    assert(in != out);
    ReleaseMutex(hmtx);
}

template<class T>
int ConstantSizedFifo<T>::getLength()
{
    int tmp;
    WaitForSingleObject(hmtx, INFINITE);
    tmp = (in + size - out) % size;
    ReleaseMutex(hmtx);
    return tmp;
}


//不为空时，isNonEmpty()返回true。
//为空时，isNonEmpty()返回false。
template<class T>
bool ConstantSizedFifo<T>::isNonEmpty()
{
    WaitForSingleObject(hmtx, INFINITE);
    bool res = (in != out);
    ReleaseMutex(hmtx);
    return res;
}

#endif