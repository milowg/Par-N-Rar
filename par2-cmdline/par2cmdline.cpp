//  This file is part of par2cmdline (a PAR 2.0 compatible file verification and
//  repair tool). See http://parchive.sourceforge.net for details of PAR 2.0.
//
//  Copyright (c) 2003 Peter Brian Clements
//
//  par2cmdline is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  par2cmdline is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "par2cmdline.h"
#include "c:\temp\unrar\dll.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

void banner(void)
{
	cout << "Par-N-Rar v1.00" << endl << endl

		<< "Par-N-Rar comes with ABSOLUTELY NO WARRANTY." << endl
		<< "This is free software, and you are welcome to redistribute it and/or modify" << endl
		<< "it under the terms of the GNU General Public License as published by the" << endl
		<< "Free Software Foundation; either version 2 of the License, or (at your" << endl
		<< "option) any later version. See COPYING for details." << endl
		<< endl
		<< "Usage: ParNRar.exe <directory to monitor>"
		<< endl;
}

//gm
long RARExtract(string sRARArchive, string sDestPath) //, string sPassword) As Integer
/*
' Description:-
' Exrtact file(s) from RAR archive.

' Parameters:-
' sRARArchive   = RAR Archive filename
' sDestPath     = Destination path for extracted file(s)
' sPassword     = Password [OPTIONAL]

' Returns:-
' Integer       = 0  Failed (no files, incorrect PW etc)
'                 -1 Failed to open RAR archive
'                 >0 Number of files extracted
*/
{
	HANDLE hHandle;
	long lStatus;
	RAROpenArchiveData uRAR;
	RARHeaderData uHeader; 
	int iFileCount;

	long lRet =  -1;

	// Open the RAR
	auto_ptr<char> apTemp( new char[sRARArchive.size() + 1] );
	uRAR.ArcName = apTemp.get();
	strcpy(uRAR.ArcName, sRARArchive.c_str());
	uRAR.OpenMode = RAR_OM_EXTRACT;
	hHandle = RAROpenArchive(&uRAR);

	// Failed to open RAR ?

	if( uRAR.OpenResult != 0)
		return lRet;

	// Password ?
	/*
	If sPassword <> "" Then
	RARSetPassword lHandle, sPassword
	End If
	*/

	//' Extract file(s)...

	iFileCount = 0;

	// Is there at lease one archived file to extract ?
	lStatus = RARReadHeader(hHandle, &uHeader);

	while (lStatus == 0)
	{

		// Process (extract) the current file within the archive
		string sDestPathAndFile = sDestPath + "\\" + uHeader.FileName;
		auto_ptr<char> apDestPath( new char[sDestPath.size() + 1] );
		char *szDestPath = apDestPath.get();
		auto_ptr<char> apDestName( new char[sDestPathAndFile.size() + 1] );
		char *szDestName = apDestName.get();

		strcpy(szDestPath, sDestPath.c_str());
		strcpy(szDestName, sDestPathAndFile.c_str());
		if( RARProcessFile(hHandle, RAR_EXTRACT, szDestPath, szDestName) == 0)
			iFileCount++;

		//' Is there another archived file in this RAR ?
		lStatus = RARReadHeader(hHandle, &uHeader);

	}

	// Close the RAR

	RARCloseArchive (hHandle);

	// Return

	lRet = iFileCount;
	return lRet;

}


