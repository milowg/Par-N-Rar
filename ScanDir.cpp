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

#include "stdafx.h"
#include <process.h>
#include "ParNRar.h"
#include "PnrMessage.h"
#include "ScanDir.h"
#include "unrar\dll.hpp"
#include "par2-cmdline\par2cmdline.h"
#include "StringUtils.h"
#include "SfvRepairer.h"
#include "FileUtils.h"
//#include "greta\regexpr2.h"
//using namespace regex;

ScanDir::ScanDir(string sDirectory)
{
	m_sDir = sDirectory;    
};

/**************************************************************************
* This function starts the ScanDir class in a thread
*/
void ScanDir::Start(string sDirectory)
{
    unsigned int uiThreadId;
    ScanDir *pWrk = new ScanDir(sDirectory);
	    _beginthreadex( NULL, 0, ScanDir::StartThread, (void*)pWrk, 0, &uiThreadId );
}

unsigned int __stdcall ScanDir::StartThread( void* pParam )
{
    ScanDir *pWrk = (ScanDir *)pParam;
    pWrk->Main();
    delete pWrk;    
    return 0;
}

void ScanDir::Main()
{
	if (_access(m_sDir.c_str(), 0) == -1)
	{
		CPnrMessage::SendDoneError("Invalid directory.");
		return;
	}

	//Get options
	if (m_sDir.substr(m_sDir.size()-1, 1) != "\\")
		m_sDir += "\\";

	m_lFound = 0;
	ScanDirR(m_sDir, theApp.m_Options.GetRecurseMonitorDir());
	if (m_lFound == 0)
		CPnrMessage::SendNoPars();
}

void ScanDir::ScanDirR(string sDir, bool bRecurse)
{
	ostringstream strm;

	strm.str(""); strm << "Scanning " << sDir;
	theApp.Trace(strm.str().c_str());

	if (sDir.substr(sDir.size()-1, 1) != "\\")
		sDir += "\\";

	vector<string> vFilesInDir = GetFilesInDir(sDir);

	CheckSingleDir(sDir, "PAR", vFilesInDir);
	CheckSingleDir(sDir, "SFV", vFilesInDir);
	CheckSingleDir(sDir, "RAR", vFilesInDir);

	if (bRecurse)
		ScanDirChildren(sDir);
}

vector<string> ScanDir::GetFilesInDir(string sDir)
{
	vector<string> vFilesInDir;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile((sDir + "*").c_str(), &FindFileData);
	ostringstream strm;

	while (hFind != INVALID_HANDLE_VALUE) 
	{		
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			vFilesInDir.push_back(FindFileData.cFileName);
		}

		if (!FindNextFile(hFind, &FindFileData))
		{
			long l = GetLastError();
			if (l != ERROR_NO_MORE_FILES)
			{
				strm.str(""); strm << "Error in FindNextFile: " << l;
				CPnrMessage::SendDoneError(strm.str().c_str());
			}
			FindClose(hFind);
			return vFilesInDir;
			break;
		}
	}
	FindClose(hFind);
	return vFilesInDir;
}

