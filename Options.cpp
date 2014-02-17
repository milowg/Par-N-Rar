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
#include "options.h"
#include "StringUtils.h"

Options::Options(void)
{
	m_bGoOnStart = false;
	CoInitialize(NULL);
	if (CoCreateInstance( MSXML::CLSID_DOMDocument, 0, CLSCTX_INPROC_SERVER, MSXML::IID_IXMLDOMDocument, (void**)&m_domSettings ) != S_OK)
	{
		AfxMessageBox("Error: Failed to create DOMDocument object", MB_OK);
		return;
	}	
	m_domSettings->load("settings.dat");
	if (m_domSettings->xml == _bstr_t(""))
	{
		m_domSettings->loadXML("<ParNRar/>");

		//Check for registry settings and convert
		HKEY hkey;
		if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\MilowSoft\\Temp\\ParNRar", 0, KEY_READ, &hkey) == ERROR_SUCCESS)
		{
			MigrateSettings(hkey);
			RegCloseKey(hkey);
			SaveSettings();
		}
	}
}

Options::~Options(void)
{
}

void Options::SaveSettings()
{
	m_domSettings->save("settings.dat");
}

void Options::MigrateSettings(HKEY hKey)
{
    TCHAR    achClass[MAX_PATH] = TEXT("");
    DWORD    cchClassName = MAX_PATH;
    DWORD    cSubKeys=0;
    DWORD    cbMaxSubKey;
    DWORD    cchMaxClass;
    DWORD    cValues;
    DWORD    cchMaxValue;
    DWORD    cbMaxValueData;
    DWORD    cbSecurityDescriptor;
    FILETIME ftLastWriteTime; 
    DWORD i, retCode;  
    TCHAR  achValue[256]; 
    DWORD cchValue = 256; 
 
    retCode = RegQueryInfoKey(hKey, achClass, &cchClassName, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass, &cValues, &cchMaxValue, &cbMaxValueData, &cbSecurityDescriptor, &ftLastWriteTime);
    if (cValues) 
    {
        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
        { 
            cchValue = 256; 
            achValue[0] = '\0'; 
			DWORD dwType;
			char szTemp[1024];
			DWORD dwDataSize = 1024;
            retCode = RegEnumValue(hKey, i,  achValue,  &cchValue,  NULL, &dwType, (BYTE *)szTemp, &dwDataSize); 
            if (retCode == ERROR_SUCCESS ) 
            { 
			switch (dwType)
			{
				case 1:
					//REG_SZ
					SetStringOption(achValue, szTemp);
					break;
				case 4:
					//REG_DWORD
					DWORD dw;
					memcpy(&dw, szTemp, dwDataSize);
					SetDwordOption(achValue, dw);
					break;
				}
            } 
        }
    }
}

void Options::SetStringOption(string sOption, string sValue)
{
	string sXPath = "/ParNRar/";
	sXPath += sOption;

	MSXML::IXMLDOMElementPtr eOption = m_domSettings->selectSingleNode(sXPath.c_str());
	if (eOption == NULL)
	{
		eOption = m_domSettings->createElement(sOption.c_str());
		m_domSettings->documentElement->appendChild(eOption);
	}
	eOption->text = sValue.c_str();
	SaveSettings();
}

void Options::SetBoolOption(char *szOption, bool bValue)
{
	if (bValue)
		SetStringOption(szOption, "true");
	else
		SetStringOption(szOption, "false");
}

bool Options::GetBoolOption(char *szOption, bool bDefault)
{
	string sOption = GetStringOption(szOption);
	bool bOption = bDefault;

	if (bDefault == true)
	{
		if (sOption == "false")
			bOption = false;
	}
	else
	{
		if (sOption == "true")
			bOption = true;
	}
	return bOption;
}

void Options::SetDwordOption(char *szOption, DWORD dwValue)
{
	char szTemp[64];	
	SetStringOption(szOption, ltoa(dwValue, szTemp, 10));
}

DWORD Options::GetDwordOption(char *szOption, DWORD dwDefault)
{
	string sOption = GetStringOption(szOption);
	DWORD dwOption = dwDefault;
	
	if (sOption != "")
	{
		dwOption = atol(sOption.c_str());
	}
	return dwOption;
}

void Options::SetMinimizeOnStart(bool bMinimizeOnStart)
{
	m_bMinimizeOnStart = bMinimizeOnStart;
}

void Options::SetGoOnStart(bool bGoOnStart)
{
	m_bGoOnStart = bGoOnStart;
}

bool Options::GetMinimizeOnStart()
{
	return m_bMinimizeOnStart;
}

bool Options::GetGoOnStart()
{
	bool bGoOnStart = m_bGoOnStart;
	m_bGoOnStart = false; //We only want to return true once
	return bGoOnStart;
}

string Options::GetStringOption(char *szOption, char *szDefault)
{
	string sXPath = "/ParNRar/";
	sXPath += szOption;
	MSXML::IXMLDOMElementPtr eOption = m_domSettings->selectSingleNode(sXPath.c_str());
	if (eOption == NULL)
		return "";
	else
		return static_cast<char *>(eOption->text);
}

//Options
bool Options::GetUseRecycleBin(){ return GetBoolOption("UseRecycleBin", true); }
void Options::SetUseRecycleBin(bool bUseRecycleBin){ SetBoolOption("UseRecycleBin", bUseRecycleBin); }

bool Options::GetDeleteWhenDone(){ return GetBoolOption("DeleteWhenDone", true); }
void Options::SetDeleteWhenDone(bool bDeleteWhenDone){ SetBoolOption("DeleteWhenDone", bDeleteWhenDone); }

