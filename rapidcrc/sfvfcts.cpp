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
#include "../PnrMessage.h"

BOOL EnterSfvMode(TCHAR *g_szBasePath, string sParfileName, string sSfvFile, FILEINFO **g_fileinfo_list_first_item)
{
	CHAR	szLineAnsi[MAX_LINE_LENGTH];
	TCHAR	szLine[MAX_LINE_LENGTH];
	TCHAR	szFilenameSfv[MAX_PATH];
	HANDLE	hFile;
	UINT	uiStringLength;
	BOOL	bErrorOccured, bEndOfFile;	
	ostringstream strm;

	BOOL	bCrcOK;
	FILEINFO * pFileinfo;
	FILEINFO * pFileinfo_prev;

	// save SFV filename and path
	// => g_szBasePath in SFV-mode is the path part of the complete filename of the .sfv file
	StringCchCopy(szFilenameSfv, MAX_PATH, sSfvFile.c_str());
	StringCchCopy(g_szBasePath, MAX_PATH, szFilenameSfv);
	ReduceToPath(g_szBasePath);
	GetLongPathName(g_szBasePath, g_szBasePath, MAX_PATH);

	// This is(should be) the ONLY place where a persistent change of the current directory is done
	// (for GetFullPathName())
	if(!SetCurrentDirectory(g_szBasePath))
		return FALSE;

	hFile = CreateFile(szFilenameSfv, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN , 0);
	if(hFile == INVALID_HANDLE_VALUE){
		if (sParfileName != "")
		{
			strm.str(""); strm << "SFV file could not be read";
			CPnrMessage::SendParAddDetails(sParfileName, strm.str(), true);			
		}
		return FALSE;
	}

	(*g_fileinfo_list_first_item) = AllocateFileinfo();
	(*g_fileinfo_list_first_item)->nextListItem = NULL;
	(*g_fileinfo_list_first_item)->dwError = NO_ERROR;

	pFileinfo = *g_fileinfo_list_first_item;
	pFileinfo_prev = NULL;
	GetNextLine(hFile, szLineAnsi, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile);
	if(bErrorOccured){
		if (sParfileName != "")
		{
			strm.str(""); strm << "SFV file could not be read";
			CPnrMessage::SendParAddDetails(sParfileName, strm.str(), true);
		}
		return FALSE;
	}

	while( !(bEndOfFile && uiStringLength == 0) ){

		if(uiStringLength > 8){

#ifdef UNICODE
			MultiByteToWideChar(CP_ACP,					// ANSI Codepage
								0,						// we use no flags; ANSI isn't a 'real' MBCC
								szLineAnsi,				// the ANSI String
								-1,						// ANSI String is 0 terminated
								szLine,					// the UNICODE destination string
								MAX_LINE_LENGTH );		// size of the UNICODE String in chars
#else
			StringCchCopy(szLine, MAX_LINE_LENGTH, szLineAnsi);
#endif

			//delete trailing spaces
			while( (szLine[uiStringLength - 1] == TEXT(' ')) && (uiStringLength > 8) ){
				szLine[uiStringLength - 1] = NULL;
				uiStringLength--;
			}

			if( (szLine[0] != TEXT(';')) && (szLine[0] != TEXT('\0')) ){
				bCrcOK = TRUE;
				for(int i=1; i <= 8; ++i)
					if(! IsLegalHexSymbol(szLine[uiStringLength-i]))
						bCrcOK = FALSE;
				if(bCrcOK){
					pFileinfo->bCrcFound = TRUE;
					pFileinfo->dwCrc32Found = HexToDword(szLine + uiStringLength - 8, 8);
					pFileinfo->dwError = NOERROR;
				}
				else
					pFileinfo->dwError = APPL_ERROR_ILLEGAL_CRC;

				uiStringLength -= 8;
				szLine[uiStringLength] = NULL; // keep only the filename
				//delete trailing spaces
				while( (szLine[uiStringLength - 1] == TEXT(' ')) && (uiStringLength > 0) ){
					szLine[uiStringLength - 1] = NULL;
					uiStringLength--;
				}

				GetFullPathName(szLine, MAX_PATH, pFileinfo->szFilename, NULL);

				pFileinfo->nextListItem = AllocateFileinfo();
				pFileinfo_prev = pFileinfo;
				pFileinfo = pFileinfo->nextListItem;
			}
		}
		
		GetNextLine(hFile, szLineAnsi, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile);
		if(bErrorOccured){
			if (sParfileName != "")
			{
				strm.str(""); strm << "SFV file could not be read";
				CPnrMessage::SendParAddDetails(sParfileName, strm.str(), true);
			}
			return FALSE;
		}
	}
	CloseHandle(hFile);

	if(pFileinfo_prev != NULL)
	{
		// we created one Fileinfo too much
		free(pFileinfo_prev->nextListItem);
		pFileinfo_prev->nextListItem = NULL;
	}

	return TRUE;
}