void ScanDir::CheckSingleDir(string sDir, string sType, vector<string> &vFilesInDir)
{
	ostringstream strm;
	vector<string> vFiles;
	vector<string>::iterator fi = vFilesInDir.begin();
/*
		rpattern pattern1( ".*?\\.avi", NOCASE); 
		
		rpattern pattern2( ".*?\\.avi", NOCASE); 
		
		rpattern pattern3( ".*?\\.avi", NOCASE); 
		
		rpattern pattern4( ".*?\\.avi", NOCASE); 
		
		rpattern pattern5( ".*?\\.avi", NOCASE); 
		
		rpattern pattern6( ".*?\\.avi", NOCASE); 
		
		rpattern pattern7( ".*?\\.avi", NOCASE); 
		
		rpattern pattern8( ".*?\\.avi", NOCASE); 
		
		rpattern pattern9( ".*?\\.avi", NOCASE); 
		
		rpattern pattern10( ".*?\\.avi", NOCASE); 
		
		rpattern pattern11( ".*?\\.avi", NOCASE); 
		
		rpattern pattern12( ".*?\\.avi", NOCASE); 
*/		

	while (fi != vFilesInDir.end())
	{		
		string sRarFile = "", sRarPath = "";				
		string sFile = (*fi);
		string sFileNoExt = FileUtils::GetFileNameNoExtension(sFile);
		CString cstrExtension = FileUtils::GetFileExtension(sFile).c_str();
		cstrExtension.MakeUpper();

		IParRepairer *iparRepair = NULL;
		if (cstrExtension == "PAR" && sType == "PAR")
		{		
			Par1Repairer *repairer = new Par1Repairer(sFile);
			iparRepair = (IParRepairer *)repairer;
			vFiles = iparRepair->GetOtherFiles(sDir + sFile);
		}
		else if (cstrExtension == "PAR2" && sType == "PAR")
		{
			Par2Repairer *repairer = new Par2Repairer(sFile);
			iparRepair = (IParRepairer *)repairer;
			vFiles = iparRepair->GetOtherFiles(sDir + sFile);
		}
		else if ((cstrExtension == "SFV" || cstrExtension == "MD5") && sType == "SFV")
		{
			SfvRepairer *repairer = new SfvRepairer(sFile);
			iparRepair = (IParRepairer *)repairer;
		}
		else if ((FileUtils::DetermineSetType(sFile) == "RAR") && sType == "RAR")
		{
			strm.str(""); strm << "Found RAR " << sFile << " (" << sDir << ")";
			theApp.Trace(strm.str().c_str());

			vector<string> vTemp;
			set<string> setTemp;
			CPnrMessage::SendFoundPar(sFile, sDir, m_sDir, setTemp, vTemp );
			m_lFound++;
		}
/*		else
		{
		match_results results;
		match_results::backref_type br;

		//TODO - TRY THIS WITH THOUSANDS OF FILES
		pattern1.count( sFile);
		
		pattern2.count( sFile);
		
		pattern3.count( sFile);
		
		br = pattern4.match( sFile, results );
		
		br = pattern5.match( sFile, results );
		
		br = pattern6.match( sFile, results );
		
		br = pattern7.match( sFile, results );
		
		br = pattern8.match( sFile, results );
		
		br = pattern9.match( sFile, results );
		
		br = pattern10.match( sFile, results );
		
		br = pattern11.match( sFile, results );
		
		br = pattern12.match( sFile, results );

		//if( br.matched ) 
		//{
		//	string sPartNum;			
		//	sFileName = "";
		//	if (results.cbackrefs() == 3)
		//	{
		//		sFileName = results.backref(1).str();
		//		sPartNum = results.backref(2).str();
		//	}
		//	else
		//	{
		//		sPartNum = results.backref(1).str();
		//	}
		}
*/

		if (iparRepair != NULL)
		{
			strm.str(""); strm << "Found PAR " << sFile << " (" << sDir << ")";
			theApp.Trace(strm.str().c_str());

			CPnrMessage::SendFoundPar(sFile, sDir, m_sDir, iparRepair->GetContainedFiles(sDir + sFile), iparRepair->GetOtherFiles(sDir + sFile) );
			delete iparRepair;
			m_lFound++;
		}

		bool bDiffFile = false;
		bool bExit = false;
		while (!bDiffFile)
		{
			++fi;
			if (fi == vFilesInDir.end())
			{
				bExit = true;
				break;
			}

			string sFullpath = sDir;
			sFullpath += (*fi);
			if (find(vFiles.begin(), vFiles.end(), sFullpath) == vFiles.end())
				bDiffFile = true;
		}	
		if (bExit)
		{
			break;
		}
	}
}

void ScanDir::ScanDirChildren(string sDir)
{
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
	ostringstream strm;

	strm.str(""); strm << "Checking children directories of " << sDir;
	theApp.Trace(strm.str().c_str());

    if ((hFind = FindFirstFile((sDir + "*").c_str(), &FindFileData)) == INVALID_HANDLE_VALUE) 
        return;

    do
    {
        if (FindFileData.cFileName[0] != '.' && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        {			
            ScanDirR(sDir + FindFileData.cFileName, true);
        }

    } while(FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);
}