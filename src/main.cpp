#include "stdafx.h"
#include "targetver.h"
#include <locale>
#include "exifdate.h"
#include "safiles.h"

using namespace std;

//---------------------------------------------------------------------------

static CHAR szProgramName[144];
static CHAR szProgramDir[144];

/*======================================================================*/
static STRING GetProgramName(TCHAR *pszPath)
/*----------------------------------------------------------------------*/
{
	STRING s = pszPath;
	for (size_t i = 0; i < s.length(); ++i)
		if (s[i] == '\\')
			s[i] = '/';
	size_t pos = s.find_last_of('/');
	if (pos != s.npos)
		s = s.substr(pos + 1);

	return s;
}
/*======================================================================*/
static int AlgoEng(STRING sName)
/*----------------------------------------------------------------------*/
{
	COUT << sName << "\n"
//				   1         2		   3		 4		   5		 6		   7
//       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
		"Modern cameras and scanners embed the creation date and time in the image files\n"
		"These are called EXIF dates. Sometimes the dates recorded in the file are wrong\n"
		"(e.g. you forgot to set the correct date and time in the camera, or you want\n"
		"the EXIF date to reflect the date/time of the original scanned material). This\n"
		"program helps you with these problems. The program expects the file to contain\n"
		"valid EXIF date and time data (usually in more than one position) in one of\n"
		"the following ASCII formats:\n"								   
		"		\"YYYY-MM-DDTHH:MM:SS\" or \"YYYY:MM:DD HH:MM:SS\"\n"					   
		"If a file does not contain any such date and time information it will not be\n"
		"modified. There are some special helper files (e.g. .XMP 'sidecar' files)\n"
		"which contain more than one EXIF date and time. For such files only those\n"
		"EXIF dates and times, which are the same as the first one in the file,\n"
		"are modified.\n"   
		"ALGORITHM: 1. Read in the parameters\n"				   
		"           2. Read in the list of all files specified at the command line.\n"
		"           3. For all files in list\n"											   
		"              3.1 Read whole file in memory completely and scan for first EXIF\n"
		"                  format date and time embedded in it. \n"							   
		"                  If no EXIF date found found move to the next file"
		"              3.2.If found save as reference date & time for this file. (EXIF\n"
		"                  dates different from the reference will not be modified).\n"	   
		"              3.3. Determine new EXIF date and time for the file\n"
		"                a. If option '-t' was given replace EXIF date and/or time from\n"
		"                   it\n"
		"                b. If option '-tt' was set replace EXIF time with it\n"					   
		"                c. Add year, month, day, etc offsets to the EXIF date and time.\n"
		"                d. Check the result if it is a valid date and time\n"
		"                e. If not move to the next file.\n"
		"                f. If option -l is used echo the modified EXIF date in the\n"   
		"                   terminal, and continue with the next file\n"
		"                g, Replace  all EXIF dates and times of the actual file which\n"
		"                   are the same as the reference with the one calculated in\n"					   
		"                   step 3.3,using thecorrect form.\n"
		"              3.4. If option -l was not given write new file to disk under a\n"
		"                   temporary name. The original file does not change.\n"
		"              3.5. If the write is successfull rename the original file: append\n"
		"                   a tilde (~) to the name\n"
		"              3.6. If the -e option was given delete the original file\n"		   
		"              3.6. Rename the new file to the original file name and set\n"	   
		"                   the file dates to that of the original.\n"
		"(C) András Sólyom 2015-2016\n";
//				   1         2		   3		 4		   5		 6		   7
//       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
		return(1);
}

