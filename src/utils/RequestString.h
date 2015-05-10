/**
* @file RequestString.h
* @brief 本文件定义了类RequestString
* @author 10gic
* @date 2011/4/28
* @version
* @note
* Detailed description.
*/

#ifndef STRING_H
#define STRING_H

#include<cassert>

#include "../Types.h"

#define STRING_SIZE 1024

class RequestString
{
public:
    /** @brief 默认实参的构造函数 */
    RequestString(uint size = STRING_SIZE);

    /** @brief 析构函数 */
    ~RequestString();

    void recycle(uint size = STRING_SIZE);

    /** @brief 返回字符串，但它将在你删除RequestString对象时，自动删除。 */
    char *getString();

    /** @brief 返回字符串，返回的是创造了一个新的字符串，你必须手动delete它。 */
    char *giveString();


    /** @brief  */
    void addString(const char *s);



private:
    char * chain;
    uint pos;
    uint size;

};

#endif