/*************************************************************************
	> Date:   2022-06-15
	> Author: liangjun
	> Desc:   公共函数
*************************************************************************/

#ifndef _PUBLIC_FUNC_H_
#define _PUBLIC_FUNC_H_

#include <stdint.h>
#include <string>
#include <vector>

class CPublicFunc {
public:
	static void Split(const std::string& sStr, const std::string& sSep, std::vector<int32_t>& vecOut);
	static void Split(const std::string& sStr, const std::string& sSep, std::vector<std::string>& vecOut);
};

#endif // _PUBLIC_FUNC_H_
