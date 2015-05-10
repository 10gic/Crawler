#ifndef CHECKER_H
#define CHECKER_H

#include"../Global.h"

//把URL加入到爬虫队列里。在“加入起始URL时”和“分析网页找到链接时”会用到。
void check(Url *u);

//对域名和文件类型进行过滤。
bool filter1(char *host,char *file);

#endif