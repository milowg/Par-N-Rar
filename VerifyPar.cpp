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
#include "VerifyPar.h"
#include "unrar\dll.hpp"
#include "rapidcrc\globals.h"
#include "par2-cmdline\par2cmdline.h"
#include "StringUtils.h"
#include "SplitFiles.h"
#include "SfvRepairer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "FileUtils.h"
#include "ScanDir.h"

//From unrar/filefn.cpp
extern void CreatePath(string sParfileName, const char *Path, const WCHAR *PathW,bool SkipLastName);

VerifyPar::VerifyPar(string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir, bool bRemoveFromList, vector<string> vOtherFilesToDelete)
{
	m_Status = STATUS_NONE;
	m_bVerified = false;
	m_bVerifyOnly = false;
	m_Repairer = NULL;
	m_bRemoveFromList = bRemoveFromList;
	m_sParfileName = sParfileName;
	m_sParfilePath = sParfilePath;
	m_sPathOffMonitorDir = sPathOffMonitorDir;
	m_sMonitorDir = sMonitorDir;
	m_vOtherFilesToDelete = vOtherFilesToDelete;
};

VerifyPar::VerifyPar(bool bVerifyOnly, string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir)
{
	m_Status = STATUS_NONE;
	m_bVerified = false;
	m_bVerifyOnly = bVerifyOnly; 
	m_Repairer = NULL;
	m_bRemoveFromList = false;
	m_sParfileName = sParfileName;
	m_sParfilePath = sParfilePath;
	m_sPathOffMonitorDir = sPathOffMonitorDir;
	m_sMonitorDir = sMonitorDir;
};

VerifyPar::VerifyPar(string sParfileName, string sMonitorDir)
{
	m_Status = STATUS_NONE;
	m_bVerified = false;
	m_bVerifyOnly = false;
	m_Repairer = NULL;
	m_sParfileName = sParfileName;
	m_sMonitorDir = sMonitorDir;
}

VerifyPar::~VerifyPar()
{
	DeleteRepairer();
};

/**************************************************************************
* These functions run the VerifyPar class in a thread
*/
void VerifyPar::StartVerify(bool bVerifyOnly, VerifyPar **ppVerifyPar, string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir)
{
    unsigned int uiThreadId;
	if (*ppVerifyPar == NULL)
	{
		*ppVerifyPar = new VerifyPar(bVerifyOnly, sParfileName, sParfilePath, sPathOffMonitorDir, sMonitorDir);
	}
	else
		(*ppVerifyPar)->m_bVerifyOnly = bVerifyOnly;
	_beginthreadex( NULL, 0, VerifyPar::StartThreadVerify, (void*)*ppVerifyPar, 0, &uiThreadId );
}

void VerifyPar::StartDelete(VerifyPar **ppVerifyPar, string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir, bool bRemoveFromList)
{
	vector<string> vDummy;
	StartDelete(ppVerifyPar, sParfileName, sParfilePath, sPathOffMonitorDir, sMonitorDir, bRemoveFromList, vDummy);
}

void VerifyPar::StartDelete(VerifyPar **ppVerifyPar, string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir, bool bRemoveFromList, vector<string> vOtherFilesToDelete)
{
    unsigned int uiThreadId;
	if (*ppVerifyPar == NULL)
	{
		*ppVerifyPar = new VerifyPar(sParfileName, sParfilePath, sPathOffMonitorDir, sMonitorDir, bRemoveFromList, vOtherFilesToDelete);
	}    
	_beginthreadex( NULL, 0, VerifyPar::StartThreadDelete, (void*)*ppVerifyPar, 0, &uiThreadId );
}

unsigned int __stdcall VerifyPar::StartThreadVerify( void* pParam )
{
    VerifyPar *pWrk = (VerifyPar *)pParam;
	pWrk->Initialize();
    pWrk->MainVerify();
    return 0;
}

unsigned int __stdcall VerifyPar::StartThreadDelete( void* pParam )
{
    VerifyPar *pWrk = (VerifyPar *)pParam;
    pWrk->Initialize();
	pWrk->MainDelete();
    return 0;
}

void VerifyPar::DeleteRepairer()
{
	if (m_Repairer != NULL)
		delete m_Repairer;
	m_Repairer = NULL;
}

