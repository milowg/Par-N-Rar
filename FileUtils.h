//  This file is part of Par-N-Rar
//  http://www.milow.net/site/projects/parnrar.html
//
//  Copyright (c) Gil Milow
//
//  Par-N-Rar is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  Par-N-Rar is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
//  This code may not be used to develop a RAR (WinRAR) compatible archiver.
//

#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__

#include <iostream>
#include <string>
#include "greta\regexpr2.h"
using namespace std;
using namespace regex;

class FileUtils
{
public:
	static bool DirIsEmptyR(string sDir)
	{
		ostringstream strm;
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile((sDir + "\\*").c_str(), &FindFileData);		

		while (hFind != INVALID_HANDLE_VALUE) 
		{	
			if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, ".."))
			{
				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					strm.str(""); strm << "Found File: " << FindFileData.cFileName;
					theApp.Trace("%s", strm.str().c_str());

					FindClose(hFind);
					return false;
				}
				else
				{
					strm.str(""); strm << "Found Directory: " << FindFileData.cFileName;
					theApp.Trace("%s", strm.str().c_str());

					if (!DirIsEmptyR(sDir + "\\" + FindFileData.cFileName))
					{
						FindClose(hFind);
						return false;
					}
				}
			}
			if (!FindNextFile(hFind, &FindFileData))
				break;
		}
		FindClose(hFind);
		return true;
	}

	static bool SendToRecycleBin(const char *szToRecycle)
	{
		SHFILEOPSTRUCT fos;
		auto_ptr<char> apTemp( new char[strlen(szToRecycle) + 2] );
		memset(&fos, 0, sizeof(fos));
		fos.wFunc = FO_DELETE;
		fos.pFrom = apTemp.get();
		memset((void *)fos.pFrom, 0, strlen(szToRecycle) + 2);
		strcpy((char *)fos.pFrom, szToRecycle);
		fos.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT;
		return (SHFileOperation(&fos) == 0);
	}

	static CString ErrorString(DWORD err)
	{
		CString Error;
		LPTSTR s;
		if(::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (LPTSTR)&s, 0, NULL) == 0)
		{ 
			// Unknown error code %08x (%d)
			CString t;
			t.Format("Unknown error 0x%08x (%d)", err, LOWORD(err));
			Error = t;
		} 
		else
		{ 
			LPTSTR p = _tcschr(s, _T('\r'));
			if(p != NULL)
			{ /* lose CRLF */
				*p = _T('\0');
			} /* lose CRLF */
			Error = s;
			::LocalFree(s);
		}
		return Error;
	}

	//Figures out what kind of file set we have based on a found file name
	static string DetermineSetType(string sFile)
	{
		match_results results;
		match_results::backref_type br;
		string sFileName;
		ostringstream strm;

		string sExt = StringUtils::FileExtension(sFile);
		CString cstrExt = sExt.c_str();
		cstrExt.MakeUpper();
		if (cstrExt == "RAR")
			return "RAR";

		//Check for pattern "file.r01"
		strm.str("");
		rpattern pattern1( ".*\\.r[0-9]+$", NOCASE); 
		br = pattern1.match( sFile, results );
		if( br.matched ) 
			return "RAR";

		//Check for pattern "file.001"
		strm.str("");
		rpattern pattern2( ".*\\.[0-9][0-9][0-9]+$", NOCASE); 
		br = pattern2.match( sFile, results );
		if( br.matched ) 
			return "RAR";

		//Check for pattern "file.sfv" or "file.md5"
		if (cstrExt == "SFV" || cstrExt == "MD5")
			return "SFV";
		else
			return "PAR";

	}

	static string GetRarFirstFilename(string sFile)
	{
		match_results results;
		match_results::backref_type br;
		string sFileName;
		ostringstream strm;	
		
		//Check for pattern "file.part01.rar"
		strm.str("");
		rpattern pattern1( "(.*?)\\.?part([0-9]+)\\.rar$", NOCASE); 
		br = pattern1.match( sFile, results );
		if( br.matched ) 
		{
			string sPartNum;			
			sFileName = "";
			if (results.cbackrefs() == 3)
			{
				sFileName = results.backref(1).str();
				sPartNum = results.backref(2).str();
			}
			else
			{
				sPartNum = results.backref(1).str();
			}
			
			string sNewPartNum = "";
			for (int i=0; i<sPartNum.length()-1; i++)
				sNewPartNum += "0";
			sNewPartNum += "1";

			strm << sFileName << ".part" << sNewPartNum << ".rar";
			return strm.str();
		}

		//At this point, if the file is "file.rar" then it is the first file in the set
		string sExt = StringUtils::FileExtension(sFile);
		CString cstrExt = sExt.c_str();
		cstrExt.MakeUpper();
		if (cstrExt == "RAR")
		{
			return sFile;
		}

		//Check for pattern "file.r01"
		strm.str("");
		rpattern pattern2( "(.*?)\\.r[0-9]+$", NOCASE); 
		br = pattern2.match( sFile, results );
		if( br.matched ) 
		{
			sFileName = "";
			if (results.cbackrefs() == 2)
			{
				sFileName = results.backref(1).str();
			}
			strm << sFileName << ".rar";
			return strm.str();
		}

		//Check for pattern "file.001"
		strm.str("");
		rpattern pattern3( "(.*?)\\.[0-9][0-9][0-9]+$", NOCASE); 
		br = pattern3.match( sFile, results );
		if( br.matched ) 
		{
			sFileName = "";
			if (results.cbackrefs() == 2)
			{
				sFileName = results.backref(1).str();
			}
			strm << sFileName << ".001";
			return strm.str();
		}

		//If we got here, we don't know what this is, so return the original file
		return sFile;
	}

	static string GetRarNextFilename(string sFile)
	{
		match_results results;
		match_results::backref_type br;
		string sFileName;
		string sPartNum;
		char szNewPartNum[1024];
		int iPartNum;
		ostringstream strm;
		
		//Check for pattern "file.part01.rar"
		strm.str("");
		rpattern pattern1( "(.*?)\\.?part([0-9]+)\\.rar", NOCASE); 
		br = pattern1.match( sFile, results );
		if( br.matched ) 
		{
	
			sFileName = "";
			if (results.cbackrefs() == 3)
			{
				sFileName = results.backref(1).str();
				sPartNum = results.backref(2).str();
			}
			else
			{
				sPartNum = results.backref(1).str();
			}
			
			iPartNum = atoi(sPartNum.c_str());
			sprintf(szNewPartNum, "%0*d", sPartNum.length(), iPartNum+1);

			strm << sFileName << ".part" << szNewPartNum << ".rar";
			return strm.str();
		}

		//At this point, if the file is "file.rar" then it is the first file in the set
		strm.str("");
		rpattern pattern2( "(.*?)\\.rar", NOCASE); 
		br = pattern2.match( sFile, results );
		if( br.matched ) 
		{			
			sFileName = "";
			if (results.cbackrefs() == 3)
			{
				sFileName = results.backref(1).str();
				sPartNum = results.backref(2).str();
			}
			else
			{
				sPartNum = results.backref(1).str();
			}

			strm << sFileName << ".r01";
			return strm.str();
		}

		//Check for pattern "file.r01"
		strm.str("");
		rpattern pattern3( "(.*?)\\.r([0-9]+)", NOCASE); 
		br = pattern3.match( sFile, results );
		if( br.matched ) 
		{
			sFileName = "";
			if (results.cbackrefs() == 2)
			{
				sFileName = results.backref(1).str();
				sPartNum = results.backref(2).str();
			}
			else
			{
				sPartNum = results.backref(1).str();
			}

			iPartNum = atoi(sPartNum.c_str());
			sprintf(szNewPartNum, "%0*d", sPartNum.length(), iPartNum+1);

			strm << sFileName << ".r" << szNewPartNum;
			return strm.str();
		}

		//Check for pattern "file.001"
		strm.str("");
		rpattern pattern4( "(.*?)\\.([0-9])+", NOCASE); 
		br = pattern4.match( sFile, results );
		if( br.matched ) 
		{
			sFileName = "";
			if (results.cbackrefs() == 2)
			{
				sFileName = results.backref(1).str();
				sPartNum = results.backref(2).str();
			}
			else
			{
				sPartNum = results.backref(1).str();
			}

			iPartNum = atoi(sPartNum.c_str());
			sprintf(szNewPartNum, "%0*d", sPartNum.length(), iPartNum+1);

			strm << sFileName << "." << szNewPartNum;
			return strm.str();
		}

		//If we got here, we don't know what this is, so return the original file
		return sFile;
	}

	static string GetFileExtension(string sFileName)
	{
		long lPos = sFileName.find_last_of('.');
		if (lPos != -1)
			return sFileName.substr(lPos+1).c_str();
		else
			return "";
	}

	static string GetFileNameNoExtension(string sFileName)
	{
		long lPos = sFileName.find_last_of('.');
		if (lPos != -1)
			return sFileName.substr(0, lPos);
		else
			return sFileName;
	}

};


#endif