bool Options::GetMinSysTray(){ return GetBoolOption("MinSysTray", true); }
void Options::SetMinSysTray(bool bMinSysTray){ SetBoolOption("MinSysTray", bMinSysTray); }

bool Options::GetRecurseMonitorDir(){ return GetBoolOption("RecurseMonitorDir", true); }
void Options::SetRecurseMonitorDir(bool bRecurseMonitorDir){ SetBoolOption("RecurseMonitorDir", bRecurseMonitorDir); }

bool Options::GetMoveNonRar(){ return GetBoolOption("MoveNonRar", true); }
void Options::SetMoveNonRar(bool bMoveNonRar){ SetBoolOption("MoveNonRar", bMoveNonRar); }

string Options::GetExtractDir(){ return GetStringOption("ExtractDir"); }
void Options::SetExtractDir(string sExtractDir){ SetStringOption("ExtractDir", sExtractDir.c_str()); }

string Options::GetNonRarDir(){ return GetStringOption("NonRarDir"); }
void Options::SetNonRarDir(string sNonRarDir){ SetStringOption("NonRarDir", sNonRarDir.c_str()); }

string Options::GetRestartDelay(){ return GetStringOption("RestartDelay", "300"); }
void Options::SetRestartDelay(string sRestartDelay){ SetStringOption("RestartDelay", sRestartDelay.c_str()); }

string Options::GetPriority(){ return GetStringOption("Priority", "Normal"); }
void Options::SetPriority(string sPriority){ SetStringOption("Priority", sPriority.c_str()); }

string Options::GetDoneScan(){ return GetStringOption("DoneScan", "Restart"); }
void Options::SetDoneScan(string sDoneScan){ SetStringOption("DoneScan", sDoneScan.c_str()); }

bool Options::GetDeleteEmptyDirs(){ return GetBoolOption("DeleteDirsWhenDone", true); }
void Options::SetDeleteEmptyDirs(bool bDeleteEmptyDirs){ SetBoolOption("DeleteDirsWhenDone", bDeleteEmptyDirs); }

bool Options::GetEmptyDirsUseRecycleBin(){ return GetBoolOption("EmptyDirsUseRecycleBin", true); }
void Options::SetEmptyDirsUseRecycleBin(bool bEmptyDirsUseRecycleBin){ SetBoolOption("EmptyDirsUseRecycleBin", bEmptyDirsUseRecycleBin); }

bool Options::GetKeepWorkItemInFocus(){ return GetBoolOption("KeepWorkItemInFocus", true); }
void Options::SetKeepWorkItemInFocus(bool bKeepWorkItemInFocus){ SetBoolOption("KeepWorkItemInFocus", bKeepWorkItemInFocus); }

bool Options::GetCloseToTray(){ return GetBoolOption("CloseToTray", true); }
void Options::SetCloseToTray(bool bCloseToTray){ SetBoolOption("CloseToTray", bCloseToTray); }


//Settings
string Options::GetCheckForNewVersion(){ return GetStringOption("CheckForNewVersion", "Weekly"); }
void Options::SetCheckForNewVersion(string sCheckForNewVersion){ SetStringOption("CheckForNewVersion", sCheckForNewVersion.c_str()); }

bool Options::GetAskVerifyOnly(){ return GetBoolOption("AskVerifyOnly", true); }
void Options::SetAskVerifyOnly(bool bAskVerifyOnly){ SetBoolOption("AskVerifyOnly", bAskVerifyOnly); }

string Options::GetMonitorDir(){ return GetStringOption("MonitorDir"); }
void Options::SetMonitorDir(string sMonitorDir){ SetStringOption("MonitorDir", sMonitorDir.c_str()); }

string Options::ConvertDirectory(string sDirName, string sParname, string sPathOffMonitorDir, string sMonitorDir)
{
	StringUtils::replace_all(sDirName, "%s", sParname);
	StringUtils::replace_all(sDirName, "%d", sPathOffMonitorDir);
	StringUtils::replace_all(sDirName, "%m", sMonitorDir);
	return sDirName;
}

void Options::ClearLastMonitorDirs(){
	for (int i=0; i<25; i++)
	{
		char sz[50];
		string sReg = "LastMonitorDir";
		sReg += _itoa(i, sz, 10);
		SetStringOption(sReg.c_str(), "");
	}
}

vector<string> Options::GetLastMonitorDirs(){
	vector<string> vLastMonitorDirs;
	for (int i=0; i<25; i++)
	{
		char sz[50];
		string sReg = "LastMonitorDir";
		sReg += _itoa(i, sz, 10);
		string sLastMonitorDir = GetStringOption((char *)sReg.c_str());
		if (sLastMonitorDir == "")
			break;
		vLastMonitorDirs.push_back(sLastMonitorDir);
	}
	return vLastMonitorDirs; 
}

void Options::SetLastMonitorDirs(vector<string> &vLastMonitorDirs){ 

	vector<string>::iterator vi = vLastMonitorDirs.begin();
	int i = 0;
	while (vi != vLastMonitorDirs.end() || i<25)
	{
		char sz[50];
		string sReg = "LastMonitorDir";
		sReg += _itoa(i, sz, 10);

		if (vi != vLastMonitorDirs.end())
		{
			SetStringOption(sReg.c_str(), (*vi).c_str());
			++vi;
		}
		else
			SetStringOption(sReg.c_str(), "");
		
		++i;		
	}
}