int VerifyPar::RARExtract(vector<string> *pvRarFiles, string sRARArchive, string sDestPath, string sParName, string sExtractPath)
{	
	HANDLE hHandle;
	long lStatus;
	RAROpenArchiveData uRAR;
	RARHeaderDataEx uHeader; 
	int iFileCountFinal, iFileCount;
	ostringstream strm;

	//Find out how many files there are for display purposes
	iFileCount = RARFileCount(sRARArchive);

	//Extract..
	auto_ptr<char> apTemp( new char[sRARArchive.size() + 1] );
	uRAR.ArcName = apTemp.get();
	strcpy(uRAR.ArcName, sRARArchive.c_str());
	uRAR.OpenMode = RAR_OM_EXTRACT;
	hHandle = RAROpenArchive(pvRarFiles, m_sParfileName, &uRAR);

	if( uRAR.OpenResult != 0)
		return -1;

	iFileCountFinal = 0;
	
	//Look for a file within the RAR.. 
	int iCount = 1;
	lStatus = RARReadHeaderEx(hHandle, &uHeader, m_sParfileName);
	while (lStatus == 0)
	{
		string sDestFile, sDestPath;
		string sRarFolder = uHeader.FileName;
		string sFileNameNoExt = uHeader.FileName;

		//For RAR files with folders, get sub folder
		if (sRarFolder.find_last_of('\\') != sRarFolder.npos)
		{
			sFileNameNoExt = sRarFolder.substr(sRarFolder.find_last_of('\\') + 1);
			sRarFolder = sRarFolder.substr(0, sRarFolder.find_last_of('\\')) + "\\";			
		}
		else
		{
			sRarFolder = "";
		}

		GetGoodFileName(sExtractPath + "\\" + sRarFolder + sFileNameNoExt, sDestFile, sDestPath);

		auto_ptr<char> apDestPath( new char[sDestPath.size() + 1] );
		char *szDestPath = apDestPath.get();
		strcpy(szDestPath, sDestPath.c_str());
		
		auto_ptr<char> apDestName( new char[sDestFile.size() + 1] );
		char *szDestName = apDestName.get();
		strcpy(szDestName, sDestFile.c_str());

		strm.str(""); strm << "Extracting (" << iCount++ << "/" << iFileCount << ")";
		CPnrMessage::SendParStatus(m_sParfileName, strm.str().c_str());
		CPnrMessage::SendParProgress(m_sParfileName, "0");

		//Write out to settings which file we are extracting
		theApp.m_Options.SetStringOption("ExtractFile", (sDestPath + "\\" + sRarFolder + sDestFile).c_str());
	
		long lRet = RARProcessFile(hHandle, RAR_EXTRACT, szDestPath, szDestName, m_sParfileName);
		
		theApp.m_Options.SetStringOption("ExtractFile", "");

		if (lRet == 21)
		{
			//Password error
			RARCloseArchive (hHandle);
			return -21;
		}
		else if( lRet != 0)
		{
			strm.str(""); strm << "Error extracting " << szDestName;
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str().c_str(), true);
			RARCloseArchive (hHandle);
			return -1;
		}
		else
			iFileCountFinal++;

		CPnrMessage::SendParProgress(m_sParfileName, "100");

		//Look for more files within the RAR
		lStatus = RARReadHeaderEx(hHandle, &uHeader, m_sParfileName);
	}

	RARCloseArchive (hHandle);
	return iFileCountFinal;

}

int VerifyPar::RARFileCount(string sRARArchive)
{
	vector<string> dummy;
	return RARFileCount(sRARArchive, &dummy);
}

int VerifyPar::RARFileCount(string sRARArchive, vector<string> *pvRarFiles)
{
	HANDLE hHandle;
	long lStatus;
	RAROpenArchiveData uRAR;
	RARHeaderDataEx uHeader; 
	int iFileCount;

	auto_ptr<char> apTemp( new char[sRARArchive.size() + 1] );
	uRAR.ArcName = apTemp.get();
	strcpy(uRAR.ArcName, sRARArchive.c_str());
	uRAR.OpenMode = RAR_OM_LIST;
	vector<string> dummy;
	hHandle = RAROpenArchive(&dummy, m_sParfileName, &uRAR);

	if( uRAR.OpenResult != 0)
		return -1;

	iFileCount = 0;
	
	//Look for a file within the RAR.. First find out how many there are
	lStatus = RARReadHeaderEx(hHandle, &uHeader, m_sParfileName);
	string sCurfile = "";
	while (lStatus == 0)
	{			
		if( RARProcessFile(hHandle, RAR_SKIP, "", "", m_sParfileName) == 0)
		{
			if (strcmp(uHeader.FileName, sCurfile.c_str()))
			{
				sCurfile = uHeader.FileName;
				pvRarFiles->push_back(sCurfile);
				iFileCount++;
			}
		}
		lStatus = RARReadHeaderEx(hHandle, &uHeader, m_sParfileName);
	}

	RARCloseArchive (hHandle);
	return iFileCount;

}

void VerifyPar::Initialize()
{
	m_sFileNoExt = m_sParfileName.substr(0, m_sParfileName.find_last_of('.'));
	m_sExtension = m_sParfileName.substr(m_sParfileName.find_last_of('.')+1);
	transform(m_sExtension.begin(), m_sExtension.end(), m_sExtension.begin(), toupper);
}

