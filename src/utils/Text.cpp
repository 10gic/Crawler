#include <cstring>

#include <cassert>
#include "Text.h"

/* lowercase a char */
char lowerCase (char a) 
{
    if (a >= 'A' && a <= 'Z')
    {
        return a - 'A'+ 'a' ;
    } 
    else
    {
        return a;
    }
}


/* create a copy of a string
 */
char *newString (const char *arg) 
{
    assert(NULL != arg);
    char *res = new char[strlen(arg) + 1];
    strcpy(res, arg);
    return res;
}

/* test if b starts with a
*/
bool startWith (const char *a, const char *b) 
{
    int i = 0;
    while (a[i] != 0) 
    {
        if (a[i] != b[i]) return false;
        i++;
    }
    return true;
}

/* test if b starts with a ignoring case
* 注意：这里a必须为小写！
*/
bool startWithIgnoreCase(const char *a,const char *b)
{
    int i = 0;
    while (a[i] != 0)
    {
        if (a[i] != (b[i]|32)) //能使小写不变，大写转换为小写
        {
            return false;
        }
        i++;
    }
    return true;
}


//注意：这里amin必须为小写！
bool endWithIgnoreCase(const char *amin,const char *b,int lb)
{
    int la = strlen(amin);
    if (la <= lb)
    {
        int i;
        int diff = lb-la;
        for (i = 0; i < la; i++)
        {
            if (amin[i] != (b[diff+i] | 32))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }

}

//返回下一个标记开始的地方。
//第一个参数为二级指针（指针的指针），在函数里会改变二级指针指向的值（一级指针）。
char *nextToken(char **posParse,const char c)
{
    bool cont = 1;
    while (cont)
    {
        if (**posParse == c || **posParse == ' ' || **posParse == '\t' || **posParse == '\r' || **posParse == '\n')
        {
            (*posParse)++; //移动下一行。
        }
        else if (**posParse == '#') //如果指向位置为#（表示注释那一行）
        {
            *posParse = strchr(*posParse,'\n');
            if (*posParse == NULL)
            {
                return NULL;
            }
            else
            {
                (*posParse)++; //移动下一行。
            }
        }
        else
        {
            cont = 0;
        }
    } //while循环结束时，(*posParse)指向一个有效标记的开始的一行。

    char *deb = *posParse;
    if (**posParse == '\"')
    {
        deb++;
        (*posParse)++;
        while (**posParse != 0 && **posParse != '\"')
        {
            (*posParse)++;
        }
    }
    else
    {
        while(**posParse != 0 && **posParse != c && **posParse != ' ' 
            && **posParse != '\t' && **posParse != '\r' && **posParse != '\n')
        {
            (*posParse)++;
        }
        if (*posParse == deb)
            return NULL; //EOF
    }

    if (**posParse != 0)
    {
        **posParse = 0;
        (*posParse)++;
    }
    return deb;
}