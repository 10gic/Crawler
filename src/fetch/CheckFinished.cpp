#include "CheckFinished.h"
#include "../Global.h"

#define checkFinishedTimeout 20
//��ʱʱ��(20��)�������е����Ӷ�ΪemptyC��������checkTimeoutʱ�����Ȼ��ΪemptyC������Ϊ���������

static time_t lastFinishedTime;
static bool isAllFinished = false;

bool checkFinished()
{
    //time_t old = lastFinishedTime;
    for (uint i = 0; i < Global::number_conn; i++)
    {
        Connection *conn = Global::connections + i;
        if (usedC == conn->state) //������usedC״̬�����ӣ���ֱ�ӷ���false��
        {
            isAllFinished = false;
            return false;
        }
    }

    //���е����Ӷ���ΪusedC��Ҳ�������е����Ӷ�ΪemptyC��ʱ��
    if (false == isAllFinished)//����ϴμ��ʱ������δ��ɣ������lastFinishedTimeΪ����ʱ�䡣
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
