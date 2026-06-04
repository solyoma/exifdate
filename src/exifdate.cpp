// exifdate.cpp : Defines the entry point for the console application.
//
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "targetver.h"
#include "exifdate.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include "safiles.h"
#ifdef _MSC_VER
	#include <Windows.h>
#else
	#include <unistd.h>
#endif
#include <fstream>
#include <cstdio>
#include <string>
#include <exception>
using namespace std;

constexpr int BASE_YEAR = 1900;
Options options;

EXIF_DATE edate;
Deltas deltas;

static bool _isdigit(char ch) { return ch >= '0' && ch <= '9'; }
static char Is2Digits(const char* p) { return _isdigit(*p) && _isdigit(p[1]); }

bool EXIF_DATE::IsValid(Part part) const
{
	bool res = true;
	if (part == Part::pBoth || part == Part::pDate)
		res &= Is2Digits(y) &&
		Is2Digits(y + 2) &&	// year
		Is2Digits(m) &&  // months
		sep1 == sep2 &&
		(sep1 == ':' || sep1 == '-') &&
		Is2Digits(d);	 // days;

	if (res && part == Part::pBoth)
		res &= (sep3 == 'T' || sep3 == ' ');

	if(res && part == Part::pBoth || part == Part::pTime)
		res &= 	Is2Digits(h) &&	// hours
				Is2Digits(n) &&	// minutes
				Is2Digits(s) &&	// seconds
				sep4 == sep5 &&
				sep4 == ':';
	return res;
}

// Deltas class
bool Deltas::SetDateDelta(const string sD) // SD format: YYYY:MM:DD[ HH:mm[:SS]] 
{
	const EXIF_DATE* ped = (const EXIF_DATE*)sD.c_str();

	bool res;
	int len = sD.length();
	if (len > 10)
		res = ped->IsValid(EXIF_DATE::Part::pBoth);
	else
		res = ped->IsValid(EXIF_DATE::Part::pDate);
	if(res && len > 10)
	   st = sD.substr(12);	// after the 'T' or space

	return res;
};

bool Deltas::SetTimeDelta(const string sT) 	 // format: HH[:MM[:SS]]
{
	int l = sT.length();
	if (l != 2 && l != 5 && l != 8)
		return false;
							// reta a full string wjen less is given
	string s = sT;
	if (l = 2) s += ":00:00";
	if (l == 5) s += ":00";

	const char* p = s.c_str();
	bool res = Is2Digits(p)		&&  // hours
			(p[2] == ':')	&&	  
			Is2Digits(p + 3)&&	// minutes
			(p[5] == ':') &&
			Is2Digits(p + 6);	// seconds
	if (res)
		st = s;
	return res;
};


// NumericDate class
void NumericDate::FromSeconds(time_t sec)
{
	struct tm t;
	localtime_s(&t, &sec);
	SetY(t.tm_year + 1900);
	SetM(t.tm_mon + 1);
	SetD(t.tm_mday);
	SetH(t.tm_hour);
	SetN(t.tm_min);
	SetS(t.tm_sec);
}

time_t NumericDate::ToSeconds() const
{
	struct tm t = {};
	t.tm_year = _y - 1900;
	t.tm_mon = _m - 1;
	t.tm_mday = _d;
	t.tm_hour = _h;
	t.tm_min = _n;
	t.tm_sec = _s;
	return mktime(&t);
}

