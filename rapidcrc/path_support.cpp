/***************************************************************************
 Copyright 2004 Sebastian Ewert

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/
//	2/5/06 gmilow - Modified 

#include "stdafx.h"
#include "globals.h"

static DWORD GetTypeOfPath(CONST TCHAR szPath[MAX_PATH]);

/*****************************************************************************
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH])
	szName	: (IN) string

Return Value:
	returns TRUE if szName is a directory; FALSE otherwise
*****************************************************************************/
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH])
{
	DWORD dwResult;

	dwResult = GetFileAttributes(szName);

	if(dwResult == 0xFFFFFFFF)
		return FALSE;
	else if(dwResult & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;
	else
		return FALSE;
}

/*****************************************************************************
DWORD GetFileSizeQW(CONST TCHAR * szFilename, QWORD * qwSize)
szFilename	: (IN) filename of the file whose filesize we want
qwSize		: (OUT) filesize as QWORD

Return Value:
returns NOERROR if everything went fine. Otherwise GetLastError() is returned

Notes:
- gets size as QWORD from a string
- is also used to validate the string as a legal, openable file (f.e. if
szFilename is some blabla GetLastError() will return 2 => file not found)
*****************************************************************************/
DWORD GetFileSizeQW(CONST TCHAR * szFilename, QWORD * qwSize)
{
	DWORD dwErrorCode;
	HANDLE hFile;
	DWORD dwLo = 0, dwHi = 0;

	hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0 , 0);
	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();

	dwLo = GetFileSize(hFile, &dwHi);
	dwErrorCode = GetLastError();

	CloseHandle(hFile);

	if(dwLo == INVALID_FILE_SIZE){
		if(dwErrorCode != NOERROR){
			(*qwSize) = 0;
			return dwErrorCode;
		}
	}

	(*qwSize) = MAKEQWORD(dwHi, dwLo);

	return NO_ERROR;
}

/*****************************************************************************
INT ReduceToPath(TCHAR szString[MAX_PATH])
	szString	: (IN/OUT) assumed to be a string containing a filepath

Return Value:
	returns the new length of the string

Notes:
	- looks for a '\' in the string from back to front; if one is found it is set
	  to \0. Otherwise the whole string is set to TEXT("")
	- used to set g_szBasePath
*****************************************************************************/
INT ReduceToPath(TCHAR szString[MAX_PATH])
{
	size_t	stStringLength;
	INT		iStringLength;

	StringCchLength(szString, MAX_PATH, & stStringLength);
	iStringLength = (INT) stStringLength;
	while( (iStringLength > 0) && (szString[iStringLength] != TEXT('\\')) )
		iStringLength--;
	if(iStringLength == 2) // this is the case for example for C:\ or C:\test.txt. Then we want soemthing like C:\ and not C:
		iStringLength++;
	szString[iStringLength] = TEXT('\0'); // replace the '\' with 0 to get the path of the sfv file
	
	return iStringLength;
}

/*****************************************************************************
TCHAR * GetFilenameWithoutPathPointer(TCHAR szFilenameLong[MAX_PATH])
	szFilenameLong	: (IN) a filename including path

Return Value:
	returns a pointer to the filepart of the string

Notes:
	- filepart if everything after the last '\' in the string. If no '\' is found,
	  we return the original pointer
*****************************************************************************/
CONST TCHAR * GetFilenameWithoutPathPointer(CONST TCHAR szFilenameLong[MAX_PATH])
{
	size_t size_temp;
	INT iIndex;

	StringCchLength(szFilenameLong, MAX_PATH, &size_temp);

	for(iIndex=((INT)size_temp - 1); iIndex >= 0; --iIndex){
		if(szFilenameLong[iIndex] == TEXT('\\'))
			break;
	}
	// if Backslash is found we want the character right beside it as the first character
	// in the short string.
	// if no backslash is found the last index is -1. In this case we also have to add +1 to get
	// the original string as the short string
	++iIndex;

	return szFilenameLong + iIndex;
}

/*****************************************************************************
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH], CONST TCHAR * szExtension)
	szName		: (IN) filename string
	szExtension	: (IN) extension we want to look for

Return Value:
	returns TRUE if szFilename has the extension

Notes:
- szExtension is assumed to be of the form .sfv or .md5, i.e. a dot and 3 chars
*****************************************************************************/
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH], CONST TCHAR szExtension[4])
{
	size_t stString;

	StringCchLength(szFilename, MAX_PATH, & stString);

	// we compare the 4 last characters
	if( (stString > 4) && (lstrcmpi(szFilename + stString - 4, szExtension) == 0) )
		return TRUE;
	else
		return FALSE;
}

