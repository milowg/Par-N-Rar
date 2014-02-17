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
#include <shlobj.h>

/*****************************************************************************
BOOL IsLegalHexSymbol(CONST TCHAR tcChar)
	tcChar	: (IN) char representing a possible HEX symbol

Return Value:
	returns TRUE if tcChar is a legal hex symbol, FALSE otherwise
*****************************************************************************/
BOOL IsLegalHexSymbol(CONST TCHAR tcChar)
{
	if((tcChar >= TEXT('0')) && (tcChar <= TEXT('9')))
		return TRUE;
	if((tcChar >= TEXT('A')) && (tcChar <= TEXT('F')))
		return TRUE;
	if((tcChar >= TEXT('a')) && (tcChar <= TEXT('f')))
		return TRUE;
	return FALSE;
}

/*****************************************************************************
DWORD HexToDword(CONST TCHAR * szHex, UINT uiStringSize)
	szHex			: (IN) string representing hex value. Leftmost 2 chars represent the
						most sigificant byte of the CRC
	uiStringSize	: (IN) length of szHex in chars exluding NULL. Assumed to be not
						larger than 8

Return Value:
	returns the CRC converted to DWORD

Notes:
	- if uiStringSize < 8 then uiStringSize characters are processed; otherwise
	  just the first 8 characters are processed (because return value is DWORD)
*****************************************************************************/
DWORD HexToDword(CONST TCHAR * szHex, UINT uiStringSize)
{
	DWORD dwAdd, dwResult;

	if(uiStringSize > 8)
		uiStringSize = 8;

	dwResult = 0;
	for(UINT i = 0; i < uiStringSize; ++i){
		if((szHex[i] >= TEXT('0')) && (szHex[i] <= TEXT('9')))
			dwAdd = (szHex[i] - TEXT('0'));
		else if((szHex[i] >= TEXT('A')) && (szHex[i] <= TEXT('F')))
			dwAdd = (szHex[i] - TEXT('A') + 10);
		else if((szHex[i] >= TEXT('a')) && (szHex[i] <= TEXT('f')))
			dwAdd = (szHex[i] - TEXT('a') + 10);
		else
			dwAdd = 0;
		dwResult = dwResult * 0x10 + dwAdd;
	}

	return dwResult;
}

/*****************************************************************************
VOID GetNextLine(CONST HANDLE hFile, CHAR * szLineAnsi, CONST UINT uiLengthLine,
				UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile)
	hFile			: (IN) handle to an already opened file
	szLineAnsi		: (OUT) string to which the line is written
	uiLengthLine	: (IN) length of szLineAnsi; to be sure that we do not write
						past szLineAnsi
	puiStringLength	: (OUT) length of string in szLineAnsi after reading the line
	pbErrorOccured	: (OUT) signales if an error occurred
	pbEndOfFile		: (OUT) signales if the end of the file has been reached

Return Value:
	returns nothing

Notes:
	- reads a line from an already opened file
*****************************************************************************/
VOID GetNextLine(CONST HANDLE hFile, CHAR * szLineAnsi, CONST UINT uiLengthLine,
				 UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile)
{
	CHAR myChar;
	UINT uiCount;
	DWORD dwBytesRead;
	BOOL bSuccess;

	// we need at least one byte for the NULL Terminator
	if(uiLengthLine <= 1){
		(*pbErrorOccured) = TRUE;
		return;
	}

	ZeroMemory(szLineAnsi, uiLengthLine);

	uiCount = 0;
	while(TRUE)
	{
		// in the next round we write into szLineAnsi[uiCount] and
		// szLineAnsi[uiCount+1] is used to terminate the string
		if(uiCount >= uiLengthLine-1){ 
			(*pbErrorOccured) = TRUE;
			return;
		}

		bSuccess = ReadFile(hFile, & myChar, sizeof(myChar), &dwBytesRead, NULL);
		// if filepointer is beyond file limits, bSuccess is still nonzero
		// i.e. bSuccess is only FALSE on real errors
		if(!bSuccess){
			(*pbErrorOccured) = TRUE;
			return;
		}

		if(dwBytesRead == 0){
			(*pbErrorOccured) = FALSE;
			(*puiStringLength) = uiCount;
			(*pbEndOfFile) = TRUE;
			return;
		}

		if( (myChar == '\n') || (myChar == '\r') ){
			do{
				bSuccess = ReadFile(hFile, & myChar, sizeof(myChar), &dwBytesRead, NULL);
				if(!bSuccess){
					(*pbErrorOccured) = TRUE;
					return;
				}
			}while((dwBytesRead > 0) && ((myChar == '\n') || (myChar == '\r')) );

			if( (dwBytesRead > 0) && !((myChar == '\n') || (myChar == '\r')) )
				SetFilePointer(hFile, -1, NULL, FILE_CURRENT);

            (*pbErrorOccured) = FALSE;
			(*puiStringLength) = uiCount;
			(*pbEndOfFile) = FALSE;
			return;
		}
		else{
			szLineAnsi[uiCount] = myChar;
			uiCount += 1;
		}
	}

	return;
}

