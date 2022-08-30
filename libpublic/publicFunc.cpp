#include "publicFunc.h"

void CPublicFunc::Split(const std::string& sStr, const std::string& sSep, std::vector<int32_t>& vecOut)
{
	vecOut.clear();
	std::size_t nLast = 0;
	std::size_t nIndex = sStr.find(sSep, nLast);
	while (nIndex != std::string::npos)
	{
		std::string sTmp = sStr.substr(nLast, nIndex-nLast);
		int32_t nTmp = atoi(sTmp.c_str());
		vecOut.push_back(nTmp);
		nLast = nIndex+sSep.length();
		nIndex=sStr.find(sSep, nLast);
	}

	if (nIndex-nLast > 0)
	{
		std::string sTmp = sStr.substr(nLast,nIndex-nLast);
		int32_t nTmp = atoi(sTmp.c_str());
		vecOut.push_back(nTmp);
	}
}

void CPublicFunc::Split(const std::string& sStr, const std::string& sSep, std::vector<std::string>& vecOut)
{
	vecOut.clear();
	std::size_t nLast = 0;
	std::size_t nIndex = sStr.find(sSep, nLast);
	while (nIndex != std::string::npos)
	{
		vecOut.push_back(sStr.substr(nLast, nIndex-nLast));
		nLast = nIndex + sSep.length();
		nIndex = sStr.find(sSep, nLast);
	}

	if (nIndex - nLast > 0)
	{
		vecOut.push_back(sStr.substr(nLast,nIndex-nLast));
	}
}