/*****************************************************************************
BOOL GetCrcFromFilename(TCHAR szFilename[MAX_PATH], DWORD * pdwFoundCrc)
	szFilename		: (IN) string; assumed to be the szFilenameShort member of the
						   FILEINFO struct
	pdwFoundCrc		: (OUT) return value is TRUE this parameter is set to the found
							CRC32 as a DWORD

Return Value:
	returns TRUE if a CRC was found in the filename. Otherwise FALSE

Notes:
	- walks through a filename from the rear to the front.
	- if a ']' or ')' is found we check if 9 chars before is an '[' or '(' and if
	  there are 8 legal Hex characters in between (via IsLegalHexSymbol)
*****************************************************************************/
BOOL GetCrcFromFilename(CONST TCHAR szFilename[MAX_PATH], DWORD * pdwFoundCrc)
{
	size_t StringSize;
	INT iIndex;
	BOOL bFound;
	TCHAR szCrc[9];
	TCHAR szFileWithoutPath[MAX_PATH];

	StringCchCopy(szFileWithoutPath, MAX_PATH, GetFilenameWithoutPathPointer(szFilename));

	if(FAILED(StringCchLength(szFileWithoutPath, MAX_PATH, &StringSize)))
		return FALSE;

	if(StringSize == 0)
		return FALSE;
	
	iIndex = (int)StringSize;
	bFound = FALSE;
	do{
		--iIndex;
		if((szFileWithoutPath[iIndex] == TEXT(']')) || (szFileWithoutPath[iIndex] == TEXT(')')) )
		{
			if ((iIndex - 9) < 0)
				break;
			else{
				bFound = TRUE;
				if (! ((szFileWithoutPath[iIndex-9] == TEXT('[')) || (szFileWithoutPath[iIndex-9] == TEXT('(')) ) )
					bFound = FALSE;
				for(int i=1; i <= 8; ++i)
					if(! IsLegalHexSymbol(szFileWithoutPath[iIndex-i]))
						bFound = FALSE;
				if(bFound)
					iIndex -= 8;
			}
		}
	}
	while((iIndex > 0) && (!bFound));

	if(!bFound)
		return FALSE;
	
	StringCchCopyN(szCrc, 9, szFileWithoutPath + iIndex, 8);
	(*pdwFoundCrc) = HexToDword(szCrc, 8);

	return TRUE;
}

/*****************************************************************************
VOID ProcessFileProperties(QWORD * pqwFilesizeSum)
	pqwFilesizeSum	: (OUT) sum of filesizes is stored here

Return Value:
returns nothing

Notes:
- sets file information like filesize, CrcFromFilename, Long Pathname, szFilenameShort
*****************************************************************************/
VOID ProcessFileProperties(TCHAR *g_szBasePath, PROGRAM_STATUS *g_program_status, FILEINFO *g_fileinfo_list_first_item, QWORD * pqwFilesizeSum)
{
	size_t stString;
	FILEINFO * pFileinfo;

	StringCchLength(g_szBasePath, MAX_PATH, & stString);
	// g_szBasePath can have a trailing '\' or not. For example 'D:\' or 'D:\Bla'. If the trailing '\'
	// is missing we increase stString by 1 because we don't want a \ as the first symbol in szFilename
	if(stString > 0)
		if( g_szBasePath[stString - 1] != TEXT('\\') )
			++stString;

	(*pqwFilesizeSum) = 0;

	pFileinfo = g_fileinfo_list_first_item;
	while(pFileinfo != NULL){
		GetLongPathName(pFileinfo->szFilename, pFileinfo->szFilename, MAX_PATH);
		pFileinfo->szFilenameShort = pFileinfo->szFilename + stString;
		if(!IsApplDefError(pFileinfo->dwError)){
			pFileinfo->dwError = GetFileSizeQW(pFileinfo->szFilename, &(pFileinfo->qwFilesize));
			if (pFileinfo->dwError == NO_ERROR){
				(*pqwFilesizeSum) += pFileinfo->qwFilesize;
				if(g_program_status->uiRapidCrcMode == MODE_NORMAL)
					if(GetCrcFromFilename(pFileinfo->szFilenameShort, & pFileinfo->dwCrc32Found))
						pFileinfo->bCrcFound = TRUE;
					else
						pFileinfo->bCrcFound = FALSE;
			}
		}

		pFileinfo = pFileinfo->nextListItem;
	}

	return;
}
