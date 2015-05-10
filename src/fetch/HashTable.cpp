#include "HashTable.h"
#include "../Types.h"

HashTable::HashTable()
{
    int total = hashSize/8;
    table = new char[total];
    for(int i=0; i<total; i++)
    {
        table[i] = 0;
    }
}

HashTable::~HashTable()
{
    delete [] table;
}

/* test if this url is allready in the hashtable
 * return true if it has been added
 * return false if it has allready been seen
 */
bool HashTable::test (Url *u) 
{
    unsigned int code = u->hashCode();
    unsigned int pos = code / 8;
    unsigned int bits = 1 << (code % 8);
    return table[pos] & bits;
}

/* set a url as present in the hashtable
 */
void HashTable::set (Url *u) 
{
    unsigned int code = u->hashCode();
    unsigned int pos = code / 8;
    unsigned int bits = 1 << (code % 8);
    table[pos] |= bits;
}

/* add a new url in the hashtable
 * return true if it has been added
 * return false if it has allready been seen
 */
bool HashTable::testSet (Url *u) 
{
    unsigned int code = u->hashCode();
    unsigned int pos = code / 8;
    unsigned int bits = 1 << (code % 8);
    int res = table[pos] & bits;
    table[pos] |= bits;
    return !res;
}