int main(int argc, char *argv[])
{
#ifdef _MSC_VER
	// Memory leak checking
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | /*_CRTDBG_CHECK_CRT_DF | */_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	if (argc != 2)
	{
		banner();
		return 0;
	}
	string sInitialDir = argv[1];
	if (sInitialDir.substr(sInitialDir.size(), 1) != "\\")
		sInitialDir += "\\";
	string sInitialDirPar = sInitialDir + "*.par*";

	//gm
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(sInitialDirPar.c_str(), &FindFileData);

	//DEST PATH NOT BEING APPENDED
	//long lRes = RARExtract("h:\\dl\\mpov2a.part01.rar", "h:\\dl"); //, string sPassword) As Integer
	//if lres > 1 == success
	//ALWAYS GET THE FIRST FILE!
	//RARExtract("h:\\dl\\mpov2a.part05.rar", "h:\\dl"); //, string sPassword) As Integer
	while (hFind != INVALID_HANDLE_VALUE) 
	{
		vector<string> vFiles;
		string sRarFile = "", sRarPath = "";				

		vFiles.clear();
		string sFile = FindFileData.cFileName;
		string sExtension = sFile.substr(sFile.find_last_of('.')+1);
		transform(sExtension.begin(), sExtension.end(), sExtension.begin(), toupper); 

		bool bParFile = true;
		if (sExtension == "PAR")
		{
			printf("Checking file %s\n", sFile.c_str());

			Par1Repairer *repairer = new Par1Repairer;
			Result  result = repairer->Process(sInitialDir + sFile, true);
			if (result == eSuccess)
			{
				vector<Par1RepairerSourceFile*>::iterator sf = repairer->sourcefiles.begin();
				// For each recoverable source file
				while (sf != repairer->sourcefiles.end())
				{
					// Do we have a source file
					Par1RepairerSourceFile *sourcefile = *sf;
					if (sourcefile)
					{
						if (sourcefile->GetTargetFile()->FileName().find(".rar", 0) == sourcefile->GetTargetFile()->FileName().size()-4)
						{
							//Get the first rar file
							if (sRarFile != "")
							{
								if (sRarFile > sourcefile->GetTargetFile()->FileName())
									sRarFile = sourcefile->GetTargetFile()->FileName();
							}
							else
								sRarFile = sourcefile->GetTargetFile()->FileName();
							sRarPath = sRarFile.substr(0, sRarFile.find_last_of('\\'));
						}

					}

					++sf;
				}

				map<string, DiskFile*>::iterator fi = repairer->diskfilemap.diskfilemap.begin();
				while (fi != repairer->diskfilemap.diskfilemap.end())
				{
					vFiles.push_back( (*fi).first.c_str() );
					++fi;
				}
				delete repairer;
			}
		}
		else if (sExtension == "PAR2")
		{
			printf("Checking file %s\n", sFile.c_str());
			Par2Repairer *repairer = new Par2Repairer;
			Result  result = repairer->Process(sInitialDir + sFile, true);
			if (result == eSuccess)
			{
				vector<Par2RepairerSourceFile*>::iterator sf = repairer->sourcefiles.begin();
				// For each recoverable source file
				while (sf != repairer->sourcefiles.end())
				{
					// Do we have a source file
					Par2RepairerSourceFile *sourcefile = *sf;
					if (sourcefile)
					{

						if (sourcefile->TargetFileName().find(".rar", 0) == sourcefile->TargetFileName().size()-4)
						{
							//Get the first rar file
							if (sRarFile != "")
							{
								if (sRarFile > sourcefile->TargetFileName())
									sRarFile = sourcefile->TargetFileName();
							}
							else
								sRarFile = sourcefile->TargetFileName();
							sRarPath = sRarFile.substr(0, sRarFile.find_last_of('\\'));
						}

					}

					++sf;
				}

				map<string, DiskFile*>::iterator fi = repairer->diskFileMap.diskfilemap.begin();
				while (fi != repairer->diskFileMap.diskfilemap.end())
				{
					vFiles.push_back( (*fi).first.c_str() );
					++fi;
				}
				delete repairer;
			}
		}
		else
			bParFile = false;

		if (bParFile)
		{
			//Extract files. Move to subdirectory if can't extract for some reason (or no rar file found)
			bool bMove = true;
			if (sRarFile != "")
			{
				printf("Extracting %s\n", sRarFile.c_str());
				if (RARExtract(sRarFile, sRarPath) > 0)
				{
					bMove = false;

					//Delete all files if extracted ok
					vector<string>::iterator fi = vFiles.begin();
					while (fi != vFiles.end())
					{
						if (_access((*fi).c_str(), 0) != -1)
						{
							printf("Deleting file %s\n", (*fi).c_str());
							do
							{
								if (DeleteFile((*fi).c_str()) == 0)
								{
									//long l = GetLastError();
									//long g = l;
									Sleep(5000);
								}

							}while(GetLastError() != S_OK);
						}
						++fi;
					}
				}
			}

			if (bMove)
			{
				//Files have no rars in them.. move to seperate dir
				string sDir = sInitialDir;
				sDir += "\\" + sFile;
				sDir += ".CANNOT_UNRAR\\";
				CreateDirectory(sDir.c_str(), NULL);

				vector<string>::iterator fi = vFiles.begin();
				while (fi != vFiles.end())
				{
					if (_access((*fi).c_str(), 0) != -1)
					{
						printf("Moving file %s\n", (*fi).c_str());
						//do
						//{
						string sFileToMove = (*fi).substr((*fi).find_last_of('\\')+1);
						MoveFile((*fi).c_str(), (sDir + sFileToMove).c_str());
						/*{
						long l = GetLastError();
						printf("Error - %d\n", l);
						Sleep(5000);
						}
						*/
						//}while(GetLastError() != S_OK);
					}
					++fi;
				}
			}
		}

		bool bDiffFile = false;
		bool bExit = false;
		while (!bDiffFile)
		{
			if (!FindNextFile(hFind, &FindFileData))
			{
				long l = GetLastError();
				if (l != ERROR_NO_MORE_FILES)
					printf("Error in FindNextFile: %d", l);
				bExit = true;
				break;
			}

			string sFullpath = "h:\\dl\\";
			sFullpath += FindFileData.cFileName;
			if (find(vFiles.begin(), vFiles.end(), sFullpath) == vFiles.end())
				bDiffFile = true;
		}	
		if (bExit)
			break;
	}
	FindClose(hFind);
	printf("No more PAR/PAR2 files found.\nDone!\n");
	scanf("%s");
//When we get here, wait for files to change. If a PAR2 file is added, wait for 1 minute until after the file size stops changing
//http://www.codeguru.com/Cpp/W-P/files/article.php/c4467/
return 0;
}





