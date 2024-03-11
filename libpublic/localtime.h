
#ifndef _LOCALTIME_H_
#define _LOCALTIME_H_

#ifdef _MSC_VER
#include <time.h>
#include <windows.h> 
#else 
#include <sys/time.h>
#include <time.h>
#endif

class CLocaltime {
public:
    CLocaltime();
    ~CLocaltime();

    int getDaylightActive();
    void nolocksLocaltime(const time_t *timep, struct tm *result);

public:
    int daylightActive;
};

#endif /* _LOCALTIME_H_ */