/*****************************************************************************
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH])
	szFilenamePattern	: (IN) string that holds the filename pattern to be checked

Return Value:
returns TRUE if there's no illegal char in szFilename otherwise FALSE. szFilename
mustn't be a pathname. It has to be JUST the filename part

Notes:
-    \ / : * ? " < > |   are illegal
*****************************************************************************/
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH])
{
	size_t stLength;
	UINT i;
	BOOL bIsLegal;

	StringCchLength(szFilename, MAX_PATH, &stLength);

	bIsLegal = TRUE;
	for(i = 0; (i < stLength) && bIsLegal; i++){
		bIsLegal =	(szFilename[i] != TEXT('\\')) &&
					(szFilename[i] != TEXT('/')) &&
					(szFilename[i] != TEXT(':')) &&
					(szFilename[i] != TEXT('*')) &&
					(szFilename[i] != TEXT('?')) &&
					(szFilename[i] != TEXT('\"')) &&
					(szFilename[i] != TEXT('<')) &&
					(szFilename[i] != TEXT('>')) &&
					(szFilename[i] != TEXT('|'));

	}

	return bIsLegal;
}

/*****************************************************************************
FILEINFO * AllocateFileinfo()

Return Value:
returns a pointer to the new FILEINFO item

Notes:
- allocates memory and does a basic error handling
*****************************************************************************/
FILEINFO * AllocateFileinfo()
{
	FILEINFO * pFileinfo;

	pFileinfo = (FILEINFO *) malloc(sizeof(FILEINFO));

	if(pFileinfo == NULL){
		MessageBox(NULL,
			TEXT("Could not allocate memory. Reason might be, that the system is out of memory.\nThe program will exit now"),
			TEXT("Heavy error"), MB_ICONERROR | MB_TASKMODAL | MB_OK);
		ExitProcess(1);
		return NULL;
	}
	else{
		ZeroMemory(pFileinfo, sizeof(FILEINFO));
	}

	return pFileinfo;
}

/*****************************************************************************
VOID AllocateMultipleFileinfo(CONST UINT uiCount)
	uiCount	: (IN) number of empty items to create

Return Value:
nothing

Notes:
- creates a list of uiCount linked FILEINFO items
*****************************************************************************/
VOID AllocateMultipleFileinfo(CONST UINT uiCount, FILEINFO *g_fileinfo_list_first_item)
{
	FILEINFO * Fileinfo_list_last;

	g_fileinfo_list_first_item = AllocateFileinfo();
	Fileinfo_list_last = g_fileinfo_list_first_item;
	for(UINT i = 1; i < uiCount; ++i) //1 to... because 1 is created before the loop
	{
		Fileinfo_list_last->nextListItem = AllocateFileinfo();
		Fileinfo_list_last = Fileinfo_list_last->nextListItem;
	}
	Fileinfo_list_last->nextListItem = NULL; // set end of the list

	return;
}

