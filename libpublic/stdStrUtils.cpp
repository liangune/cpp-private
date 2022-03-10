/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   字符串操作
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <vector>
#include <functional>
#include "stdStrUtils.h"

using namespace std;

string StdStrUtils::Int2Str(int64_t nInt)
{
	std::stringstream ss;
	string str;
	ss << nInt;
	ss >> str;
	return str;
}

int64_t StdStrUtils::Str2Int(const string &sInt)
{
	return strtoll(sInt.c_str(), NULL, 10);
}
string StdStrUtils::Format(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	string str = FormatV(fmt, args);
	va_end(args);

	return str;
}

string StdStrUtils::FormatV(const char *fmt, va_list args)
{
	if (fmt == NULL)
		return "";

	int size = FORMAT_BLOCK_SIZE;
	va_list argCopy;

	char *cstr = reinterpret_cast<char*>(malloc(sizeof(char) * size));
	if (cstr == NULL)
		return "";

	while (1)
	{
		va_copy(argCopy, args);

		int nActual = vsnprintf(cstr, size, fmt, argCopy);
		va_end(argCopy);

		if (nActual > -1 && nActual < size) // We got a valid result
		{
			string str(cstr, nActual);
			free(cstr);
			return str;
		}
		if (nActual > -1)                   // Exactly what we will need (glibc 2.1)
			size = nActual + 1;
		else                                // Let's try to double the size (glibc 2.0)
			size *= 2;

		char *new_cstr = reinterpret_cast<char*>(realloc(cstr, sizeof(char) * size));
		if (new_cstr == NULL)
		{
			free(cstr);
			return "";
		}

		cstr = new_cstr;
	}

	return "";
}

void StdStrUtils::ToUpper(string &str)
{
	transform(str.begin(), str.end(), str.begin(), ::toupper);
}

void StdStrUtils::ToLower(string &str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}

int StdStrUtils::ToUpper(int c)
{
	if (c >= 'a' && c <= 'z')
		c ^= 0x20;
	return c;
}

int StdStrUtils::ToLower(int c)
{
	if (c >= 'A' && c <= 'Z')
		c ^= 0x20;
	return c;
}

bool StdStrUtils::EqualsNoCase(const string &str1, const string &str2)
{
	string tmp1 = str1;
	string tmp2 = str2;
	ToLower(tmp1);
	ToLower(tmp2);

	return tmp1.compare(tmp2) == 0;
}

string StdStrUtils::Left(const string &str, size_t count)
{
	count = max((size_t)0, min(count, str.size()));
	return str.substr(0, count);
}

string StdStrUtils::Mid(const string &str, size_t first, size_t count /* = string::npos */)
{
	if (first + count > str.size())
		count = str.size() - first;

	if (first > str.size())
		return string();

	assert(first + count <= str.size());

	return str.substr(first, count);
}

string StdStrUtils::Right(const string &str, size_t count)
{
	count = max((size_t)0, min(count, str.size()));
	return str.substr(str.size() - count);
}

string& StdStrUtils::Trim(string &str)
{
	TrimLeft(str);
	return TrimRight(str);
}

string& StdStrUtils::TrimLeft(string &str)
{
	str.erase(str.begin(), ::find_if(str.begin(), str.end(), ::not1(::ptr_fun<int, int>(::isspace))));
	return str;
}

string& StdStrUtils::TrimRight(string&str)
{
	str.erase(::find_if(str.rbegin(), str.rend(), ::not1(::ptr_fun<int, int>(::isspace))).base(), str.end());
	return str;
}

string StdStrUtils::TrimString(const string& str, const std::string& drop)
{
	std::string r = str;
	// trim right
	r = r.erase(str.find_last_not_of(drop)+1);
	// trim left
	return r.erase(0,r.find_first_not_of(drop));
}

int StdStrUtils::Replace(string &str, char oldChar, char newChar)
{
	int replacedChars = 0;
	for (string::iterator it = str.begin(); it != str.end(); it++)
	{
		if (*it == oldChar)
		{
			*it = newChar;
			replacedChars++;
		}
	}

	return replacedChars;
}

int StdStrUtils::Replace(string &str, const string &oldStr, const string &newStr)
{
	int replacedChars = 0;
	size_t index = 0;

	while (index < str.size() && (index = str.find(oldStr, index)) != string::npos)
	{
		str.replace(index, oldStr.size(), newStr);
		index += newStr.size();
		replacedChars++;
	}

	return replacedChars;
}

