/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   随机数
*************************************************************************/

#ifndef _RAND_H_
#define _RAND_H_

#include <stdint.h>
#include <string>

class Rand {
public:
    //随机数 [nMin, nMax]
    static uint32_t randInt(uint32_t nMin, uint32_t nMax);
    
    // 随机字符串
    static std::string randString(const uint32_t nSize);

    static uint32_t nRandCount;
};

#endif // _RAND_H_