void VerifyPar::MainDeleteFiles(vector<string> vDeleteFiles)
{	
	vector<string>::iterator fi = vDeleteFiles.begin();		
	while (fi != vDeleteFiles.end())
	{
		if (_access((*fi).c_str(), 0) != -1)
		{
			MyDeleteFile((*fi).c_str());
		}
		++fi;
	}
}

void VerifyPar::MainDelete()
{
	vector<string> vFiles;
	bool bBypassFirstfileCheck;

	m_Status = STATUS_DELETING;
	DeleteRepairer(); //Make sure no file handles are still open

	if (FileUtils::DetermineSetType(m_sParfileName) == "RAR")
	{
		//If this is a RAR set the files found for the set will be in the OtherFiles collection
		copy(m_vOtherFilesToDelete.begin(), m_vOtherFilesToDelete.end(), back_inserter(vFiles));	

		vFiles.push_back(m_sParfilePath + m_sParfileName);
		bBypassFirstfileCheck = true;
	}
	else
	{
		LoadRepairer();
		vFiles = m_Repairer->GetOtherFiles(m_sParfilePath + m_sParfileName);
		vFiles.push_back(m_sParfilePath + m_sParfileName);
		vFiles.push_back(m_sParfilePath + m_sFileNoExt + ".pnr"); //Add pnr file to set to delete if there

		//Insert any other files we have to delete
		copy(m_vOtherFilesToDelete.begin(), m_vOtherFilesToDelete.end(), back_inserter(vFiles));	

		set<string> setContainedFiles = m_Repairer->GetContainedFiles(m_sParfilePath + m_sParfileName);
		copy(setContainedFiles.begin(), setContainedFiles.end(), back_inserter(vFiles));
		bBypassFirstfileCheck = false;
	}

	//Make sure file still exists
	if ( bBypassFirstfileCheck || (_access((m_sParfilePath + m_sParfileName).c_str(), 0) != -1) )
	{
		sort(vFiles.begin(), vFiles.end());
		vector<string>::iterator fi = vFiles.begin();		
		while (fi != vFiles.end())
		{
			if (_access((*fi).c_str(), 0) != -1)
			{
				MyDeleteFile((*fi).c_str());
			}
			++fi;
		}
	}

	if (m_bRemoveFromList)
		CPnrMessage::SendRemove(m_sParfileName);
}

void VerifyPar::LoadRepairer()
{	
	if (m_Repairer == NULL)
	{
		if (m_sExtension == "PAR")				
		{
			Par1Repairer *repairer = new Par1Repairer(m_sParfileName);
			m_Repairer = (IParRepairer *)repairer;			
		}
		else if (m_sExtension == "PAR2")
		{
			Par2Repairer *repairer = new Par2Repairer(m_sParfileName);
			m_Repairer = (IParRepairer *)repairer;			
		}
		else if (m_sExtension == "SFV" || m_sExtension == "MD5")
		{
			SfvRepairer *repairer = new SfvRepairer(m_sParfileName);
			m_Repairer = (IParRepairer *)repairer;			
		}
		else
			return;
	}
	m_Repairer->m_bSkipVerify = false;
}

