#include "../Global.h"
#include "Url.h"

static bool canGetUrl (bool *testPriority);

uint space = 0;

#define maxPerCall 100


void sequencer()
{
    bool testPriority = true;
    if (space == 0) {
        space = Global::inter->putAll();  //得到能够装载的URL数
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
按优先级顺序将URL加入到namedSiteList，队列中有URL返回true，否则返回false。

if(URLPriorityWait中有url)
从中获得url加入到namedSiteList;
else if(URLPtiority中有url)
从中获得url加入到namedSiteList;
else {
    if(URLDiskwait中有url)
        从中获得url加入到namedSiteList;
    else  //URLDisk中有url
        从中获得url加入到namedSiteList;
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