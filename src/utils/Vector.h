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
    T *operator[](uint i);  //�����±��������


private:
    T **tab;   //����ָ�룬�ں�����������ʹ�á�
    uint pos;  //pos��ʾvector��ʹ��������
    uint size;  //size��ʾvector��ռ��������

};


template <class T>
Vector<T>::Vector(uint size)
{
    this->size = size;
    pos = 0;
    tab = new T* [size];
}


//����tab���ǵ������󣬶��Ƕ������飬���Ծ��ȵ���ÿ�����������������delete tab[i];�������ջؿռ䣨delete [] tab;����
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
    if (pos==size) //������������ˣ����������һ��������ǰ�Ķ�������ɾ��������һ��������һ���Ķ������飩��
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