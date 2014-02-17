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

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <windows.h>
#include <strsafe.h>

//****** some defines *******
// conditional compilation
//#define USE_MD5_REF			// use reference implementation from rfc
#define USE_MD5_OSSL			// use OpenSSL MD5 assembly implementation
//#define USE_TIME_MEASUREMENT

// some sizes for variables
#define MAX_BUFFER_SIZE_CALC	0x10000
#define MAX_BUFFER_SIZE_OFN 0xFFFFF // Win9x has a problem with values where just the first bit is set like 0x20000 for OFN buffer:
#define MAX_LINE_LENGTH 1000
#define MAX_RESULT_LINE 200
#define CRC_AS_STRING_LENGHT 9
#define MD5_AS_STRING_LENGHT 33
#define INFOTEXT_STRING_LENGHT 30

// special error codes ("Bit 29 is reserved for application-defined error codes; no system
// error code has this bit set. If you are defining an error code for your application, set
// this bit to one. That indicates that the error code has been defined by an application,
// and ensures that your error code does not conflict with any error codes defined by the system.")
#define APPL_ERROR 0x20000000
#define APPL_ERROR_ILLEGAL_CRC (APPL_ERROR + 1)

// RapidCRC modes; also used in the action functions
#define MODE_NORMAL				0
#define MODE_SFV				1
#define MODE_MD5				2

//****** custom datatypes *******

typedef unsigned __int64 QWORD, *LPQWORD;

//****** some macros *******
#define MAKEQWORD(a, b)	\
	((QWORD)( ((QWORD) ((DWORD) (a))) << 32 | ((DWORD) (b))))

//****** structures ******* 
// sort descending with sortorder typesize (TCHAR[5] < DWORD !)

typedef struct _FILEINFO{
	QWORD	qwFilesize;
	FLOAT	fSeconds;
	DWORD	dwCrc32Result;
	DWORD	dwCrc32Found;
	DWORD	dwError;
	BOOL	bCrcFound;
	BOOL	bMd5Found;
	_FILEINFO * nextListItem;
	TCHAR	szFilename[MAX_PATH];
	TCHAR *	szFilenameShort;
	TCHAR	szCrcResult[CRC_AS_STRING_LENGHT];
	TCHAR	szMd5Result[MD5_AS_STRING_LENGHT];
	TCHAR	szInfo[INFOTEXT_STRING_LENGHT];
	BYTE	abMd5Result[16];
	BYTE	abMd5Found[16];
}FILEINFO;

typedef struct{
	QWORD *				pqwFilesizeSum;
	HWND *				arrHwnd;		// array
}THREAD_PARAMS_FILEINFO;

typedef struct{
	QWORD				qwBytesReadCurFile;				// out
	QWORD				qwBytesReadAllFiles;			// out
	BOOL				bCalculateCrc;					// in
	BOOL				bCalculateMd5;					// in
	HWND				* arrHwnd;						// in
	FILEINFO			* pFileinfo_cur;				// out
}THREAD_PARAMS_CALC;

typedef struct{
	UINT			uiRapidCrcMode;
	BOOL			bCrcCalculated;
	BOOL			bMd5Calculated;
}PROGRAM_STATUS;

BOOL ShowResult(string sParfileName, FILEINFO * pFileinfo, PROGRAM_STATUS *g_program_status, bool bAlreadyVerified);

//helper functions (helpfcts.cpp)
BOOL IsLegalHexSymbol(CONST TCHAR tcChar);
DWORD HexToDword(CONST TCHAR * szHex, UINT uiStringSize);
VOID GetNextLine(CONST HANDLE hFile, CHAR * szLineAnsi, CONST UINT uiLengthLine, UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile);
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH]);
FILEINFO * AllocateFileinfo();
VOID AllocateMultipleFileinfo(CONST UINT uiCount, FILEINFO *g_fileinfo_list_first_item);
VOID DeallocateFileinfoMemory(FILEINFO *g_fileinfo_list_first_item);
BOOL IsApplDefError(CONST DWORD dwError);
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH], CONST TCHAR szSearchString[MAX_PATH]);
BOOL IsWin2000orHigher();
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut);
#ifdef USE_TIME_MEASUREMENT
	VOID StartTimeMeasure(CONST BOOL bStart);
#endif
	
//path support functions (path_support.cpp)
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH]);
DWORD GetFileSizeQW(CONST TCHAR * szFilename, QWORD * qwSize);
INT ReduceToPath(TCHAR szString[MAX_PATH]);
CONST TCHAR * GetFilenameWithoutPathPointer(CONST TCHAR szFilenameLong[MAX_PATH]);
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH], CONST TCHAR * szExtension);
BOOL GetCrcFromFilename(CONST TCHAR szFilename[MAX_PATH], DWORD * pdwFoundCrc);
VOID ProcessFileProperties(TCHAR *g_szBasePath, PROGRAM_STATUS *g_program_status, FILEINFO *g_fileinfo_list_first_item, QWORD * pqwFilesizeSum);

//SFV and MD5 functions (sfvfcts.cpp)
BOOL EnterSfvMode(TCHAR *g_szBasePath, string sParfileName, string sSfvFile, FILEINFO **g_fileinfo_list_first_item);
BOOL EnterMd5Mode(TCHAR *g_szBasePath, string sParfileName, string sMd5File, FILEINFO **g_fileinfo_list_first_item);

//Thread procedures (threadprocs.cpp)
bool ThreadProc_Calc(bool *bSkipVerify, string sParfileName, vector<string> *m_vVerified, THREAD_PARAMS_CALC* pthread_params_calc, PROGRAM_STATUS *g_program_status, FILEINFO *g_fileinfo_list_first_item);

#if defined(USE_MD5_REF)
#  include "md5_ref.h"
#elif defined(USE_MD5_OSSL)
#  include "md5_ossl.h"
#else
#  error USE_MD5_REF or USE_MD5_OSSL have to be defined
#endif

#endif
