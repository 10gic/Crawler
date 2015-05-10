#pragma once

#ifndef TYPES_H
#define TYPES_H

// Size of the HashSize (max number of urls that can be fetched)
#define hashSize 64000000

#define StdVectSize 100
#define maxPageSize    100000

#define maxUrlSize 512   //��������URL���ȡ�
#define maxSiteSize 40   //����host name�ĳ��ȡ�

#define maxCookieSize 128

#define namedSiteListSize 20000  //NamedSite����Ĵ�С������ͬʱ��ȡ��վ������
#define IPSiteListSize 10000

#define ramUrls 100000   // Max number of urls in ram

#define maxUrlsBySite 256 //һ��ҳ����ĳ����������Url����////�������������URL������URLsDiskWait�����

#define BufSmall   7000  //С�������Ĵ�С(һ�ν���)������WSARecvʱָ����������С��
#define BufBig  4000000 //�󻺳����Ĵ�С(������ҳ��С���ܳ������ֵ)����IO������ָ���Ļ�������С��
//���������˱�����������ҳ�������ļ����Ĵ�СΪ��BufBig-BufSmall����Լ3M�࣬�������ڹ��߻�û֧��gzip��ѹ����������Ӧ���ʵ����ô�һЩ����
//����������ҳ��˵���ˣ������������ҳ�е��ļ����򲻹���


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