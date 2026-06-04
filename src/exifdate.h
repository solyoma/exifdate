#pragma once
#include <corecrt.h>
#include <Windows.h>
#include <string>
#include <string.h>
#include <stdexcept>
#include <cstdlib>

extern const int BASE_YEAR;
////////////////////////////////
template <class T> T sa_abs(T a) { return (a > 0 ? a : -a); }

//--------------------- Options class --------
// global options for exif time change from the command line.
struct Options
{
	bool keepOld = false;
	bool listDateAndTime = false;
	char dateDelim=0,	// delimiter between date section (':' or '-' 2025:03:04 or 2025-03-04) 0: not set up yet
		 dtDelim=' ',	// delimiter between date and time: either space of 'T'
		 ech='\0';	    // character after exif date STRING
};
//--------------------- Deltas class --------
// Contains the parameters from the command line for date/time change.
class Deltas
{
public:
	int d = 0, m = 0, y = 0, o = 0, n = 0, s = 0; // +- deltas for day, month, year, hour, minute, second
	std::string sd, st;								  // date and time to set into exif

	void Reset() { d = m = y = o = n = s = 0; sd.clear(); st.clear(); }
	bool SetDayDelta(int sign, const CHAR* p) { d = sign * STRTOL(p, nullptr, 10); return true; }
	bool SetMonthDelta(int sign, const CHAR* p) { m = sign * STRTOL(p, nullptr, 10); return true; }
	bool SetYearDelta(int sign, const CHAR* p) { y = sign * STRTOL(p, nullptr, 10); return true; }
	bool SetHourDelta(int sign, const CHAR* p) { o = sign * STRTOL(p, nullptr, 10); return true; }
	bool SetMinuteDelta(int sign, const CHAR* p) { n = sign * STRTOL(p, nullptr, 10); return true; }
	bool SetSecondDelta(int sign, const CHAR* p) { s = sign * STRTOL(p, nullptr, 10); return true; };
	bool SetDateDelta(const std::string sD); // SD format: YYYY:MM:DD[ HH:mm[:SS]] 
	bool SetTimeDelta(const std::string sT); // format: HH:MM[:SS]
};

// packed structure for EXIF date and time string in memory.
#pragma pack(push,1)
union EXIF_DATE
{
	struct
	{
		char y[4],		// year
			sep1,		// :
			m[2],		// month
			sep2,		// :
			d[2],		// day
			sep3,		// space or 'T'
			h[2],		// hour
			sep4,		// :
			n[2],		// minute
			sep5,		// :
			s[2],		// second
			ech;		// ending null character
	};
	char buf[20];

	EXIF_DATE() 
	{ 			// "0000:00:00T00:00:00"
		memset(buf, '0', sizeof(buf)-1); 
		sep1 = sep2 = ':';
		sep3 = 'T';
		sep4 = sep5 = ':';
		ech = 0;
	}

	enum class Part {pDate, pTime, pBoth};
	bool IsValid(Part part) const;
	EXIF_DATE operator=(const EXIF_DATE& o)
	{
		memcpy(buf, o.buf, sizeof(buf));
		return *this;
	}
	bool operator==(const EXIF_DATE o)
	{
		return !strncmp(buf, o.buf, sizeof(buf));
	}
};
#pragma pack(pop)	// back to default

//////////// globals ////////////////////
extern Options options;
extern Deltas deltas;

//--------------------- NumericDate class --------
// converts from and to an EXIF_DATE structure. The date and time are stored 
// in separate fields and can be modified separately. When converting back to 
// EXIF_DATE the validity of the date and time is checked. The year is stored 
// as an offset from BASE_YEAR (e.g. 2025 is stored as 25 if BASE_YEAR is 2000). 
// This allows to store years up to 4095 (BASE_YEAR + 4095) in a 12 bit field.
class NumericDate 
{
public:
	NumericDate() {}
	NumericDate(EXIF_DATE &exifDate) :_exifDate(exifDate)  { _FromExifDate(); }
	void Setup(const EXIF_DATE *pExifDate) { _exifDate = *pExifDate; _FromExifDate(); }

	EXIF_DATE &ExifDate()  { return _exifDate; }

	int Y() const { return _y; }
	int M() const { return _m; }
	int D() const { return _d; }
	int H() const { return _h; }
	int N() const { return _n; }
	int S() const { return _s; }

	void SetY(int v) { _y = v & 0xFFF; }
	void SetM(int v) { _m = v & 0x0F; }	
	void SetD(int v) { _d = v & 0x3F; }
	void SetH(int v) { _h = v & 0x3F; }
	void SetN(int v) { _n = v & 0x3F; }
	void SetS(int v) { _s = v & 0x3F; }

	void FromSeconds(time_t sec);
	time_t ToSeconds() const;
	void SetDateTime(Deltas deltas);
	void Add(Deltas deltas); // after it _exifDate is ok too
	bool IsLeapYear(int year) const;
	void ToExifDate(EXIF_DATE *edt = nullptr); // nullptr => into _exifDate

	bool operator==(NumericDate other) { return _exifDate == other._exifDate; }

	static bool ValidateExifDate(const EXIF_DATE *pED);
private:
	int _y=-1, _m=-1, _d=-1, _h=-1, _n=-1, _s=-1;  // invalid values
	EXIF_DATE _exifDate;

	void _FromExifDate();
	void _AddDeltas(Deltas deltas);
};

//--------------------- DateModifier class --------
// Modifies the EXIF dates in one file. The file is read into memory 
// and the EXIF date string is modified in memory. If option -l is 
// not given then the modified file is written to disk under a 
// temporary name and if the write is successful the original file 
// is renamed with a tilde (~) appended to its name. 
// If option -e is given then the original file is deleted. Finally 
// the new file is renamed to the original file name and its file 
// dates are set to that of the original.
class DateModifier
{
public:
	friend std::ostream &operator<<(std::ostream &os, DateModifier &md);

	DateModifier(STRING fileName) : _fileName(fileName)
	{ 
		int pos = _fileName.find_last_of('.');

		_newName = _fileName.substr(0, pos) + L("X") + _fileName.substr(pos);
		options.dateDelim = 0;	// not set up yet for this file
	}
	~DateModifier() { delete[] _pbuf; }

	int ReadImageFile(); // whole file into memory. return 0 if OK, < 0 if error
	int Modify(bool doSave = true);	// !doSave then only display original EXIF date but do not create new file

private:
	NumericDate _firstDate, _actDate;

	char _modifiedDateTime[20];	// the first modification comes here and be copied to next values when they are the same

	STRING _fileName, _newName;
	bool _fileIsRead = false;	// file is read into memory?
	long _size=0;		// file size

	char *_pbuf = nullptr,	// file data is read to here
		 *_pDate = nullptr, // points to actual date time string in pbuf
		 *_pose = nullptr;  // points AFTER EOF in memory
	EXIF_DATE* _ped = 0;	// points into the exif date string in pbuf

	FILETIME _create{ 0,0 }, _lasta{ 0,0 }, _lastw{ 0,0 };
	bool _fileTimesOk=false;

	bool _GetExifDate(bool first = false); // if first use 'firstDT' else use 'edtsh'
	void _SetExifDate();					// back from _actDate into pDate in memory


	int _ModifyFile(bool getall = true);	// find from pos in pb and modify it getall = doSave for Modify
};
