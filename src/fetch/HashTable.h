#ifndef HASHTABLE_H
#define HASHTABLE_H

#include"../utils/Url.h"

class HashTable
{
private:
    int size;
    char *table;

public:
    HashTable();
    ~HashTable();

    /* test if this url is allready in the hashtable
    * return true if it has been added
    * return false if it has allready been seen
    */
    bool test(Url *u);

    /* set a url as present in the hashtable
    */
    void set(Url *u);

    /* add a new url in the hashtable
    * return true if it has been added
    * return false if it has allready been seen
    */
    bool testSet(Url *u);

};

#endif