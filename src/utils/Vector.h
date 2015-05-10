#pragma once

#ifndef VECTOR_H
#define VECTOR_H

#include "../Types.h"

template <class T>
class Vector
{
public:
    Vector(uint size = StdVectSize);
    ~Vector();
    void recycle();
    void addElement(T *elem);
    inline uint getLength(){return pos;}
    inline T **getTab(){return tab;}
    T *operator[](uint i);  //重载下标运算符。


private:
    T **tab;   //二级指针，在后做对象数组使用。
    uint pos;  //pos表示vector的使用容量。
    uint size;  //size表示vector的占用容量。

};


template <class T>
Vector<T>::Vector(uint size)
{
    this->size = size;
    pos = 0;
    tab = new T* [size];
}


//由于tab不是单个对象，而是对象数组，所以就先调用每个对象的析构函数（delete tab[i];），再收回空间（delete [] tab;）。
template <class T>
Vector<T>::~Vector () 
{
    for (uint i=0; i<pos; i++)
    {
        delete tab[i];
    }
    delete [] tab;
}

template <class T>
void Vector<T>::recycle()
{
    for (uint i=0;i<pos;i++)
    {
        delete tab[i];
    }
    pos = 0;
}

template <class T>
void Vector<T>::addElement(T *elem)
{
    if (pos==size) //如果容量不够了，则把容量加一倍（把以前的对象数组删除，创建一个容量大一倍的对象数组）。
    {
        size *= 2;
        T **tmp = new T* [size];
        for (uint i=0;i<pos;i++)
        {
            tmp[i] = tab[i];
        }
        delete [] tab;
        tab = tmp;
    }
    tab[pos] = elem;
    pos++;
}

template <class T>
T *Vector<T>::operator [](uint i)
{
    if (i<pos)
    {
        return tab[i];
    }
    else
    {
        return NULL;
    }
}

#endif VECTOR_H