bool NumericDate::IsLeapYear(int year) const
{
	if (year <= 0)
		throw std::runtime_error("Invalid year value");
	return year > 0 && ((year % 4) == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

void NumericDate::Add(Deltas deltas)
{
	if (deltas.sd.empty() && deltas.st.empty())
		_AddDeltas(deltas);
	else
		SetDateTime(deltas);
}

void NumericDate::_AddDeltas(Deltas deltas)
{
	int y = _y + deltas.y,
		m = _m + deltas.m,
		d = _d + deltas.d,
		h = _h + deltas.o,
		n = _n + deltas.n,
		s = _s + deltas.s;
	while (s >= 60) { s -= 60; n++; }
	while (s < 0) { s += 60; n--; }
	while (n >= 60) { n -= 60; h++; }
	while (n < 0) { n += 60; h--; }
	while (h >= 24) { h -= 24; d++; }
	while (h < 0) { h += 24; d--; }
	static const int mdays[2][13] = {
		{0,31,28,31,30,31,30,31,31,30,31,30,31},
		{0,31,29,31,30,31,30,31,31,30,31,30,31}
	};

	while (true)
	{
		int leap = IsLeapYear(BASE_YEAR + y) ? 1 : 0;
		if (d > mdays[leap][m])
		{
			d -= mdays[leap][m];
			if (++m > 12)
			{
				m = 1;
				SetY(Y() + 1);
			}
		}
		else if (d < 1)
		{
			if (--m < 1)
			{
				SetY(Y() - 1);
				if (y < BASE_YEAR)
					throw std::runtime_error("Resulting date is out of valid range");
				_m = 12;
			}
			int leap = IsLeapYear(BASE_YEAR + y) ? 1 : 0;
			_d += mdays[leap][_m];
		}
		else
			break;
	}
	// only set when no errors
	_y = y;
	_m = m;
	_d = d;
	_h = h;
	_n = n;
	_s = s;
	ToExifDate(); // to _exifDate
}

void NumericDate::SetDateTime(Deltas deltas) // but keep the original delimiters
{
	if(!deltas.sd.empty())
	{
		char sep1 = _exifDate.sep1, sep2 = _exifDate.sep2, sep3 = _exifDate.sep3;
		char* pb = _exifDate.buf;
		strncpy_s(pb, sizeof(EXIF_DATE::buf), deltas.sd.c_str(), sizeof(EXIF_DATE::buf) - 1);
		_exifDate.sep1 = sep1;
		_exifDate.sep2 = sep2;
		_exifDate.sep3 = sep3;
	}
	if (!deltas.st.empty())			// length of "23:59:59" w.o. ending 0 is 8
	{
		char* pb = _exifDate.h;
		strncpy_s(pb, 9, deltas.st.c_str(), 8);
	}
	_FromExifDate();
}

void NumericDate::_FromExifDate()	  // 2015:03:09T05:56:33 or 2015-03-09 05:56:33
{
	ValidateExifDate(&_exifDate);	// throws if the date is not valid, otherwise we can be sure that the date and time parts are in the expected format
	auto convertPart = [&](int pos, int size) -> int
		{
			char b[5] = {};
			strncpy_s(b, 5, _exifDate.buf + pos, size);
			b[size] = 0;
			return stoi(b);
		};

	SetY(convertPart(0, 4));
	SetM(convertPart(5, 2));
	SetD(convertPart(8, 2));
	SetH(convertPart(11, 2));
	SetN(convertPart(14, 2));
	SetS(convertPart(17, 2));
}

void NumericDate::ToExifDate(EXIF_DATE *edt)
{
	if (!edt)	// then convert from numeric values, else just copy the buffer, which is already OK
	{
		edt = &_exifDate;

		if (_s < 0 || _s > 59 || _n < 0 || _n > 59 || _h < 0 || _h > 23 || _d < 0 || _d > 31 || _m < 0 || _m > 12 || _y < BASE_YEAR || _y > 2100)
			throw std::runtime_error("Invalid date-time value");
		sprintf_s(edt->buf, sizeof(edt->buf), "%04d%c%02d%c%02d%c%02d:%02d:%02d",
			_y, edt->sep1, _m, edt->sep2, _d, edt->sep3, _h, _n, _s);
	}
	else
		memcpy(edt->buf, _exifDate.buf, sizeof(EXIF_DATE::buf));
}

bool NumericDate::ValidateExifDate(const EXIF_DATE *pED)   // static function. pED must not be nullptr!
{
	if (!pED)
		throw std::runtime_error("Exif date is null");
	// Formats:
	//   Both
	//		2015:03:09T05:56:33 or 2015:03:09 05:56:33 or 
	//		2015-03-09T05:56:33 or 2015-03-09 05:56:33
	//   Date
	//		2015:03:09 or 2015-03-09 
	//	 Time
	//		12:56:33

	if (!pED->IsValid(EXIF_DATE::Part::pBoth))
		return false;// throw std::runtime_error("Invalid EXIF date/time format");
	return true;
}


/*----------- DateModifier class ------ */

int DateModifier::ReadImageFile() // whole file into memory. return 0 if OK, < 0 if error
{
	if (_pbuf != nullptr)		// already read in
		return 0;

#ifdef _MSC_VER
	HANDLE h;
	h = CreateFile(_fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
	{
		DWORD le = GetLastError();
		STRING s;
		// Display the error message and exit the process
		if(le == ERROR_FILE_NOT_FOUND || le == ERROR_PATH_NOT_FOUND)
			s = L("File not found");
		else 
			s = L("Invalid parameter");
		CERR << s << L(" error opening file '") << _fileName << L("'. Quiting\n");
		return -1;
	}
	else
	{
		_fileTimesOk = GetFileTime(h, &_create, &_lasta, &_lastw) != 0;
		DWORD high;
		_size = GetFileSize(h, &high);
		CloseHandle(h);
		if (high)
		{
			CERR << L("File '") << _fileName << L("' is too large! It will not be processed!\n");
			return -1;
		}
	}
#else
	struct _stat st;
	if (_wstat(_name.c_str(), &st))
	{
		STRING s;
		if (errno == ENOENT)
			s = L("File not found");
		else if (errno == EINVAL)
			s = L("Invalid parameter");
		CERR << s << L(" error opening file '") << _name << L("'. Quiting\n");
		return -1;
	}
	_size = st.st_size;

	// read all of file in memory
#endif
	ifstream ifs(_fileName.c_str(), ios_base::binary);		// original file
	_pbuf = new char[_size];	// size must be smaller than 4 Gig!
	_pose = _pbuf + (_size - 14);	// need at least 14 bytes for date, so no need to look after this
	bool ok = !ifs.read(_pbuf, _size).fail();
	if (!ok)	// error reading file
	{
		CERR << L("Error reading file '") << _fileName << L("'. Qutiing\n");
		return -2;
	}
	ifs.close();
	return 0;
}

int DateModifier::Modify(bool isSave)	// if isSave == false  only display original EXIF date but do not create new file
{									// EXPECTS: file already read in memory (_pbuf)
	int found = _ModifyFile(isSave);	// get all flags
	if (!found)
	{
		CERR << L("No EXIF date found in file.\n");
		return -31;
	}

	if (!isSave)
		return found;

	ofstream ofs(_newName.c_str(), ios_base::binary);	// new file
	ofs.write(_pbuf, _size);		// first SIZE data: all file was read in!
	bool isOk = ofs.good();			// write successfull
	ofs.close();

	STRING s = _fileName + L("~");
	RENAME(_fileName.c_str(), s.c_str());
	RENAME(_newName.c_str(), _fileName.c_str());
	// set file date and time to the original
#ifdef _MSC_VER
	HANDLE h;
	h = CreateFile(_fileName.c_str(), GENERIC_WRITE | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (SetFileTime(h, &_create, &_lasta, &_lastw) == 0)	// OK
	{
		CERR << "Error setting file time to'" << _fileName << "'\n";
	}
	CloseHandle(h);
#else
	struct stat stat;
	stat(_name.c_str(), &stat);
	stat.st_ctime = _create.dwLowDateTime;	// this is not exactly correct, but it is the best we can do in Linux
	stat.st_atime = _lasta;
	stat.st_mtime = _lastw;
	 // ???
#endif
	if (!options.keepOld)
	{
		REMOVE(s.c_str());
	}

	return found;
}

bool DateModifier::_GetExifDate(bool first)	 // from buffer pointed by _pDate
{
	_ped = (EXIF_DATE*)_pDate;
	if (!NumericDate::ValidateExifDate(_ped))
		return false;

	_actDate.Setup(_ped);
	if (first)
		_firstDate.Setup(&_actDate.ExifDate());

	return true;
}

void DateModifier::_SetExifDate()  // back into pDate in memory
{
	memcpy(_pDate, _actDate.ExifDate().buf, sizeof(EXIF_DATE::buf));
}

int DateModifier::_ModifyFile(bool getall)	// find from pos in pb and modify it
{
	int found = 0;				//  no date found yet in file
	_pDate = _pbuf;
	// get first EXIF date string, modify it and save both the original and the modified one

	while (!((found = _GetExifDate(true))))	// else '_pDate' into '_firstDate' and '_actDate' if it is valid
		++_pDate;

	if (!found && _pDate >= _pose - sizeof(EXIF_DATE) + 1) // then no EXIF date string in file
		return found;
	try
	{
		// first exif date found, now modify it
		_actDate.Add(deltas);	  // it throws on error
		
		memcpy(_pDate,				_actDate.ExifDate().buf, sizeof(EXIF_DATE::buf));	// write it in memory when OK
		memcpy(_modifiedDateTime,	_actDate.ExifDate().buf, sizeof(EXIF_DATE::buf));	// store it for successive dates

		_pDate += sizeof(EXIF_DATE);

		// now go through the file and change all, but only when they are the same as the first!

		while (_pDate < _pose - sizeof(EXIF_DATE) + 1)
		{
			if (_GetExifDate(false)) // then exif date found: modify it
			{
				if (_actDate == _firstDate)
				{
					++found;
// instead of recalculating, just reuse
// 					_actDate.Add(deltas);	  // it throws on error
//					memcpy(_pDate, _actDate.ExifDate().buf, sizeof(EXIF_DATE::buf));	// write it in memory when OK
					memcpy(_pDate, _modifiedDateTime, sizeof(EXIF_DATE::buf));	// write it in memory when OK
				}
			}
			_pDate += sizeof(EXIF_DATE);
		}
		return found;
	}
	catch (std::runtime_error err)
	{
		return false;
	}
}


// EXIF date output
ostream &operator<<(ostream &os, DateModifier &md)
{
	os << "File: '" << md._fileName << "' dates " << md._firstDate.ExifDate().buf << " => " << md._modifiedDateTime << "\n";
	return os;
}