//TODO - refactor this function up into smaller chunks
void VerifyPar::MainVerify()
{
	ostringstream strm;
	ParSettings fileSettings;
	vector<string> vFiles, vVerifiedFiles;
	string sRarPath = "";				

	m_Status = STATUS_VERIFYING;

	fileSettings.m_sFile = m_sParfileName;
	fileSettings.m_sFilePathNoExt = m_sParfilePath + m_sFileNoExt;
	fileSettings.m_sFileNoExt = m_sFileNoExt;
	fileSettings.m_sFilePath = m_sParfilePath + m_sParfileName; 

	//If we already validated this file, do not check it
	if (_access((fileSettings.m_sFilePathNoExt + ".pnr").c_str(), 0) != -1)		
		LoadParSettings(&fileSettings);

	if (fileSettings.m_bFileValidated)
	{
		CPnrMessage::SendParDone(m_sParfileName, "Already Verified", false, DONE);
		return;
	}

	bool bParDone = false;
	vector<string> vDoneFiles;
	vector<string> vRenamedFiles; //Files renamed during repair (ie. file.ext.1)

	if (FileUtils::DetermineSetType(m_sParfileName) != "RAR")
	{
		LoadRepairer();				
		bParDone = m_Repairer->Repair(m_sParfilePath, m_sParfileName, vFiles, vRenamedFiles, vVerifiedFiles, fileSettings, vDoneFiles, m_bVerifyOnly);
		if (!bParDone)
			return;

		fileSettings.m_bFileValidated = true;
		m_bVerified = true;
		
		if (theApp.m_Options.GetDeleteWhenDone())
		{
			//Delete any renamed files, since they are replaced by good files
			vector<string>::iterator fiRenamed = vRenamedFiles.begin();
			while (fiRenamed != vRenamedFiles.end())
			{
				MyDeleteFile((*fiRenamed).c_str());
				++fiRenamed;
			}
		}

		//If we are only verifying, then do not continue
		if (m_bVerifyOnly)
		{
			CPnrMessage::SendParDone(m_sParfileName, "Verified", false, VERIFIED);
			return;
		}

		//Put the original PAR file on the list
		vFiles.push_back( m_sParfilePath + m_sParfileName );
	}

	CPnrMessage::SendPareStatus(m_sParfileName, VERIFIED);
	m_Status = STATUS_EXTRACTING;
	CPnrMessage::SendParProgress(m_sParfileName, "0");

	string sExtractPath;
	vector<string> vRarFiles;
	bool bMoveNonRar = theApp.m_Options.GetMoveNonRar();

	sExtractPath = theApp.m_Options.GetExtractDir();			

	if (sExtractPath.substr(sExtractPath.size(), 1) == "\\")
		sExtractPath = sExtractPath.substr(0, sExtractPath.size() - 1);

	//Replace variable names
	sExtractPath = theApp.m_Options.ConvertDirectory(sExtractPath, m_sFileNoExt, m_sPathOffMonitorDir, m_sMonitorDir);
	
	bool bDeleted;
	bool bInvalidRar = false, bInvalidRarDeleted = false;

	if (vDoneFiles.size() != 0)
	{
		//We already have the files done. No need to extract.
		DeleteRepairer(); //Make sure no file handles are still open			

		//Move "done files" to done directory if necessary
		vector<string>::iterator fiDone = vDoneFiles.begin();
		while (fiDone != vDoneFiles.end())
		{
			if (_access((*fiDone).c_str(), 0) != -1)
			{
				//We don't want to delete this file if it fails to move
				vector<string>::iterator iterExtraFile = find(vFiles.begin(), vFiles.end(), (*fiDone));
				if (iterExtraFile != vFiles.end())
					vFiles.erase(iterExtraFile);

				if (sExtractPath != "")
				{
					string sDestForMove = GetDestinationForMove((*fiDone), sExtractPath);

					string sDummy, sDestPath;
					sDestForMove = GetGoodFileName(sDestForMove, sDummy, sDestPath);

					strm.str(""); strm << "Moving " << (*fiDone) << " to " << sDestForMove;
					CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);					

					//Make sure directory exists
					CreatePath(m_sParfileName, sDestPath.c_str(), NULL, false);
					if (MoveFile((*fiDone).c_str(), sDestForMove.c_str()) == 0)
					{
						strm.str(""); strm << "Failed to move file (" << FileUtils::ErrorString(GetLastError()) << ")" ;
						CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
					}
				}
			}
			fiDone++;
		}

		//Put all remaining files in vFiles to mark for deletion
		bDeleted = theApp.m_Options.GetDeleteWhenDone();
	}
	else
	{
		//Get next Rar in set and extract it
		string sRarFile;
		bool bRarFile;
		if (FileUtils::DetermineSetType(m_sParfileName) != "RAR")
		{
			bRarFile = false;
			sRarFile = GetRarFileInSet(vVerifiedFiles);
		}
		else
		{
			bRarFile = true;
			sRarFile = m_sParfilePath + m_sParfileName;
		}

		if (sRarFile == "")
		{
			CPnrMessage::SendParAddDetails(m_sParfileName, "No RAR file found to unRAR", true);					
			HandleInvalidRarSet(vVerifiedFiles, fileSettings, bDeleted);
			bInvalidRar = true;
			bInvalidRarDeleted = (theApp.m_Options.GetNonRarDir() != "" && bDeleted);
		}

		while (sRarFile != "")
		{			
			//Get Extract Directory
			sRarPath = sRarFile.substr(0, sRarFile.find_last_of('\\'));

			if (sExtractPath == "")
				sExtractPath = sRarPath;

			bDeleted = false;
			long lRet;

			//Check for RAR files in archive. If any are found, change extract dir to RAR file dir
			vector<string> vRarFiles;
			RARFileCount(sRarFile, &vRarFiles);
			bool bSetContainsRarFiles = SetContainsRarFiles(vRarFiles);
			if (bSetContainsRarFiles)
			{
				strm.str(""); strm << "Set contains RAR files. Extracting to same directory as set.";
				CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
				sExtractPath = sRarPath;
			}

			vector<string> vRarFilesInst = ExtractFiles(sRarFile, sRarPath, m_sFileNoExt, sExtractPath, lRet);
			if (lRet == -21)
			{
				//Password error..
				CPnrMessage::SendParDone(m_sParfileName, "Password required", false, DONE);
				return;
			}
			else if (vRarFilesInst.size() == 0)
			{
				if (bRarFile)
				{
					//If this is a "rar-only" set and it fails extraction, then that's ok.. 
					strm.str(""); strm << "Could not extract RAR set";
					CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
					CPnrMessage::SendParDone(m_sParfileName, "Can not extract", false, DONE);
					return;
				}

				//If the "rar" file is a .001 file that fails extraction then assume it is a split file and join it
				if ((sRarFile.length() > 4) && (sRarFile.substr(sRarFile.length() - 4) == ".001"))
				{
					vRarFilesInst = SplitFiles::JoinFiles(m_sParfileName, sRarFile, sExtractPath);
				}

				if (vRarFilesInst.size() == 0)
				{
					//This is truly an invalid RAR set
					bInvalidRar = true;
					HandleInvalidRarSet(vVerifiedFiles, fileSettings, bDeleted);					
					vVerifiedFiles.clear();
					break;
				}				
			}			

			if (bSetContainsRarFiles)
				ScanDir::Start(sRarPath);

			//Delete RAR files if they should be deleted
			vector<string>::iterator fi = vRarFilesInst.begin();

			DeleteRepairer(); //Make sure no file handles are still open
			while (fi != vRarFilesInst.end())
			{
				if (theApp.m_Options.GetDeleteWhenDone())
				{
					//Delete the RAR file
					bDeleted = true;

					if (_access((*fi).c_str(), 0) != -1)
					{
						MyDeleteFile((*fi).c_str());
					}
				}

				//Remove the RAR file from the verified list
				vector<string>::iterator iterRarFiles;
				iterRarFiles = find(vVerifiedFiles.begin(), vVerifiedFiles.end(), (*fi));
				if (iterRarFiles != vVerifiedFiles.end())
					vVerifiedFiles.erase(iterRarFiles);

				++fi;
			}

			copy(vRarFilesInst.begin(), vRarFilesInst.end(), back_inserter(vRarFiles));
			sRarFile = GetRarFileInSet(vVerifiedFiles);
		}

		//Finish up
		vector<string>::iterator fi = vVerifiedFiles.begin();
		while (fi != vVerifiedFiles.end())
		{
			//Check if this file is not related to the RAR set, and is one of the verified files (ie. not in the PAR set either)
			if ( bMoveNonRar && (find(vRarFiles.begin(), vRarFiles.end(), (*fi)) == vRarFiles.end()) )
			{
				DeleteRepairer(); //Make sure no file handles are still open

				if (sExtractPath != "")
				{
					if (_access((*fi).c_str(), 0) != -1)
					{
						//This file is not in the RAR set, so should be moved to destination directory (if option is set)
						string sDestForMove = GetDestinationForMove((*fi), sExtractPath);
						if (sDestForMove == (*fi))
						{
							//The files are the same, which means the done directory is the same as monitor directory.
							//	Since we have "move extra files" set, then we want to keep this file.
							vector<string>::iterator iterExtraFile = find(vFiles.begin(), vFiles.end(), (*fi));
							if (iterExtraFile != vFiles.end())
								vFiles.erase(iterExtraFile);
						}
						else
						{
							//Check for files we do not want
							string sExt = StringUtils::FileExtension((*fi));
							if (sExt != "SFV")
							{
								strm.str(""); strm << "Moving extra file " << (*fi) << " to " << sDestForMove;
								CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);					
								if (MoveFile((*fi).c_str(), sDestForMove.c_str()) == 0)
								{
									strm.str(""); strm << "Failed to move file (" << FileUtils::ErrorString(GetLastError()) << ")" ;
									CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
								}
							}
						}
					}
				}
			}
			++fi;
		}
	}

	if (bDeleted)
	{
		//Delete the rest of the files in the PAR set, including the pnr file if it is there
		vFiles.push_back(m_sParfilePath + m_sFileNoExt + ".pnr");
		vector<string>::iterator fi = vFiles.begin();
		while (fi != vFiles.end())
		{
			if (_access((*fi).c_str(), 0) != -1)
			{
				MyDeleteFile((*fi).c_str());
			}
			++fi;
		}

	}
	else
	{
		//Write to PNR file so we don't attempt to validate this again			
		SaveParSettings(&fileSettings);
	}

	if (bInvalidRar)
		CPnrMessage::SendParDone(m_sParfileName, "Done", bInvalidRarDeleted, DONE);
	else
		CPnrMessage::SendParDone(m_sParfileName, "Done", bDeleted, DONE);
}

