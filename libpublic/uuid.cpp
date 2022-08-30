/*************************************************************************
	> Date:   2020-12-28
	> Author: liangjun
	> Desc:   uuid
*************************************************************************/

#include "uuid.h"

#define UUID_LENGTH 32

std::string CUUID::generateUUID()
{
    char buf[UUID_LENGTH+1] = {0};

#ifdef _MSC_VER
    GUID guid;
    CoCreateGuid(&guid);
    _snprintf_s(
                buf,
                sizeof(buf),
                "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
                guid.Data1, guid.Data2, guid.Data3,
                guid.Data4[0], guid.Data4[1],
                guid.Data4[2], guid.Data4[3],
                guid.Data4[4], guid.Data4[5],
                guid.Data4[6], guid.Data4[7]);
#else
    uuid_t uuid;
    uuid_generate(uuid);
    for(int i = 0; i < 16; i++) {
        sprintf(&buf[i*2], "%02x", (unsigned int)uuid[i]);
    }
#endif

    return std::string(buf);
}

std::string CUUID::generateStdUUID()
{
    char buf[UUID_LENGTH+5] = {0};
    
#ifdef _MSC_VER
    GUID guid;
    CoCreateGuid(&guid);
    _snprintf_s(
                buf,
                sizeof(buf),
                "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                guid.Data1, guid.Data2, guid.Data3,
                guid.Data4[0], guid.Data4[1],
                guid.Data4[2], guid.Data4[3],
                guid.Data4[4], guid.Data4[5],
                guid.Data4[6], guid.Data4[7]);
#else
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, buf);
#endif

    return std::string(buf);
}