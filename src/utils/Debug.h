#ifndef DEBUG_H
#define DEBUG_H

#ifdef _DEBUG

#include"LogFile.h"

#define addlog(i)   Global::gLog.Log(i)
#define addlog2(i,j)  Global::gLog.Log(i,j)

#define crash(s)   (std::cerr<< s << std::endl)

#else

#define addlog(i)   ((void) 0)
#define addlog2(i,j)  ((void) 0)

#define crash(s)   ((void) 0)

#endif


#endif