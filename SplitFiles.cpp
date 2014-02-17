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
#include "SplitFiles.h"
#include "PnrMessage.h"
#include <iomanip>
#include <io.h>

#define BUFFER_SIZE 1024*1024

//From unrar/filefn.cpp
extern void CreatePath(string sParfileName, const char *Path, const WCHAR *PathW,bool SkipLastName);

vector<string> SplitFiles::JoinFiles(string sParfileName, string sFirstJoinFile, string sExtractPath)
{
	vector<string> vSourceFiles, vEmpty;

	if (sExtractPath == "")
		sExtractPath = "\\";

	//Double check file ends with .001, and is not just ".001"
	if ((sFirstJoinFile.length() < 5) || (sFirstJoinFile.substr(sFirstJoinFile.length() - 4) != ".001"))
		return vEmpty;
	else if (sFirstJoinFile.size() == 4)
		return vEmpty;
	
	//Get a list of all files in this set
	string sBaseName = sFirstJoinFile.substr(0, sFirstJoinFile.length()-3);
	int iPartNum = 2;
	ostringstream strm;
	vSourceFiles.push_back(sFirstJoinFile);
	do
	{
		strm.str(""); strm << setfill('0') << setw(3) << iPartNum;
		if ( _access((sBaseName + strm.str()).c_str(), 0) == -1 )
			break;
		else
			vSourceFiles.push_back(sBaseName + strm.str());
		iPartNum++;
	}while(iPartNum < 1000);

	FILE *fCurFile, *fJoinedFile;
	string sCurFile = sFirstJoinFile;
	auto_ptr<char> apBuffer( new char[BUFFER_SIZE] );
	char *buffer = apBuffer.get();	

	//Get the joined file name by stripping off the path of the .001 file and putting it in the done directory
	string sJoinedFile;
	if (sFirstJoinFile.find_last_of('\\') != sFirstJoinFile.npos)
		sJoinedFile = sFirstJoinFile.substr(sFirstJoinFile.find_last_of('\\') + 1);
	else		
		sJoinedFile = sFirstJoinFile;

	if (sExtractPath.substr(sExtractPath.length() - 1, 1) != "\\")
		sJoinedFile = sExtractPath + "\\" + sJoinedFile.substr(0, sJoinedFile.size() - 4);
	else
		sJoinedFile = sExtractPath + sJoinedFile.substr(0, sJoinedFile.size() - 4);

	//Make sure directory exists
	CreatePath(sParfileName, sExtractPath.c_str(), NULL, false);

	//Inform the user
	CPnrMessage::SendParAddDetails(sParfileName, "Assuming .001 means split file set.", true);
	CPnrMessage::SendParAddDetails(sParfileName, "Creating joined file " + sJoinedFile, true);
	strm.str(""); strm << "Joining";
	CPnrMessage::SendParStatus(sParfileName, strm.str().c_str());

	//Open the destination file
	fJoinedFile = fopen(sJoinedFile.c_str(), "wb");
	if (fJoinedFile == NULL)
	{
		CPnrMessage::SendParAddDetails(sParfileName, "Failed to created joined file!", true);
		return vEmpty;
	}

	//Join the files one by one
	CPnrMessage::SendParProgress(sParfileName, "0");
	vector<string>::iterator fi = vSourceFiles.begin();
	int iFileNum = 1;
	while (fi != vSourceFiles.end())
	{
		fCurFile = fopen((*fi).c_str(), "rb");
		if (fCurFile == NULL)
			break;
			CPnrMessage::SendParAddDetails(sParfileName, "Joining " + (*fi), true);

		DWORD dwRead;
		while (!feof(fCurFile) && (dwRead = fread(buffer, sizeof(char), BUFFER_SIZE, fCurFile))) 
		{
			if (fwrite(buffer, sizeof(char), dwRead, fJoinedFile) != dwRead) 
			{

				CPnrMessage::SendParAddDetails(sParfileName, "Failed to write to joined file!", true);
				fclose(fCurFile);
				fclose(fJoinedFile);
				return vEmpty;
			}
		}

		strm.str(""); strm << ((float)iFileNum / vSourceFiles.size() * 100);
		Sleep(1000);
		CPnrMessage::SendParProgress(sParfileName, strm.str().c_str());
		fclose(fCurFile);			
		++fi;
		++iFileNum;
	}

	fclose(fJoinedFile);
	return vSourceFiles;
}