/*======================================================================*/
static int AlgoHu(STRING sName)
/*----------------------------------------------------------------------*/
{
	COUT << sName << "\n"
//				   1         2		   3		 4		   5		 6		   7
//       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
		"A működés alapelve: a fájlnak van EXIF fejléce és abba a dátum és idő be van\n"  
		"írva, akár többször is a következő formátumok egyikében:\n"					  
		"		\"ÉÉÉÉ-HH-NNTÓÓ:PP:SS\" or \"ÉÉÉÉ:HH:MM ÓÓ:PP:SS\"\n"					  
		"Ha egy fájlban ilyenek nincsenek, akkor a program azokat nem módosítja! Vannak\n"
		"olyan kép segédfájlok (Pl. .XMP fájlok), amelyekben több különböző dátum is van\n"
		"Ezek közül csak a kép készítésének dátumai íródnak át, a módosítás dátumai nem.\n\n"
		"ALGORITMUS: 1. Beolvassuk a változtatás paramétereit\n"	  
		"            2. beolvassuk az összes megadott fájl adatait egy listába.\n"		  
		"            3. A beolvasott fájl listában szereplő minden fájlra:\n"			  
		"              3.1. Beolvassuk a teljes fájlt és keresünk benne EXIF formájú\n"
		"                   dátumokat.\n"
		"              3.2. Ha találunk megjegyezzük az elsőt referenciának. Csak az ezzel\n"
		"                   megegyező dátumokat módosítjuk\n"
		"                a. Ha -t meg volt adva, akkor lecseréljük a dátumot és\n"		  
		"                    (feltételesen) az időt a -t-vel megadottra\n."				  
		"                b. Ha -tt volt megadva, akkor lecseréljük az időt a -tt -belire\n."
		//                                                         1990.01.01. 00:00:00      
		"                c. Ha volt módosító adat megadva azt hozzáadjuk vagy levonjuk a\n"
		"                   külön-külön számolva az évet, hónapot, stb.\n"							  
		"                d. Ha az eredmény hibás dátum lesz hibaüzenetet írunk ki és\n"	  
		"                   folytatjuk a következő fájllal\n"
		"                f. Ha a -l opciót megadtuk a módosított dátumot kiíratjuk a\n"
		"                   képernyőre és nem folytatjuk az EXIF dátumok keresését, csak\n"
		"                   áttérünk a következő fájlra.\n"
		"              3.3  Ha nem volt -l, akkor a móódosított fájlt egy ideiglenes\n"
		"                   névvel kiírjuk a diszkre. Az eredeti fájl nem változik.\n"			  
		"              3.4. Ha a kiírás sikeres átnevezzük az eredeti fájlt: hozzáadunk\n"
		"                   egy ~ karaktert a nevéhez\n"										  
		"              3.5. Ha a -n opciót is megadtuk az eredeti fájlt letöröljük.\n"	   
		"              3.6. Átnevezzük az új fájlt a régi nevére és a fájl dátumát is\n"
		"                    átírjuk az eredetiére.\n"
		"(C) Sólyom András 2015-2016\n"
		;

	return 1;
}

