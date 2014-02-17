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

#include "StdAfx.h"
#include "SfvRepairer.h"
#include "PnrMessage.h"
#include "rapidcrc\globals.h"
#include <algorithm>
#include <string>

SfvRepairer::SfvRepairer(string sParfileName) : IParRepairer(sParfileName)
{
	m_bSkipVerify = false;
}

SfvRepairer::~SfvRepairer(void)
{
}

set<string> SfvRepairer::GetContainedFiles(string filename)
{
	//TODO - Clean this code up (take code from VerifyPar and combine with this code in this class)
	FILEINFO *g_fileinfo_list_first_item = NULL;
	THREAD_PARAMS_CALC pthread_params_calc;
	TCHAR g_szBasePath[MAX_PATH];
	PROGRAM_STATUS g_program_status;
	set<string> setFiles;

	string m_sExtension = filename.substr(filename.find_last_of('.')+1);
	transform(m_sExtension.begin(), m_sExtension.end(), m_sExtension.begin(), toupper);

	g_program_status.bCrcCalculated = FALSE;
	g_program_status.bMd5Calculated = FALSE;
	pthread_params_calc.bCalculateMd5 = FALSE;
	pthread_params_calc.bCalculateCrc = FALSE;	
	if (m_sExtension == "SFV")
	{
		pthread_params_calc.bCalculateCrc = true;
		g_program_status.uiRapidCrcMode = MODE_SFV;
		EnterSfvMode(g_szBasePath, "", filename, &g_fileinfo_list_first_item);
	}
	else if (m_sExtension == "MD5")
	{
		g_program_status.uiRapidCrcMode = MODE_MD5;
		pthread_params_calc.bCalculateMd5 = true;
		EnterMd5Mode(g_szBasePath, "", filename, &g_fileinfo_list_first_item);
	}
	else
		return setFiles;

	if (g_fileinfo_list_first_item == NULL)
	{
		//File is not there or something is wrong
		return setFiles;
	}

	FILEINFO *pFileInfo = g_fileinfo_list_first_item;
	do
	{
		setFiles.insert( pFileInfo->szFilename );
		pFileInfo = pFileInfo->nextListItem;
	}while (pFileInfo != NULL);

	DeallocateFileinfoMemory(g_fileinfo_list_first_item);

	return setFiles;
}

bool SfvRepairer::Repair(string sDir, string sFile, vector<string> &vFiles, vector<string> &vRenamedFiles, vector<string> &vVerifiedFiles, ParSettings &fileSettings, vector<string> &vDoneFiles, bool bVerifyOnly)
{
	bool bRet = false;
	ostringstream strm;

	m_bSkipVerify = false;
	strm.str(""); strm << "Checking SFV file " << sDir << sFile ;
	CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

	FILEINFO *g_fileinfo_list_first_item;
	THREAD_PARAMS_CALC pthread_params_calc;
	TCHAR g_szBasePath[MAX_PATH];
	QWORD pqwFilesizeSum;
	PROGRAM_STATUS g_program_status;

	g_program_status.bCrcCalculated = FALSE;
	g_program_status.bMd5Calculated = FALSE;
	pthread_params_calc.bCalculateMd5 = FALSE;
	pthread_params_calc.bCalculateCrc = FALSE;	

	string sExtension = m_sParfileName.substr(m_sParfileName.find_last_of('.')+1);
	transform(sExtension.begin(), sExtension.end(), sExtension.begin(), toupper);

	if (sExtension == "SFV")
	{
		pthread_params_calc.bCalculateCrc = true;
		g_program_status.uiRapidCrcMode = MODE_SFV;
		bRet = EnterSfvMode(g_szBasePath, m_sParfileName, sDir + sFile, &g_fileinfo_list_first_item);
	}
	else if (sExtension == "MD5")
	{
		g_program_status.uiRapidCrcMode = MODE_MD5;
		pthread_params_calc.bCalculateMd5 = true;
		bRet = EnterMd5Mode(g_szBasePath, m_sParfileName, sDir + sFile, &g_fileinfo_list_first_item);
	}
	else
	{
		goto CLEANUP;
	}
	
	if (bRet)
	{
		ProcessFileProperties(g_szBasePath, &g_program_status, g_fileinfo_list_first_item, &pqwFilesizeSum);
		pthread_params_calc.pFileinfo_cur = g_fileinfo_list_first_item;
		pthread_params_calc.qwBytesReadAllFiles = 0;
		pthread_params_calc.qwBytesReadCurFile = 0;
		bRet = ThreadProc_Calc(&m_bSkipVerify, m_sParfileName, &m_vVerified, &pthread_params_calc, &g_program_status, g_fileinfo_list_first_item);	
			
		if (m_bSkipVerify) 
		{
			CPnrMessage::SendParDone(m_sParfileName, "Skipped", false, DONE);
			bRet = false;
			goto CLEANUP; //Check for skipped verification
		}

		//Get a list of all files looked at. Since there is only one SFV/MD5 file, vVerifiedFiles and vFiles are the same.
		FILEINFO *pFileInfo = g_fileinfo_list_first_item;
		do
		{
			vFiles.push_back( pFileInfo->szFilename );
			vVerifiedFiles.push_back( pFileInfo->szFilename );
			pFileInfo = pFileInfo->nextListItem;
		}while (pFileInfo != NULL);

		if (bRet)
			CPnrMessage::SendParAddDetails(m_sParfileName, "Passed Verification", true);
		else
		{
			eStatus status = DONE;
			if (bVerifyOnly)
				status = VERIFIED;

			CPnrMessage::SendParDone(m_sParfileName, "Failed Verification", false, status);
		}
	}

CLEANUP:
	//Clean up
	DeallocateFileinfoMemory(g_fileinfo_list_first_item);

	return bRet;
}