void VerifyPar::HandleInvalidRarSet(vector<string> &vVerifiedFiles, ParSettings &fileSettings, bool &bDeleted)
{		

	//Check if we should move the file(s) and delete the par files
	string sNonRarDir = theApp.m_Options.GetNonRarDir();
	if (sNonRarDir != "")
	{
		if (MoveFiles(vVerifiedFiles, fileSettings, sNonRarDir))
			bDeleted = true;
	}
	else
	{
		bDeleted = false;
		SaveParSettings(&fileSettings);
	}	
}

string VerifyPar::GetRarFileInSet(vector<string> &vFiles)
{
	string sRarFile = "";
	vector<string>::iterator fi = vFiles.begin();
	while (fi != vFiles.end())
	{
		string sFile = *fi;
		//Look for .rar or .001 files
		if ((sFile.find(".rar", 0) == sFile.size()-4) || (sFile.find(".001", 0) == sFile.size()-4))
		{
			//Get the first rar file
			if (sRarFile != "")
			{
				if (sRarFile > sFile)
					sRarFile = sFile;
			}
			else
				sRarFile = sFile;
		}
		++fi;
	}
	return sRarFile;
}

bool VerifyPar::MoveFiles(vector<string> vFiles, ParSettings &pSettings, string sNonRarDir)
{
	bool bRet = true;
	vector<HANDLE> vFileHandles;
	ostringstream strm;

	DeleteRepairer(); //Make sure no file handles are still open

	//Replace variable names
	sNonRarDir = theApp.m_Options.ConvertDirectory(sNonRarDir, pSettings.m_sFileNoExt, m_sPathOffMonitorDir, m_sMonitorDir);

	strm.str(""); strm << "Moving files to " << sNonRarDir;
	CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

	vector<string>::iterator fi = vFiles.begin();
	vector<HANDLE>::iterator hi;
	while (fi != vFiles.end())
	{
		HANDLE hf = CreateFile((*fi).c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);
		if (hf != INVALID_HANDLE_VALUE)
		{
			vFileHandles.push_back(hf);
		}
		else
		{
			//Error occurred. Release handles and return false;
			hi = vFileHandles.begin();
			while (hi != vFileHandles.end())
			{
				CloseHandle(*hi);
				hi++;
			}

			bRet = false;
			goto CLEANUP;
		}
		++fi;
	}

	//Make sure directory exists
	CreatePath(m_sParfileName, sNonRarDir.c_str(), NULL, false);

	//Move files
	fi = vFiles.begin();
	hi = vFileHandles.begin();
	while (fi != vFiles.end())
	{
		if (_access((*fi).c_str(), 0) != -1)
		{
			strm.str(""); strm << "Moving file " << (*fi);
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
			CloseHandle(*hi);
			if (MoveFile((*fi).c_str(), GetDestinationForMove((*fi), sNonRarDir).c_str()) == 0)
			{
				strm.str(""); strm << "Failed to move file (" << FileUtils::ErrorString(GetLastError()) << ")" ;
				CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
				bRet = false;
			}
			vFiles.erase(fi);			
			fi = vFiles.begin();
			vFileHandles.erase(hi);			
			hi = vFileHandles.begin();
		}
		else
		{
			CloseHandle(*hi);
			++fi;
			++hi;
		}
	}

CLEANUP:
	//Delete pnr file if it is there
	string sPnrFile = pSettings.m_sFilePathNoExt + ".pnr";
	if (_access(sPnrFile.c_str(), 0) != -1)
		MyDeleteFile(sPnrFile.c_str());

	return bRet;
}

