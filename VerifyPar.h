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

#pragma once
#include <string>
#include "ParSettings.h"
#include "IParRepairer.h"

using namespace std;
enum VERIFYSTATUS { STATUS_NONE, STATUS_VERIFYING, STATUS_EXTRACTING, STATUS_DELETING };

class VerifyPar
{
public:
	VerifyPar(bool bVerifyOnly, string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir);
	VerifyPar(string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir, bool bRemoveFromList, vector<string> vOtherFilesToDelete);
	VerifyPar(string sParfileName, string sMonitorDir);
	static void StartVerify(bool bVerifyOnly, VerifyPar **ppVerifyPar, string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir);
	static void StartDelete(VerifyPar **ppVerifyPar, string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir, bool bRemoveFromList);
	static void StartDelete(VerifyPar **ppVerifyPar, string sParfileName, string sParfilePath, string sPathOffMonitorDir, string sMonitorDir, bool bRemoveFromList, vector<string> vOtherFilesToDelete);
	static void StartDeleteFiles(string sParfileName);
	virtual ~VerifyPar();
	bool SkipVerify();

	void MainDeleteFiles(vector<string> vDeleteFiles);
	void MainDelete();
	void Initialize();
	bool m_bVerifyOnly;				//True if we are only verifying	
	VERIFYSTATUS m_Status;

protected:
	bool m_bSkipVerify;				//True if we need to stop this verification
	string m_sParfileName;			//The name of the file	(ex: File.par)
	string m_sParfilePath;			//The path to the file, including ending slash (ex: c:\monitor\subdir)
	string m_sPathOffMonitorDir;	//The path off of the monitor dir where the file was found (ie. subdir)
	string m_sMonitorDir;			//The monitor path (ex. c:\monitor)
	IParRepairer *m_Repairer;
	string m_sFileNoExt;
	string m_sExtension;
	bool m_bRemoveFromList;			//Remove from list when done deleting	
	bool m_bVerified;				//True if verification passed
	vector<string> m_vOtherFilesToDelete;
	
	void MainVerify();		
	int RARExtract(vector<string> *pvRarFiles, string sRARArchive, string sDestPath, string sParName, string sExtractPath);
	static unsigned int __stdcall StartThreadVerify( void* pParam );
	static unsigned int __stdcall StartThreadDelete( void* pParam );
	void SaveParSettings(ParSettings *pSettings);
	void LoadParSettings(ParSettings *pSettings);
	void SendFileError(CException *err, string sAction, ParSettings *pSettings);
	vector<string> ExtractFiles(string sRarFile, string sRarPath, string sFileNoExt, string sExtractPath, long &lRet);
	int RARFileCount(string sRARArchive, vector<string> *pvRarFiles);
	int RARFileCount(string sRARArchive);
	bool MoveFiles(vector<string> vFiles, ParSettings &pSettings, string sNonRarDir);
	string GetDestinationForMove(string sSrcPath, string sDestDir);
	bool MyDeleteFile(const char *szFile);
	string GetRarFileInSet(vector<string> &vFiles);
	void HandleInvalidRarSet(vector<string> &vVerifiedFiles, ParSettings &fileSettings, bool &bDeleted);
	void LoadRepairer();
	void DeleteRepairer();
	string GetGoodFileName(string sFilename, string &sDestName, string &sDestPath);
	string GetCopyNumber(string sDestPathAndFileNoExt, string sExt);
	bool DeleteDirIfEmpty(string sDir);
	bool SetContainsRarFiles(vector<string> vFiles);
};