BOOL EnterMd5Mode(TCHAR *g_szBasePath, string sParfileName, string sMd5File, FILEINFO **g_fileinfo_list_first_item)
{
	CHAR	szLineAnsi[MAX_LINE_LENGTH];
	TCHAR	szLine[MAX_LINE_LENGTH];
	TCHAR	szFilenameMd5[MAX_PATH];
	HANDLE	hFile;
	UINT	uiStringLength;
	BOOL	bErrorOccured, bEndOfFile;
	UINT	uiIndex;
	ostringstream strm;

	BOOL	bMd5OK;
	FILEINFO * pFileinfo;
	FILEINFO * pFileinfo_prev;

	// save MD5 filename and path
	// => g_szBasePath in MD5-mode is the path part of the complete filename of the .md5 file
	StringCchCopy(szFilenameMd5, MAX_PATH, sMd5File.c_str());
	StringCchCopy(g_szBasePath, MAX_PATH, szFilenameMd5);
	ReduceToPath(g_szBasePath);
	GetLongPathName(g_szBasePath, g_szBasePath, MAX_PATH);

	// This is(should be) the ONLY place where a persistent change of the current directory is done
	// (for GetFullPathName())
	if(!SetCurrentDirectory(g_szBasePath))
		return FALSE;

	hFile = CreateFile(szFilenameMd5, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN , 0);
	if(hFile == INVALID_HANDLE_VALUE){
		if (sParfileName != "")
		{
			strm.str(""); strm << "MD5 file could not be read";
			CPnrMessage::SendParAddDetails(sParfileName, strm.str(), true);
		}
		return FALSE;
	}

	(*g_fileinfo_list_first_item) = AllocateFileinfo();
	(*g_fileinfo_list_first_item)->nextListItem = NULL;

	pFileinfo = (*g_fileinfo_list_first_item);
	pFileinfo_prev = NULL;
	GetNextLine(hFile, szLineAnsi, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile);
	if(bErrorOccured){
		if (sParfileName != "")
		{
			strm.str(""); strm << "MD5 file could not be read";
			CPnrMessage::SendParAddDetails(sParfileName, strm.str(), true);
		}
		return FALSE;
	}

	while( !(bEndOfFile && uiStringLength == 0) ){

		if(uiStringLength > 34){ // a valid line has 32 hex values for the md5 value and then either "  " or " *"

#ifdef UNICODE
			MultiByteToWideChar(CP_ACP,	// ANSI Codepage
				0,						// we use no flags; ANSI isn't a 'real' MBCC
				szLineAnsi,				// the ANSI String
				-1,						// ANSI String is 0 terminated
				szLine,					// the UNICODE destination string
				MAX_LINE_LENGTH );		// size of the UNICODE String in chars
#else
			StringCchCopy(szLine, MAX_LINE_LENGTH, szLineAnsi);
#endif

			if( szLine[0] != TEXT('\0') ){
				bMd5OK = TRUE;
				for(uiIndex=0; uiIndex < 32; ++uiIndex)
					if(! IsLegalHexSymbol(szLine[uiIndex]))
						bMd5OK = FALSE;
				if(bMd5OK){
					pFileinfo->bMd5Found = TRUE;
					for(uiIndex=0; uiIndex < 16; ++uiIndex)
						pFileinfo->abMd5Found[uiIndex] = (BYTE)HexToDword(szLine + uiIndex * 2, 2);
					pFileinfo->dwError = NOERROR;
				}
				else
					pFileinfo->dwError = APPL_ERROR_ILLEGAL_CRC;

				//delete trailing spaces
				while(szLine[uiStringLength - 1] == TEXT(' ')){
					szLine[uiStringLength - 1] = NULL;
					uiStringLength--;
				}

				//find leading spaces and '*'
				uiIndex = 32; // szLine[32] is the first char after the md5
				while( (uiIndex < uiStringLength) && ((szLine[uiIndex] == TEXT(' ')) || (szLine[uiIndex] == TEXT('*'))) )
					uiIndex++;

				GetFullPathName(szLine + uiIndex, MAX_PATH, pFileinfo->szFilename, NULL);

				pFileinfo->nextListItem = AllocateFileinfo();
				pFileinfo_prev = pFileinfo;
				pFileinfo = pFileinfo->nextListItem;
			}
		}

		GetNextLine(hFile, szLineAnsi, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile);
		if(bErrorOccured){
			if (sParfileName != "")
			{
				strm.str(""); strm << "MD5 file could not be read";
				CPnrMessage::SendParAddDetails(sParfileName, strm.str(), true);
			}
			return FALSE;
		}
	}
	CloseHandle(hFile);

	if(pFileinfo_prev != NULL)
	{
		// we created one Fileinfo too much
		free(pFileinfo_prev->nextListItem);
		pFileinfo_prev->nextListItem = NULL;
	}

	return TRUE;
}