string VerifyPar::GetDestinationForMove(string sSrcPath, string sDestDir)
{
	string sDestPath;

	//Trim spaces
	StringUtils::trim(sDestDir);
	StringUtils::trim(sSrcPath);

	if (sDestDir.substr(sDestDir.size()-1, 1) != "\\")
		sDestDir += "\\";
	if (sSrcPath.find_last_of('\\') != sSrcPath.npos)
		sSrcPath = sSrcPath.substr(sSrcPath.find_last_of('\\') + 1);

	sDestPath = sDestDir;
	sDestPath += sSrcPath;			
	return sDestPath;
}

void VerifyPar::SaveParSettings(ParSettings *pSettings)
{
	if (pSettings == NULL)
		return;

	int result;
	CFile file;
	CFileException exception;

	result = file.Open((pSettings->m_sFilePathNoExt + ".pnr").c_str(), CFile::modeCreate | CFile::modeWrite, &exception);

	if ( result == 0 )
	{	
		SendFileError(&exception, "Failed to create file ", pSettings);
		return;
	}

	CArchive archive( &file, CArchive::store );

	try 
	{
		pSettings->Serialize( archive );
	}
	catch ( CException *err )
	{
		SendFileError(err, "Failed to write to file ", pSettings);
		archive.Close();
		file.Close();
		return;
	}

	archive.Close();
	file.Close();
}

