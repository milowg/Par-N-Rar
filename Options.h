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
#import "msxml.dll" named_guids 

class Options
{
protected:
	MSXML::IXMLDOMDocumentPtr m_domSettings;
	
	void MigrateSettings(HKEY hKey);
	bool m_bGoOnStart;
	bool m_bMinimizeOnStart;
	void Options::SaveSettings();

public:
	Options(void);
	~Options(void);

	//Global Flags
	bool GetGoOnStart();
	bool GetMinimizeOnStart();
	void SetGoOnStart(bool bGoOnStart);
	void SetMinimizeOnStart(bool bMinimizeOnStart);

	//Options
	bool GetUseRecycleBin();
	void SetUseRecycleBin(bool bUseRecycleBin);
	
	bool GetMinSysTray();
	void SetMinSysTray(bool bMinSysTray);

	bool GetDeleteWhenDone();
	void SetDeleteWhenDone(bool bDeleteWhenDone);

	bool GetRecurseMonitorDir();
	void SetRecurseMonitorDir(bool bRecurseMonitorDir);

	bool GetMoveNonRar();
	void SetMoveNonRar(bool bMoveNonRar);

	string GetExtractDir();
	void SetExtractDir(string sExtractDir);

	string GetNonRarDir();
	void SetNonRarDir(string sNonRarDir);

	string GetRestartDelay();
	void SetRestartDelay(string sRestartDelay);

	string GetMonitorDir();
	void SetMonitorDir(string sMonitorDir);

	string GetPriority();
	void SetPriority(string sPriority);

	string ConvertDirectory(string sDirName, string sParname, string sPathOffMonitorDir, string sMonitorDir);

	string GetDoneScan();
	void SetDoneScan(string sDoneScan);

	bool GetDeleteEmptyDirs();
	void SetDeleteEmptyDirs(bool bDeleteEmptyDirs);

	bool GetEmptyDirsUseRecycleBin();
	void SetEmptyDirsUseRecycleBin(bool bEmptyDirsUseRecycleBin);

	bool GetKeepWorkItemInFocus();
	void SetKeepWorkItemInFocus(bool bKeepWorkItemInFocus);

	bool GetCloseToTray();
	void SetCloseToTray(bool bCloseToTray);


	//Settings
	vector<string> GetLastMonitorDirs();
	void SetLastMonitorDirs(vector<string> &vLastMonitorDirs);
	void ClearLastMonitorDirs();

	bool GetAskVerifyOnly();
	void SetAskVerifyOnly(bool bAskVerifyOnly);

	string GetCheckForNewVersion();
	void SetCheckForNewVersion(string sCheckForNewVersion);

	bool GetBoolOption(char *szOption, bool bDefault);
	void SetBoolOption(char *szOption, bool bValue);
	DWORD GetDwordOption(char *szOption, DWORD dwDefault);
	void SetDwordOption(char *szOption, DWORD dwValue);
	string GetStringOption(char *szOption, char *szDefault = NULL);
	void SetStringOption(string sOption, string sValue);

};