////////  This file is part of par2cmdline (a PAR 2.0 compatible file verification and
////////  repair tool). See http://parchive.sourceforge.net for details of PAR 2.0.
////////
////////  Copyright (c) 2003 Peter Brian Clements
////////
////////  par2cmdline is free software; you can redistribute it and/or modify
////////  it under the terms of the GNU General Public License as published by
////////  the Free Software Foundation; either version 2 of the License, or
////////  (at your option) any later version.
////////
////////  par2cmdline is distributed in the hope that it will be useful,
////////  but WITHOUT ANY WARRANTY; without even the implied warranty of
////////  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
////////  GNU General Public License for more details.
////////
////////  You should have received a copy of the GNU General Public License
////////  along with this program; if not, write to the Free Software
////////  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//////
//////#include "par2cmdline.h"
//////
//////#ifdef _MSC_VER
//////#ifdef _DEBUG
//////#undef THIS_FILE
//////static char THIS_FILE[]=__FILE__;
//////#define new DEBUG_NEW
//////#endif
//////#endif
//////
//////int main(int argc, char *argv[])
//////{
//////  string version = PACKAGE " version " VERSION;
//////
//////#ifdef _MSC_VER
//////  // Memory leak checking
//////  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | /*_CRTDBG_CHECK_CRT_DF | */_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//////#endif
//////
//////  cout << version << ", Copyright (C) 2003 Peter Brian Clements." << endl
//////       << endl
//////       << "par2cmdline comes with ABSOLUTELY NO WARRANTY." << endl
//////       << endl
//////       << "This is free software, and you are welcome to redistribute it and/or modify" << endl
//////       << "it under the terms of the GNU General Public License as published by the" << endl
//////       << "Free Software Foundation; either version 2 of the License, or (at your" << endl
//////       << "option) any later version. See COPYING for details." << endl
//////       << endl;
//////
//////  // Parse the command line
//////  CommandLine *commandline = new CommandLine;
//////
//////  Result result = eInvalidCommandLineArguments;
//////  
//////  if (!commandline->Parse(argc, argv))
//////  {
//////    CommandLine::usage();
//////  }
//////  else
//////  {
//////    // Which operation was selected
//////    switch (commandline->GetOperation())
//////    {
//////    case CommandLine::opCreate:
//////      {
//////        // Create recovery data
//////
//////        Par2Creator *creator = new Par2Creator;
//////        result = creator->Process(*commandline);
//////        delete creator;
//////      }
//////      break;
//////    case CommandLine::opVerify:
//////      {
//////        // Verify damaged files
//////        switch (commandline->GetVersion())
//////        {
//////        case CommandLine::verPar1:
//////          {
//////            Par1Repairer *repairer = new Par1Repairer;
//////            result = repairer->Process(*commandline, false);
//////            delete repairer;
//////          }
//////          break;
//////        case CommandLine::verPar2:
//////          {
//////            Par2Repairer *repairer = new Par2Repairer;
//////            result = repairer->Process(*commandline, false);
//////            delete repairer;
//////          }
//////          break;
//////        case CommandLine::opNone:
//////          break;
//////        }
//////      }
//////      break;
//////    case CommandLine::opRepair:
//////      {
//////        // Repair damaged files
//////        switch (commandline->GetVersion())
//////        {
//////        case CommandLine::verPar1:
//////          {
//////            Par1Repairer *repairer = new Par1Repairer;
//////            result = repairer->Process(*commandline, true);
//////            delete repairer;
//////          }
//////          break;
//////        case CommandLine::verPar2:
//////          {
//////            Par2Repairer *repairer = new Par2Repairer;
//////            result = repairer->Process(*commandline, true);
//////            delete repairer;
//////          }
//////          break;
//////        case CommandLine::opNone:
//////          break;
//////        }
//////      }
//////      break;
//////    case CommandLine::opNone:
//////      break;
//////    }
//////  }
//////
//////  delete commandline;
//////
//////  return result;
//////}
