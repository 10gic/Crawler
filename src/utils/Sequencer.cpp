#include "../Global.h"
#include "Url.h"

static bool canGetUrl (bool *testPriority);

uint space = 0;

#define maxPerCall 100


void sequencer()
{
    bool testPriority = true;
    if (space == 0) {
        space = Global::inter->putAll();  //�õ��ܹ�װ�ص�URL��
    }
    int still = space;
    if (still > maxPerCall) still = maxPerCall;
    while (still) {
        if (canGetUrl(&testPriority)) 
        {
            space--; still--;
        } 
        else 
        {
            still = 0;
        }
    }
}


/*
�����ȼ�˳��URL���뵽namedSiteList����������URL����true�����򷵻�false��

if(URLPriorityWait����url)
���л��url���뵽namedSiteList;
else if(URLPtiority����url)
���л��url���뵽namedSiteList;
else {
    if(URLDiskwait����url)
        ���л��url���뵽namedSiteList;
    else  //URLDisk����url
        ���л��url���뵽namedSiteList;
}
*/
bool canGetUrl(bool *testPriority)
{
    Url *u;
    u = Global::URLsDisk->tryGet();
    if (u != NULL) 
    {
        Global::namedSiteList[u->hostHashCode()].putUrl(u);
        return true;
    }
    else
    {
        return false;
    }
}