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
* ע�⣺����a����ΪСд��
*/
bool startWithIgnoreCase(const char *a,const char *b)
{
    int i = 0;
    while (a[i] != 0)
    {
        if (a[i] != (b[i]|32)) //��ʹСд���䣬��дת��ΪСд
        {
            return false;
        }
        i++;
    }
    return true;
}


//ע�⣺����amin����ΪСд��
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

//������һ����ǿ�ʼ�ĵط���
//��һ������Ϊ����ָ�루ָ���ָ�룩���ں������ı����ָ��ָ���ֵ��һ��ָ�룩��
char *nextToken(char **posParse,const char c)
{
    bool cont = 1;
    while (cont)
    {
        if (**posParse == c || **posParse == ' ' || **posParse == '\t' || **posParse == '\r' || **posParse == '\n')
        {
            (*posParse)++; //�ƶ���һ�С�
        }
        else if (**posParse == '#') //���ָ��λ��Ϊ#����ʾע����һ�У�
        {
            *posParse = strchr(*posParse,'\n');
            if (*posParse == NULL)
            {
                return NULL;
            }
            else
            {
                (*posParse)++; //�ƶ���һ�С�
            }
        }
        else
        {
            cont = 0;
        }
    } //whileѭ������ʱ��(*posParse)ָ��һ����Ч��ǵĿ�ʼ��һ�С�

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