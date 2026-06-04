// stdafx.cpp : source file that includes just the standard includes
// exifdate.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

STRING NormalizePath(STRING &str, bool isDir)
{
	for (size_t i = 0; i != str.length(); ++i)
		if (str[i] == '\\')
			str[i] = '/';
	if (isDir && str[str.length() - 1] != '/')
		str += STR("/");
	return str;
}