void VerifyPar::LoadParSettings(ParSettings *pSettings)
{	
	if ( pSettings == NULL )
		return;

	int result;
	CFile file;
	CFileException exception;

	result = file.Open( (pSettings->m_sFilePathNoExt + ".pnr").c_str(), CFile::modeRead, &exception );

	if ( result == 0 )
	{	
		SendFileError(&exception, "Failed to read from file ", pSettings);
		return;
	}

	CArchive archive( &file, CArchive::load );

	try {
		pSettings->Serialize( archive );
	}
	catch ( CException *err )
	{
		SendFileError(err, "Failed to read fromfile ", pSettings);
		archive.Close();
		file.Close();
		return;
	}

	archive.Close();
	file.Close();
}

void VerifyPar::SendFileError(CException *err, string sAction, ParSettings *pSettings)
{
	ostringstream strm;

	TCHAR szCause[255];
	err->GetErrorMessage(szCause, 255);
	strm << sAction << pSettings->m_sFilePathNoExt.c_str() << ".pnr -- " <<  szCause ;	
	CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
}

vector<string> VerifyPar::ExtractFiles(string sRarFile, string sRarPath, string sFileNoExt, string sExtractPath, long &lRet)
{
	vector<string> vRarFiles;

	lRet = RARExtract(&vRarFiles, sRarFile, sRarPath, sFileNoExt, sExtractPath);
	if (lRet <= 0)
	{
		vRarFiles.clear();
	}
	return vRarFiles;
}

bool VerifyPar::MyDeleteFile(const char *szFile)
{

	ostringstream strm;
	DeleteRepairer(); //Make sure no file handles are still open
	bool bRet = false;
	if (!theApp.m_Options.GetUseRecycleBin())
	{
		strm.str(""); strm << "Deleting file: " << szFile ;
		CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), false);
		bRet = DeleteFile(szFile) == TRUE;
	}
	else
	{
		strm.str(""); strm << "Sending file to Recycle Bin: " << szFile ;
		CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), false);
		bRet = FileUtils::SendToRecycleBin(szFile);
	}

	if (!bRet)
	{
		strm.str(""); strm << "Failed to delete file (" << FileUtils::ErrorString(GetLastError()) << ")" ;
		CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), false);
	}
	else if (theApp.m_Options.GetDeleteEmptyDirs())
	{
		strm.str(""); strm << "[" << m_sParfileName << "] Checking for empty directories to delete";
		theApp.Trace("%s", strm.str().c_str());

		string sFile = szFile;
		string sDir = "";
		long lSlashPos = sFile.find_last_of('\\');	
		if (lSlashPos != sFile.npos)
		{
			sDir = sFile.substr(0, sFile.find_last_of('\\'));
		}
		else
		{
			sDir = sFile;
		}

		//Check if the directory is empty (or if it has subdirs, if they are empty) and if so, delete it
		bRet = DeleteDirIfEmpty(sDir);
		if (bRet == false)
		{
			strm.str(""); strm << "Deleting directory failed (" << FileUtils::ErrorString(GetLastError()) << ")";
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), false);
		}

		//Recursively check up to monitor directory and delete any empty directories
		CString cstrDirUpper = sDir.c_str(), cstrMonitorDirUpper = m_sMonitorDir.c_str();
		cstrDirUpper.MakeUpper();
		cstrMonitorDirUpper.MakeUpper();
		while ( (cstrDirUpper != cstrMonitorDirUpper) && (cstrDirUpper.GetLength() >= cstrMonitorDirUpper.GetLength()) && (bRet == true) )
		{
			lSlashPos = sDir.find_last_of('\\');	
			if (lSlashPos != sDir.npos)
			{
				sDir = sDir.substr(0, sDir.find_last_of('\\'));
			}
			cstrDirUpper = sDir.c_str();
			cstrDirUpper.MakeUpper();

			strm.str(""); strm << "CurrentDir = " << cstrDirUpper.GetBuffer(0) << " (len=" << cstrDirUpper.GetLength() << "), MonitorDir = " << cstrMonitorDirUpper.GetBuffer(0) << "(len=" << cstrMonitorDirUpper.GetLength() << ")";
			theApp.Trace("%s", strm.str().c_str());

			if ((cstrDirUpper != cstrMonitorDirUpper) && cstrDirUpper.GetLength() >= cstrMonitorDirUpper.GetLength())
			{
				bRet = DeleteDirIfEmpty(sDir);
			}
		}
	}
	return bRet;

}

