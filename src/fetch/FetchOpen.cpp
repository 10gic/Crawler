#include "FetchOpen.h"
#include "../Global.h"


void fetchDns()
{
    //Submit queries
    //larbin�ж�DNS�����������ơ������DNS�������첽�ģ��������ģ���DNS����û�����ơ�
    NamedSite *site = Global::dnsSites->tryGet();
    if (NULL == site)
    {
        return;
    }
    else
    {
        site->newQuery();
        site->scanQueue();
    }
}


void fetchOpen()
{

    int cont = 1;

    //��okSites����Ԫ����freeConns�������ӡ�
    while (cont && Global::freeConns->isNonEmpty())
    {
        //�ӿ������ص�Url����(okSites)�л�ȡUrl��
        IPSite *s = Global::okSites->tryGet();
        if (s == NULL)
        {
            cont = 0;
        }
        else
        {
            s->fetch();
        }
    }

}