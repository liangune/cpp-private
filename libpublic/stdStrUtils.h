/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   字符串操作
*************************************************************************/

#ifndef _STDSTRUTILS_H_
#define _STDSTRUTILS_H_
#include <string>
#include <stdarg.h>
#include <vector>
#include <map>
#include <set>

typedef std::vector<std::string > StrVec;
typedef std::vector<int> IntVec;
typedef std::set<std::string > StrSet;

class StdStrUtils
{
	static const int FORMAT_BLOCK_SIZE = 2048;
 public:
	static std::string  Int2Str(int64_t n);
	static int64_t Str2Int(const std::string &sInt);
	static std::string Format(const char *fmt, ...);
	static std::string FormatV(const char *fmt, va_list args);
	static void ToUpper(std::string &str);
	static void ToLower(std::string &str);
	static int ToUpper(int c);
	static int ToLower(int c);
	static bool EqualsNoCase(const std::string  &str1, const std::string &str2);
	static std::string Left(const std::string  &str, size_t count);
	static std::string Mid(const std::string &str, size_t first, size_t count = std::string::npos);
	static std::string Right(const std::string &str, size_t count);
	static std::string& Trim(std::string &str);
	static std::string& TrimLeft(std::string &str);
	static std::string& TrimRight(std::string &str);
	static std::string TrimString(const std::string& str, const std::string& drop);
	static int Replace(std::string &str, char oldChar, char newChar);
	static int Replace(std::string &str, const std::string &oldStr, const std::string &newStr);
	static bool StartsWith(const std::string &str, const std::string &str2, bool useCase = false);
	static bool EndsWith(const std::string &str, const std::string &str2, bool useCase = false);
	static int SplitString(const std::string& str, StrVec& items, const std::string& splitter);
	// 过滤空格、tab的分割字符函数
	static int SplitStringTrim(const std::string& str, StrVec& items, const std::string& splitter);
	static int SplitStringToInt(const std::string& str, IntVec& items, const std::string& splitter);
    
 public:
	static char * HtmlEscaping(char *src);
	static char * Htmlize(char *src);
	static char * DeHtmlize(char *src);
};
#endif /* _STDSTRUTILS_H_ */
