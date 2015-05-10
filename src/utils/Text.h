/**
* @file Text.h
* @brief 本文件对一些字符串操作的函数进行了声明，其定义在对应的源文件里。
* @author 10gic
* @date 2011/4/28
* @version 
* @note
* Detailed description.
*/

#ifndef TEXT_H
#define TEXT_H

char lowerCase(char a);

char *newString (const char *arg);

bool startWith (const char *a, const char *b);

bool startWithIgnoreCase(const char *a,const char *b);

bool endWithIgnoreCase(const char *amin,const char *b,int lb);

char *nextToken(char **posParse,char c = ' ');

#endif