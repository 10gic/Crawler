#include "CheckFinished.h"
#include "../Global.h"

#define checkFinishedTimeout 20
//超时时间(20秒)，当所有的连接都为emptyC，若经过checkTimeout时间后仍然都为emptyC，则认为爬虫结束。

static time_t lastFinishedTime;
static bool isAllFinished = false;

bool checkFinished()
{
    //time_t old = lastFinishedTime;
    for (uint i = 0; i < Global::number_conn; i++)
    {
        Connection *conn = Global::connections + i;
        if (usedC == conn->state) //发现有usedC状态的连接，则直接返回false。
        {
            isAllFinished = false;
            return false;
        }
    }

    //所有的连接都不为usedC（也就是所有的连接都为emptyC）时。
    if (false == isAllFinished)//如果上次检测时有连接未完成，则更新lastFinishedTime为现在时间。
    {
        lastFinishedTime = time(NULL);
    }
    isAllFinished = true;

    if ((time(NULL) - lastFinishedTime) > checkFinishedTimeout)
    {
        return true;
    }
    else
    {
        return false;
    }
}
