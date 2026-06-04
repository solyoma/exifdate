// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <sys/stat.h>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here

#ifdef _UNICODE
	typedef std::wstring STRING;
	#define ostream	wostream
	#define istream wistream
	#define main(a,b)	_tmain(a,b)
	#define TO_STRING(a)	to_wstring(a)
	#define  COUT std::wcout
	#define  CERR std::wcerr
	#define STR(a)	L##a
	#define _mkdir(a) _wmkdir(a)
	#define _unlink(a) _wunlink(a)
	#define rename(a,b) _wrename((a),(b))
	#define remove(a) _wremove(a)
	#ifndef TCHAR
		#define TCHAR wchar_t
	#endif
	#define strcpy(a,b) wcscpy((a),(b))
	#define strchr(a,b) wcschr((a),(b))
	#define strrchr(a,b) wcsrchr((a),(b))
	#define strncpy(a,b) wcsncpy((a),(b))
	#define strlen(a) wcslen(a)
#else
	typedef std::string STRING;
	typedef std::cout COUT
	#define TO_STRING(a)	to_string(a)
	#define  COUT std::cout
	#define  CERR std::cerr
	#define STR(a)	a
	#ifndef TCHAR
		#define TCHAR char 
	#endif
#endif

STRING NormalizePath(STRING &str, bool isDir = false);