/*======================================================================*/
static int UsageHu(STRING sName)
/*----------------------------------------------------------------------*/
{
	COUT << sName << "\n"
		//				   1         2		   3		 4		   5		 6		   7
		//       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
		"Modern kamerák és telefonok a felvételkészítés időpontját is rögzítik a kép\n"
		"fájlokban, az ún. EXIF fejlécben. Modern lapbeolvasók is megteszik ezt a\n"
		"beolvasás dátumával. Ha a dátum/idő hibásan volt beállítva a kamerában akkor\n"
		"hibás dátum kerül a képekbe is.\n"
		"Hasonlóképpen át akarhatjuk állítani egy beolvasott fotóban tárolt dátumot az\n"
		"eredeti fotó készítésének dátumára\n"
		"Ezzel a programmal módosíthatjuk a kép fájlok fejlécében található dátumot és\n"
		"időt.\n\n"
		"Jelmagyarázat:\n"
		"    szöveg <> -ben: kötelező paraméter megnevezése,\n"
		"    szöveg []-ben nem kötelező paraméter megnevezése.\n"
		"    A | jel két oldalán megadott paraméterek bármelyike megadható, de csak az\n"
		"     egyik.\n"
		"    Például: -d|D<napok száma> azt jelenti, hogy a napok számát megadhatjuk\n"
		"     vagy a -d vagy a -D opcióval. Ha a dátumot 5 nappal korábbra akarjuk\n"
		"     állítani, akkor ehhez a -D5 opciót kell használjuk.\n"
		"    A | jelet soha nem írjuk be!"
		"A program használata:\n"
		<< sName << " [opciók] <fájl, vagy mappa név> [<fájl, vagy mappa név> [...] ]\n"
		" Alap opciók:\n"
		" -h[h|e]: súgó kiíratás a gép területi beállítása szerint angol vagy magyar\n"
		"           nyelven. A []-ben levő opciókat akkor használjuk, ha nem a területi\n"
		"          beállítások szerinti nyelvet akarjuk használni (-hh magyar,-he angol)\n"
		" -ha[e|h] algoritmus súgójának kiíratása.\n"
		" -e: ne tartsa meg az eredeti fájlokat. Ha nem adjuk meg, akkor az eredeti\n"
		"     fájlokat a program nem törli le, csak egy ~ karaktert ragaszt az eredeti\n"
		"     név végére."
		"     FIGYELEM: *** NEM vállalunk semmilyen felelősséget *** azért, hogy a\n"
		"     módosított fájlok továbbra is használhatóak lesznek, úgyhogy ezt az opciót\n"
		"     mindenki csk a saját felelősségére használja!\n"
		" -r: reguláris kifejezések használata a fájlok keresésénél. Ha nem tudja mik\n"
		"     azok, ne használja ezt az opciót!\n"
		"     Fájl nevekben (mappa nevekben nem!) -r nélkül is használhatja a szokásos\n"
		"     (*, ?, stb) dzsókereket, pl. \"*.jpg\"\n"
		" -l: csak listázza ki az új dátumokat de ne módosítsa a fájlokat\n\n"
		"<fájl vagy mappa név> : lehet egy fájl neve, egy mappa neve, vagy egy mappán\n"
		"     belül több, adott név mintának megfelelő fájl.\n\n"

		"Módosítási opciók: (dátum és idő változtatása):\n"
		" -t<dátum>[<idő>]: alap dátum és idő beállítása. A többi opció ezt módosítja\n"
		"    Formátum: ÉÉÉÉHHNN[ÓÓPP[SS]]\n"
		" -tt<idő>       : csak az idő (ÓÓPPSS) beállítása. A dátum változatlan marad.\n\n"
		" A továbbiakban az opció előtti '+', ill '-' a <paraméter> értékének dátumhoz\n"
		" ill. időhöz hozzáadását, illetve levonását jelenti, vagyis +d12 hozzáad,\n"
		" -d12 levon 12 napot. Ha a végdátum emiatt érvénytelenné válna, akkor"
		"  az aktuális fájl nem módosul. (pl. eredeti dátum 2015.02.27, hozzáadunk 2\n\n"
		"  napot érvénytelen dátumot kapunk, míg, ha az eredeti dátum 2016.02.27, akkor\n"
		"  az eredmény 2015.02.29. lesz.)\n"
		"  Nem lehet betűköz az opció és a szám között!\n"
		" A '+|-' az opciós betű előtt a szám előjelét adja meg." 
		" +|-d<napok száma> \n"
//		" +|-m<hónapok száma>\n"
//		" +|-y<évek száma>\n"
		" +|-o<órák száma>\n"
		" +|-n<percek száma>\n"
		" +|-s<másodpercek>\n\n"
//				   1         2		   3		 4		   5		 6		   7
//       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
		"Egyidejűleg több opciót is használhatunk.\n"
		"Ha egy opció többször szerepel, mindig az utolsó az érvényes.\n"
		"A módosítások sorrendje: Először a -t (ha van), majd a -tt  (ha van) opciókat\n"
		"alkalmazza a program, ezt követik a többiek.\n\n"
		"Példa (Windows): " << sName << " -t20150308 +o2\"C:\\hibás dátumok\\pista*.jpg\"\n"
		"      (Linux) : " << sName << " -t20150308 +o2 /hibás\\ dátumok/pista*.jpg\n\n";

	return 1;
}
/*======================================================================*/
static int UsageEng(STRING sName)
/*----------------------------------------------------------------------*/
{
	COUT << sName << 
//				   1         2		   3		 4		   5		 6		   7
//       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
		"EXIF date and time modifier V2.0 for image and sidecar (.xmp) files\n"
		"Usage: " << sName << " [options] <file name> [<file name> [...] ]\n"
		" where options are:\n"
		" -h: print this help. The system language determines the help language.\n"
		"       Only English and Hungarian languages are supported however\n"
		" -h<h|e>: print help in Hungarian or in English\n"
		" -ha[e|h]: print algorithm help in system language, English or Hungarian\n"
		" -e: DELETE original files. If not given original files are kept with a tilde\n"
		"     (~) appended to the original file name\n"
		"     **** WARNING *** Use this option at your own risk! We give no guarantee\n"
		"     that the modified files will be good image files. You have been warned!\n"
		" -r: use regular expressions to match file name. No problem if you do not know\n"
		"     what those are. You can still use wildcards (e.g. * ?) what your OS\n"
		"     understands."
		" -l: only list modified dates but do not change the files\n\n"

		"Modification options (date and time offsets):\n"
		" -t<date>[<time>]: set EXIF date (and optionally time).\n"
		"    Format: YYYMMDD[HHMM[SS]]\n"
		" -tt<time>       : set EXIF time and leave date intact.\n"
		"    Format: HHMM[SS]\n\n"
		" '+/-' before option letter below: add or subtract given time:\n"
		" +/-d<days to add or subtract\n"
//		" +/-m<months to add or subtract>\n"
//		" +/-y<years to add or subtract>\n"
		" +/-o<hours to add or subtract>\n"
		" +/-n<minutes to add or subtract>\n"
		" +/-s<seconds to add or subtract>\n\n"
//				   1         2		   3		 4		   5		 6		   7
//       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
		"More than one modification option may be used at the same time.\n"
		"When -y or -Y is used and the original or modified EXIF year is a leap year\n"
		"   then the month and day will also change!\n"
		"If an option occurs more than once the last one will be in effect.\n"
		" Modification order: Option -t<> is applied first followed by the others.\n\n"
		"<file name> : file path where the file name may contain a regular expression.\n"
		"Examples: " << sName << " -b -t20150308 this\\path\\*.NEF\n"
		"          " << sName << " -o1 this\\path\\*.jpg\n"
		;
//				   1         2		   3		 4		   5		 6		   7
//       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
	return(1);
}