bool VerifyPar::DeleteDirIfEmpty(string sDir)
{	
	ostringstream strm;
	strm << "[" << m_sParfileName << "] Checking if directory is empty (" << sDir << ")";
	theApp.Trace("%s", strm.str().c_str());

	if (FileUtils::DirIsEmptyR(sDir.c_str()))
	{
		if (!theApp.m_Options.GetEmptyDirsUseRecycleBin())
		{
			strm.str(""); strm << "Deleting directory: " << sDir ;
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), false);
			return RemoveDirectory(sDir.c_str()) == TRUE;
		}
		else
		{
			strm.str(""); strm << "Sending directory to Recycle Bin: " << sDir ;
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), false);
			return FileUtils::SendToRecycleBin(sDir.c_str());
		}
	}	
	else
	{
		strm.str(""); strm << "[" << m_sParfileName << "] Directory is not empty (" << sDir << ")";
		theApp.Trace("%s", strm.str().c_str());
	}
	return true;
}

//GetGoodFileName
//	Returns a valid file name to open/copy to. This file name cannot exist.
//  Change file name if it already exists
string VerifyPar::GetGoodFileName(string sFilename, string &sDestName, string &sDestPath)
{
	string sDestPathAndFileNoExt = "";			

	//Normalize filename (remove any extra slashes) keeping initial double slash (for network names)
	if (sFilename.find_first_of('\\\\', 0) == 0)
	{
		StringUtils::replace_all(sFilename, "\\\\", "\\");
		sFilename = "\\" + sFilename;
	}
	else
	{
		StringUtils::replace_all(sFilename, "\\\\", "\\");
	}

	//Put all directories into a list
	vector<string> vDirs;
	long lStartPos = 0, lSlashPos = 0;
	long lDirCount = 0;
	do
	{
		lSlashPos = sFilename.find_first_of('\\', lStartPos);
		if (lSlashPos != sFilename.npos)
		{
			//Insert the directory
			vDirs.push_back(sFilename.substr(lStartPos, lSlashPos - lStartPos));
		}
		else
		{
			//Insert the file name
			vDirs.push_back(sFilename.substr(lStartPos));
		}
		lDirCount++;
		lStartPos = lSlashPos + 1;
	}while (lSlashPos != sFilename.npos);

	//Go through each to make sure they are not files.
	vector<string>::iterator iterDirs = vDirs.begin();		
	long lCount = 0;
	while (iterDirs != vDirs.end())
	{
		string sDir = (*iterDirs);
		string sFile = sDir, sExt = "";				

		lCount++;
		if (lCount == lDirCount)
		{
			//This is the file. Seperate it out by extension
			long lDotPos = sDir.find_last_of('.');	
			if (lDotPos != sDir.npos)
			{
				sFile = sDir.substr(0, sDir.find_last_of('.'));
				sExt = sDir.substr(sDir.find_last_of('.'));
			}
		}
		else
		{
			//This is a directory. Make sure there are no spaces or dots at the end of the name
			StringUtils::trim(sFile, " \t\r.", false);
		}
		//Add it to the final name
		string sDestToCheck;
		if (lCount == 1)			
			sDestToCheck = sFile;
		else
			sDestToCheck = sDestPathAndFileNoExt + "\\" + sFile;
		string sCopy = GetCopyNumber(sDestToCheck, sExt);
		sDestName = sFile + sCopy + sExt;
		sDestPath = sDestPathAndFileNoExt;
		if (lCount == 1)			
			sDestPathAndFileNoExt = sDestName;
		else
			sDestPathAndFileNoExt += "\\" + sDestName;
		++iterDirs;
	}

	return sDestPathAndFileNoExt;
}

string VerifyPar::GetCopyNumber(string sDestPathAndFileNoExt, string sExt)
{
	int iCopy = 1;
	char szCopy[64];
	string sCopy = "";
	while (_access((sDestPathAndFileNoExt + sCopy + sExt).c_str(), 0) == 0)
	{	
		//If this is a file, then change the name of the file we extract to
		struct _stat buf;
		if ( _stat((sDestPathAndFileNoExt + sCopy + sExt).c_str(), &buf) == 0 )
		{
			if (buf.st_mode & _S_IFREG)
			{
				sCopy = " (";
				_itoa_s(iCopy, szCopy, 64, 10);
				sCopy += szCopy;
				sCopy += ")";
				iCopy++;
			}
			else
				break;
		}
		else
			break;	//Not a file
	}
	return sCopy;
}

bool VerifyPar::SkipVerify()
{
	m_bSkipVerify = true;
	if (m_Repairer != NULL)
		m_Repairer->m_bSkipVerify = true;
	return true;
}

bool VerifyPar::SetContainsRarFiles(vector<string> vFiles)
{
	bool bRet = false;
	vector<string>::iterator fi = vFiles.begin();		
	while (fi != vFiles.end())
	{
		string sExt = FileUtils::GetFileExtension(*fi);
		CString cstrExt = sExt.c_str();
		cstrExt.MakeUpper();
		if (cstrExt == "RAR")
			bRet = true;
		++fi;
	}
	return bRet;
}