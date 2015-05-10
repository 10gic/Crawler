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

//检查连接的状态，保存网页都在这个函数里，可以在这里设置多线程。（设置n个线程，则每个线程检查Global::number_conn/n个连接。）
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
                //这个连接还没有从完成端口返回。
                break;

            case  IOFinished:
                //cout<<"网页已经下载完！"<<endl;
                //cout<<conn->request.getString()<<endl;
                //cout<<"本次共接收的字节数为："<<endl;
                //cout<<conn->pConnData->top<<endl;
                endOfFile(conn);  //保存文件或网页等。

                conn->recycle();//释放资源，包括释放完成端口单IO数据
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;

            case IOCloseByOtherSide:
                endOfFile(conn);  //保存文件或网页等。

                conn->recycle();//释放资源，包括释放完成端口单IO数据
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;

            case IOTooBig:
                cout << "下载的内容太大，下载被忽略！" << endl;
                addlog("下载的内容太大，下载被忽略！");
                cout << conn->request.getString() << endl;

                conn->recycle();//释放资源，包括释放完成端口单IO数据
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;

            case IONotInterest:

                conn->recycle();//释放资源，包括释放完成端口单IO数据
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;

            case IOOtherError:

                conn->recycle();//释放资源，包括释放完成端口单IO数据
                conn->state = emptyC;
                Global::freeConns->put(conn);
                break;
            }//end switch

        }//end if
    }//end for
}

//保存网页等。
static void endOfFile(Connection *conn)
{
    conn->pConnData->parser->endInput();
    if (200 == ((Html *)(conn->pConnData->parser))->respondCode)
    {
        endOfLoad((Html *)conn->pConnData->parser); //该函数在Output.cpp里。保存网页或者其它文件。
    }
}