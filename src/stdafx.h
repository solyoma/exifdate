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
#ifdef _MSC_VER
	#include <tchar.h>
	#include <Windows.h>
	#include <WinBase.h>
	#include <fileapi.h>	// GetFileTime
	#include <io.h>
#endif

// TODO: reference additional headers your program requires here

#ifdef _UNICODE
	typedef std::wstring STRING;
	#define ostream	wostream
	#define istream wistream

	#define main(a,b)	_tmain(a,b)
	#define REGEX	wregex
	#define TO_STRING(a)	to_wstring(a)
	#define COUT std::wcout
	#define CERR std::wcerr
	#define L(a)	L##a
	#define MKDIR(a) _wmkdir(a)
	#define UNLINK(a) _wunlink(a)
	#define RENAME(a,b) _wrename((a),(b))
	#define REMOVE(a) _wremove((a))
	#define STRTOL(a,b,c) wcstol((a),(b),(c))
	#ifndef TCHAR
		#define CHAR wchar_t
	#else
		#define CHAR TCHAR
	#endif
	//#define STRCPY(a,b) wcscpy((a),(b))
	//#define STRCHR(a,b) wcschr((a),(b))
	//#define STRRCHR(a,b) wcsrchr((a),(b))
	//#define STRNCPY(a,b) wcsncpy((a),(b))
	//#define STRLEN(a) wcslen(a)
#else
	typedef std::string STRING;
	typedef std::cout COUT
	#define TO_STRING(a)	to_string(a)
	#define COUT std::cout
	#define CERR std::cerr
	#define L(a)	a
	#ifndef TCHAR
		#define CHAR wchar_t
	#else
		#define CHAR TCHAR
	#endif
	#define RENAME(a,b) _rename((a),(b))
	#define REMOVE(a) _remove((a))
	#define REGEX		regex
	#define MKDIR(a) _mkdir(a)
	#define UNLINK(a) _unlink(a)
	#define STRTOL(a,b,c) wcstol((a),(b),(c))
#endif

STRING NormalizePath(STRING &str, bool isDir = false);
