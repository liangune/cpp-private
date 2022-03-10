/*************************************************************************
	> Date:   2020-12-28
	> Author: liangjun
	> Desc:   uuid
*************************************************************************/

#ifndef _LIB_PUBLIC_UUID_H_
#define _LIB_PUBLIC_UUID_H_

#include <string>
#include <stdio.h>
#ifdef _MSC_VER
#include <ObjBase.h>
#else
#include <libuuid/uuid.h>
#endif

class CUUID {
public:
    // uuid标准格式: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx, 形式为8-4-4-4-12的32个字符
    static std::string generateStdUUID();

    // 32个字符
    static std::string generateUUID();
};

#endif // !_LIB_PUBLIC_UUID_H_