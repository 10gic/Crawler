#include "Checker.h"
#include "../utils/Text.h"

void check(Url *u)
{
    //�������URLû�б���ȡ������û����ȡ������뵽��ϣ���С�
    if(Global::seen->testSet(u))
    {
        Global::URLsDisk->put(u);
    }
    else
    {
        //URL�Ѿ���ȡ���ˡ�
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
    default: //���������û���塣
        //cerr<<""<<endl;
        ;
    }

    //��ȷ����׺Ϊhtml��htmʱ�������ˡ�
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