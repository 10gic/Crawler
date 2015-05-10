#pragma once

#ifndef TYPES_H
#define TYPES_H

// Size of the HashSize (max number of urls that can be fetched)
#define hashSize 64000000

#define StdVectSize 100
#define maxPageSize    100000

#define maxUrlSize 512   //控制整个URL长度。
#define maxSiteSize 40   //控制host name的长度。

#define maxCookieSize 128

#define namedSiteListSize 20000  //NamedSite数组的大小。控制同时爬取的站点数。
#define IPSiteListSize 10000

#define ramUrls 100000   // Max number of urls in ram

#define maxUrlsBySite 256 //一个页面中某个域名最多的Url数。////如果超过个数，URL将放在URLsDiskWait队列里。

#define BufSmall   7000  //小缓冲区的大小(一次接收)，调用WSARecv时指定缓冲区大小。
#define BufBig  4000000 //大缓冲区的大小(整个网页大小不能超过这个值)，单IO数据里指定的缓存区大小。
//这里限制了本程序下载网页（包括文件）的大小为：BufBig-BufSmall（大约3M多，由于现在工具还没支持gzip解压，所以这里应当适当设置大一些）。
//对于下载网页来说够了，如果是下载网页中的文件，则不够！


enum FetchError
{
  success,
  noDNS,
  noConnection,
  forbiddenRobots,
  timeout,
  badType,
  tooBig,
  err30X,
  err40X,
  earlyStop,
  duplicate,
  fastRobots,
  fastNoConn,
  fastNoDns,
  tooDeep,
  urlDup
};

typedef signed char int8_t;
typedef short int int16_t;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

typedef	unsigned int uint;

#endif