bool StdStrUtils::StartsWith(const std::string &str, const std::string &str2, bool useCase /* = false */)
{
	std::string left = StdStrUtils::Left(str, str2.size());

	if (useCase)
		return left.compare(str2) == 0;

	return StdStrUtils::EqualsNoCase(left, str2);
}

bool StdStrUtils::EndsWith(const std::string &str, const std::string &str2, bool useCase /* = false */)
{
	std::string right = StdStrUtils::Right(str, str2.size());

	if (useCase)
		return right.compare(str2) == 0;

	return StdStrUtils::EqualsNoCase(right, str2);
}

int StdStrUtils::SplitString(const std::string& str, StrVec& items, const std::string& splitter)
{
	std::string src_str = TrimString(str, splitter);
	int pos = 0;
	while (1)
	{
		if (src_str == splitter || src_str.empty())
		{
			break;
		}
		pos = src_str.find(splitter);
		items.push_back(src_str.substr(0, pos));
		if (pos == (int)string::npos)
		{
			break;
		}
		src_str = src_str.substr(pos+splitter.size());
		src_str = TrimString(src_str, splitter);
	}
	return items.size();
}

int StdStrUtils::SplitStringTrim(const string& str, StrVec& items, const string& splitter)
{
	items.clear();
	StrVec vFields;
	int iCnt = StdStrUtils::SplitString(str, vFields, splitter);
	for (int i = 0; i < iCnt; ++i)
	{ 
		string str = StdStrUtils::Trim(vFields[i]);
		items.push_back(str);
	}

	return items.size();
}

int StdStrUtils::SplitStringToInt(const string& str, IntVec& items, const string& splitter)
{
	items.clear();
	StrVec vFields;
	int iCnt = StdStrUtils::SplitString(str, vFields, splitter);
	for (int i = 0; i < iCnt; ++i)
	{
		items.push_back( Str2Int(StdStrUtils::Trim(vFields[i])) );
	}

	return items.size();
}

char * StdStrUtils::HtmlEscaping(char *src)
{
	int olen = strlen(src);
	char  *p = src, *d, *res;
	int specials = 0;

	if ( !src ) return(NULL);
	while( p && *p ) {
		if ( *p == '<' || *p == '>' || *p == '\"' || *p == '&' )
		specials++;
		p++;
	}
	res = (char *)malloc(strlen(src) + 1 + specials*5 ); /* worst case */
	if ( !res ) return(NULL);

	if ( specials == 0 ) {
		memcpy(res, src, olen+1);
		return(res);
	}
	p = src;
	d = res;
	while ( *p ) {
		if ( *p == '<' ) {
			strcpy(d,"&lt;");d+=3;
		} else
		if ( *p == '>' ) {
			strcpy(d,"&gt;");d+=3;
		} else
		if ( *p == '\"' ) {
			strcpy(d,"&quot;"); d+=5;
		} else
		if ( *p == '&' ) {
			strcpy(d,"&amp;"); d+=4;
		} else
		*d = *p;
		p++;d++;
	}
	*d = 0;
	return(res);
}

char * StdStrUtils::Htmlize(char *src)
{
	char  *res;
	unsigned char *s = (unsigned char*)src, *d;
	unsigned char xdig[17] = "0123456789ABCDEF";

	res = (char *)malloc(strlen(src) * 3 + 1 ); /* worst case */
	if ( !res ) return(NULL);
	d = (unsigned char*)res;
	while( *s ) {
		if ( *s!='/' &&
				*s!='.' &&
				*s!='-' &&
				*s!='_' &&
				*s!='~' &&
				((*s >= 0x80) || (*s <= 0x20) || !isalnum(*s) ) ) {
			*d++ = '%';
			*d++ = xdig[ (*s) / 16 ];
			*d   = xdig[ (*s) % 16 ];
		} else
		*d = *s;
		d++; s++;
	}
	*d=0;
	return(res);
}

#define HEXTOI(arg) (((arg)<='9')? ((arg)-'0'):(tolower(arg)-'a' + 10))
char * StdStrUtils::DeHtmlize(char *src)
{
	char  *res;
	unsigned char *s = (unsigned char*)src, *d;

	// res = (char *)xmalloc(strlen(src) + 1, "dehtmlize(): dehtmlize"); /* worst case */
	res = (char*)malloc(strlen(src) + 1);
	if ( !res ) return(NULL);
	d = (unsigned char*)res;
	while( *s ) {
		if ( (*s=='%') && isxdigit(*(s+1)) && isxdigit(*(s+2)) ) {
			*d = (HEXTOI(*(s+1)) << 4) | (HEXTOI(*(s+2)));
			s+=2;
		} else
		*d = *s;
		d++; s++;
	}
	*d=0;
	return(res);
}