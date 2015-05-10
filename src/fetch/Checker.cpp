#include "Checker.h"
#include "../utils/Text.h"

void check(Url *u)
{
    //测试这个URL没有被爬取过。若没有爬取过则加入到哈希表中。
    if(Global::seen->testSet(u))
    {
        Global::URLsDisk->put(u);
    }
    else
    {
        //URL已经爬取过了。
        delete u;
    }
}

bool filter1(char *host,char *file)
{

    switch(Global::limitDomainLevel)
    {
    case 0:
        break;
    case 1:
        break;
    case 2:
        if (strcmp(host,Global::startURLDomain))
        {
            return false;
        }
        break;
    default: //其它情况还没定义。
        //cerr<<""<<endl;
        ;
    }

    //先确保后缀为html或htm时不被过滤。
    int len = strlen(file);
    if (endWithIgnoreCase("html",file,len) || file[len-1] == '/' ||endWithIgnoreCase("htm",file,len))
    {
        return true;
    }

    int i = 0;
    while (Global::forbExt[i] != NULL)
    {
        if (endWithIgnoreCase(Global::forbExt[i],file,len))
        {
            return false;
        }
        i++;
    }
    return true;
}