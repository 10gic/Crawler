#include "FetchPipe.h"
#include "../Types.h"
#include "../Global.h"
#include "../interface/Output.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <exception>
using std::exception;

static void endOfFile(Connection *conn);

//������ӵ�״̬��������ҳ�������������������������ö��̡߳�������n���̣߳���ÿ���̼߳��Global::number_conn/n�����ӡ���
void checkAll()
{
    for (uint i = 0; i < Global::number_conn; i++)
    {
        Connection *conn = Global::connections + i;
        if (usedC == conn->state)
        {
            switch (conn->pConnData->state)
            {
            case IONotSet:
                //������ӻ�û�д���ɶ˿ڷ��ء�
                break;

            case  IOFinished:
                //cout<<"��ҳ�Ѿ������꣡"<<endl;
                //cout<<conn->request.getString()<<endl;
                //cout<<"���ι����յ��ֽ���Ϊ��"<<endl;
                //cout<<conn->pConnData->top<<endl;
                endOfFile(conn);  //�����ļ�����ҳ�ȡ�

                conn->recycle();//�ͷ���Դ�������ͷ���ɶ˿ڵ�IO����
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;

            case IOCloseByOtherSide:
                endOfFile(conn);  //�����ļ�����ҳ�ȡ�

                conn->recycle();//�ͷ���Դ�������ͷ���ɶ˿ڵ�IO����
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;

            case IOTooBig:
                cout << "���ص�����̫�����ر����ԣ�" << endl;
                addlog("���ص�����̫�����ر����ԣ�");
                cout << conn->request.getString() << endl;

                conn->recycle();//�ͷ���Դ�������ͷ���ɶ˿ڵ�IO����
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;

            case IONotInterest:

                conn->recycle();//�ͷ���Դ�������ͷ���ɶ˿ڵ�IO����
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;

            case IOOtherError:

                conn->recycle();//�ͷ���Դ�������ͷ���ɶ˿ڵ�IO����
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;
            }//end switch

        }//end if
    }//end for
}

//������ҳ�ȡ�
static void endOfFile(Connection *conn)
{
    conn->pConnData->parser->endInput();
    if (200 == ((Html *)(conn->pConnData->parser))->respondCode)
    {
        endOfLoad((Html *)conn->pConnData->parser); //�ú�����Output.cpp�������ҳ���������ļ���
    }
}