/*======================================================================*/
static int Usage(STRING sName, const TCHAR *pch=nullptr)
/*----------------------------------------------------------------------*/
{
	locale loc = COUT.getloc();
	int lang = loc.name().find_first_of("_hu") == string::npos; // english
	if (pch)
	{
		if (pch[2] == 'a') // algorithm help?
		{
			if (pch[3] == 'e')
				return AlgoEng(sName);
			else if (pch[3] == 'h')
				return AlgoHu(sName);
			else
				return lang ? AlgoEng(sName) : AlgoHu(sName);
		}
		else
		{
			if (pch[2] == 'e')
				return UsageEng(sName);
			else if (pch[2] == 'h')
				return UsageHu(sName);
			else
				return lang ? UsageEng(sName) : UsageHu(sName);
		}
	}
	return  lang ? UsageEng(sName) : UsageHu(sName);
}

#ifdef UNICODE
// convert wchar_t* strictly ASCII date and time strings to char* with size limit 20 
// Returns pointer to a statuc buffer.
static char* DateStringToCharString(CHAR* buf)
{
	static char res[20]; // max length for date-time string
	int size = 20;

	char* pb = res;
	CHAR* pbs = buf;
	while (size--)
		*pb++ = (char)*pbs++;
	*pb = 0;
	return res;
}
#else
#define DateStringToCharString(buf, size) (buf)
#endif


/*======================================================================*/
int main(int argc, TCHAR **argv)
/*----------------------------------------------------------------------*/
{
	STRING sName = GetProgramName(*argv);
	if (argc == 1 || (argc == 2 && *argv[1] == '-' && argv[1][1] == 'h'))
		return Usage(sName, argv[1]);

	SA_FILES saFiles;
	int sign = 1;

	while (--argc)
	{
		++argv;
		try
		{
			if ((sign = argv[0][0] == '-' ? -1 : (argv[0][0] == '+' ? 1 : 0)))
			{
				switch (argv[0][1])
				{
					case 'd': deltas.SetDayDelta(sign, argv[0] + 2); break;	// +/-days

					case 'e': options.keepOld = false;			  break;

					case 'h': return Usage(GetProgramName(*argv));
					case 'l': options.listDateAndTime = true; break;
					// nothing for months! case 'm': deltas._m = sign * stoi(argv[0] + 2); break;	// +months
					case 'n': deltas.SetMinuteDelta(sign, argv[0] + 2); break;	// +/-minutes
					case 'o': deltas.SetHourDelta(sign, argv[0] + 2); break;	// +/-hours
					case 's': deltas.SetSecondDelta(sign, argv[0] + 2); break;	// +/-seconds
					// nothing for years! case 'y': deltas._y = sign * stoi(argv[0] + 2); break;	// +years
					case 't': if (sign == +1) 
								return Usage(sName);
							  if (argv[0][2] == 't')
								  deltas.SetTimeDelta(DateStringToCharString(argv[0] + 3));	// throws if error
							  else
								  deltas.SetDateDelta(DateStringToCharString(argv[0] + 2));	// throws if error
						break;
				default: 
					return Usage(sName);
				}
			}
			else			// get all file names and process them each
			{
				saFiles.Add(STRING(argv[0]));		// get all files which matches the path to list
			}
		}
		catch (...)
		{
			COUT << L("invalid modification parameters!\nExiting.\n");
			return -1;
		}
	}

	STRING name;

	for (int i = 0; i < saFiles.Count(); ++i)
	{
		name = saFiles.Name(i);
		DateModifier modDate(name);
		int err;
		if ((err = modDate.ReadImageFile()) != 0)	// then error
			return err;
		 	// if listDateAndTime true do not write new file
		if(modDate.Modify(!options.listDateAndTime) > 0)  // otherwise an error occured and signalled
			COUT << modDate;		// display EXIF date
	}
	return 0;
}
