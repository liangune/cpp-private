#ifndef _EXCEPTION_DUMP_H_
#define _EXCEPTION_DUMP_H_

#ifdef _MSC_VER

#include <Windows.h>
#include <dbghelp.h>

#pragma comment(lib,"dbghelp.lib")

class CExceptionDump
{
public:
	CExceptionDump();
	~CExceptionDump();

	static LONG UnhandledException(EXCEPTION_POINTERS *pException);
};

#endif
#endif // !_EXCEPTION_DUMP_H_