/*****************************************************************************
VOID DeallocateFileinfoMemory(CONST HWND hListView);
	hListView	: (IN) handle to the listview

Return Value:
- returns nothing

Notes:
- dealloctates the FILEINFO list
*****************************************************************************/
VOID DeallocateFileinfoMemory(FILEINFO *g_fileinfo_list_first_item)
{
	FILEINFO * Fileinfo;

	while(g_fileinfo_list_first_item != NULL){
		Fileinfo = g_fileinfo_list_first_item->nextListItem;
		free(g_fileinfo_list_first_item);
		g_fileinfo_list_first_item = Fileinfo;
	}

	return;
}

/*****************************************************************************
BOOL IsApplDefError(CONST DWORD dwError)
	dwError : (IN) error to be processed

Return Value:
	- return TRUE if dwError is one of the user defined errors in global.h
*****************************************************************************/
BOOL IsApplDefError(CONST DWORD dwError)
{
	// return (dwError == APPL_ERROR_ILLEGAL_CRC);
	return (dwError & APPL_ERROR) > 0 ? TRUE : FALSE;
}

/*****************************************************************************
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH], CONST TCHAR szSearchString[MAX_PATH])
	szSearchPattern	: (IN) 
	szSearchString	: (IN) 

Return Value:
returns TRUE if szSearchPattern is a prefix of szSearchString
*****************************************************************************/
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH], CONST TCHAR szSearchString[MAX_PATH])
{
	size_t stStringLength;

	StringCchLength(szSearchPattern, MAX_PATH, & stStringLength);

	for(UINT i = 0; i < stStringLength; i++)
		if(szSearchPattern[i] != szSearchString[i])
			return FALSE;
	return TRUE;
}

/*****************************************************************************
BOOL IsWin2000orHigher()

Return Value:
returns TRUE if running system is Win2k or later
*****************************************************************************/
BOOL IsWin2000orHigher() 
{
	OSVERSIONINFO ovi;

	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(& ovi);

	return (ovi.dwMajorVersion >= 5);
}

/*****************************************************************************
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut)
	szString		: (OUT) size of the new array
	stBufferLength	: (IN) length of szString in TCHAR
	tcIn			: (IN) what we look for
	tcOut			: (IN) what we use as the replacement

Return Value:
returns nothing
*****************************************************************************/
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut)
{
	for(UINT i = 0; i < stBufferLength && szString[i] != 0; i++)
		if(szString[i] == tcIn)
			szString[i] = tcOut;

	return;
}

/*****************************************************************************
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut)
	szString		: (OUT) size of the new array
	stBufferLength	: (IN) length of szString in TCHAR
	tcIn			: (IN) what we look for
	tcOut			: (IN) what we use as the replacement

Return Value:
returns nothing

Notes:
- the macro USE_TIME_MEASUREMENT controls if this function is allowed to use or not
*****************************************************************************/
#ifdef USE_TIME_MEASUREMENT
VOID StartTimeMeasure(CONST BOOL bStart)
{
	static QWORD qwStart, qwStop, wqFreq;
	
	if(bStart){
		OutputDebugString(TEXT("===>>>>>>>>> Anfang Zeitmessung\r\n"));
		QueryPerformanceFrequency((LARGE_INTEGER*)&wqFreq);
		QueryPerformanceCounter((LARGE_INTEGER*) &qwStart);
	}
	else{
		QueryPerformanceCounter((LARGE_INTEGER*) &qwStop);
		TCHAR szBla[200];
		FLOAT fTime = (FLOAT)((qwStop - qwStart) / (FLOAT)wqFreq);
		UINT uiTics = (UINT)(qwStop - qwStart);
		StringCchPrintf(szBla, 200, TEXT("===>>>>>>>>> Ende Zeitmessung: %u (%f sek)\r\n"), uiTics, fTime );
		//StringCchPrintf(szBla, 200, TEXT("===>>>>>>>>> Ende Zeitmessung: %f\r\n"), fTime );
		OutputDebugString(szBla);
	}

	return;
}
#endif

