//	11/1/05 gmilow - Modified 

#include "stdafx.h"
#include "rar.hpp"

bool FileCreate(vector<string> *pvRarFiles, string sParfileName, RAROptions *Cmd,File *NewFile,char *Name,wchar *NameW,
				OVERWRITE_MODE Mode,bool *UserReject,Int64 FileSize,
				uint FileTime)
{
	if (UserReject!=NULL)
		*UserReject=false;
	while (FileExist(Name,NameW))
	{
		if (Mode==OVERWRITE_NONE)
		{
			if (UserReject!=NULL)
				*UserReject=true;
			return(false);
		}
#ifdef SILENT
		Mode=OVERWRITE_ALL;
#endif
		if (Cmd->AllYes || Mode==OVERWRITE_ALL)
			break;
		if (Mode==OVERWRITE_ASK)
		{
			eprintf(sParfileName, St(MFileExists),Name);
			int Choice=Ask(pvRarFiles, sParfileName,St(MYesNoAllRenQ));
			if (Choice==1)
				break;
			if (Choice==2)
			{
				if (UserReject!=NULL)
					*UserReject=true;
				return(false);
			}
			if (Choice==3)
			{
				Cmd->Overwrite=OVERWRITE_ALL;
				break;
			}
			if (Choice==4)
			{
				if (UserReject!=NULL)
					*UserReject=true;
				Cmd->Overwrite=OVERWRITE_NONE;
				return(false);
			}
			if (Choice==5)
			{
				mprintf(sParfileName, St(MAskNewName));

				char NewName[NM];
#ifdef  _WIN_32
				File SrcFile(pvRarFiles, sParfileName);
				SrcFile.SetHandleType(FILE_HANDLESTD);
				int Size=SrcFile.Read(NewName,NM);
				NewName[Size]=0;
				OemToChar(NewName,NewName);
#else
				fgets(NewName,sizeof(NewName),stdin);
#endif
				RemoveLF(NewName);
				if (PointToName(NewName)==NewName)
					strcpy(PointToName(Name),NewName);
				else
					strcpy(Name,NewName);
				if (NameW!=NULL)
					*NameW=0;
				continue;
			}
			if (Choice==6)
				ErrHandler.Exit(USER_BREAK);
		}
	}
	if (NewFile!=NULL && NewFile->Create(Name,NameW))
		return(true);
	PrepareToDelete(Name,NameW);
	CreatePath(sParfileName, Name,NameW,true);
	return(NewFile!=NULL ? NewFile->Create(Name,NameW):DelFile(Name,NameW));
}
