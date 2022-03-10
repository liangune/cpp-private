/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   随机数
*************************************************************************/
#include "rand.h"

#ifdef _MSC_VER
#include <time.h>
#include <windows.h>
#else
#include <sys/time.h>
#endif

uint32_t Rand::nRandCount = 0;

uint32_t Rand::randInt(uint32_t nMin, uint32_t nMax)
{
    if (nMin > nMax)
		return 0;
	if (nMin==nMax)
		return nMax;
	
#ifdef _MSC_VER
	nRandCount++;
	uint32_t ms = GetTickCount() + nRandCount;
	srand(ms);
#else
    struct timeval current;
    gettimeofday(&current, NULL);
	srand(current.tv_usec);
#endif

	return nMin + rand()%(nMax+1-nMin);
}

std::string Rand::randString(const uint32_t nSize)
{
	std::string s;
	for (uint32_t i=0; i<nSize; i++)
	{
		int n = randInt(65, 90);
		if (n%2==1)
			n += 32;
		char c = n;
		s += c;
	}
	return s;
}
