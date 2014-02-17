//	11/1/05 gmilow - Modified 
#include "stdafx.h"
#include "rar.hpp"
#include "dll.hpp"

static int RarErrorToDll(int ErrCode);

struct DataSet
{
	CommandData Cmd;
	CmdExtract Extract;
	Archive Arc;
	int OpenMode;
	int HeaderSize;

	DataSet(vector<string> *pvRarFiles, string sParfileName): Cmd(pvRarFiles, sParfileName), Arc(pvRarFiles, sParfileName, &Cmd), Extract(pvRarFiles, sParfileName){};
};


HANDLE PASCAL RAROpenArchive(vector<string> *pvRarFiles, string sParfileName, struct RAROpenArchiveData *r)
{
	RAROpenArchiveDataEx rx;
	memset(&rx,0,sizeof(rx));
	rx.ArcName=r->ArcName;
	rx.OpenMode=r->OpenMode;
	rx.CmtBuf=r->CmtBuf;
	rx.CmtBufSize=r->CmtBufSize;
	HANDLE hArc=RAROpenArchiveEx(pvRarFiles, sParfileName, &rx);
	r->OpenResult=rx.OpenResult;
	r->CmtSize=rx.CmtSize;
	r->CmtState=rx.CmtState;
	return(hArc);
}


HANDLE PASCAL RAROpenArchiveEx(vector<string> *pvRarFiles, string sParfileName, struct RAROpenArchiveDataEx *r)
{
	try
	{
		r->OpenResult=0;
		DataSet *Data=new DataSet(pvRarFiles, sParfileName);
		Data->OpenMode=r->OpenMode;
		Data->Cmd.FileArgs->AddString("*");

		char an[NM];
		if (r->ArcName==NULL && r->ArcNameW!=NULL)
		{
			WideToChar(r->ArcNameW,an,NM);
			r->ArcName=an;
		}

		Data->Cmd.AddArcName(r->ArcName,r->ArcNameW);
		Data->Cmd.Overwrite=OVERWRITE_ALL;
		Data->Cmd.VersionControl=1;
		if (!Data->Arc.Open(r->ArcName,r->ArcNameW))
		{
			delete Data;
			r->OpenResult=ERAR_EOPEN;
			return(NULL);
		}
		if (!Data->Arc.IsArchive(false))
		{
			delete Data;
			r->OpenResult=ERAR_BAD_ARCHIVE;
			return(NULL);
		}
		r->Flags=Data->Arc.NewMhd.Flags;
		Array<byte> CmtData(sParfileName);
		//gmilow - ignore comments
		r->CmtState=r->CmtSize=0;
		if (Data->Arc.Signed)
			r->Flags|=0x20;
		Data->Extract.ExtractArchiveInit(&Data->Cmd,Data->Arc);
		return((HANDLE)Data);
	}
	catch (int ErrCode)
	{
		r->OpenResult=RarErrorToDll(ErrCode);
		return(NULL);
	}
}


int PASCAL RARCloseArchive(HANDLE hArcData)
{
	DataSet *Data=(DataSet *)hArcData;
	bool Success=Data==NULL ? false:Data->Arc.Close();
	delete Data;
	return(Success ? 0:ERAR_ECLOSE);
}


int PASCAL RARReadHeader(HANDLE hArcData,struct RARHeaderData *D, string sParfileName)
{
	DataSet *Data=(DataSet *)hArcData;
	try
	{
		if ((Data->HeaderSize=Data->Arc.SearchBlock(FILE_HEAD))<=0)
		{
			if (Data->Arc.Volume && Data->Arc.GetHeaderType()==ENDARC_HEAD &&
				(Data->Arc.EndArcHead.Flags & EARC_NEXT_VOLUME))
				if (MergeArchive(Data->Arc,NULL,false,'L'))
				{
					Data->Arc.Seek(Data->Arc.CurBlockPos,SEEK_SET);
					return(RARReadHeader(hArcData,D,sParfileName));
				}
				else
					return(ERAR_EOPEN);
			return(Data->Arc.BrokenFileHeader ? ERAR_BAD_DATA:ERAR_END_ARCHIVE);
		}
		if (Data->OpenMode==RAR_OM_LIST && (Data->Arc.NewLhd.Flags & LHD_SPLIT_BEFORE))
		{
			if (RARProcessFile(hArcData,RAR_SKIP,NULL,NULL,sParfileName)==0)
				return(RARReadHeader(hArcData,D,sParfileName));
		}
		strncpy(D->ArcName,Data->Arc.FileName,sizeof(D->ArcName));
		strncpy(D->FileName,Data->Arc.NewLhd.FileName,sizeof(D->FileName));
		D->Flags=Data->Arc.NewLhd.Flags;
		D->PackSize=Data->Arc.NewLhd.PackSize;
		D->UnpSize=Data->Arc.NewLhd.UnpSize;
		D->HostOS=Data->Arc.NewLhd.HostOS;
		D->FileCRC=Data->Arc.NewLhd.FileCRC;
		D->FileTime=Data->Arc.NewLhd.FileTime;
		D->UnpVer=Data->Arc.NewLhd.UnpVer;
		D->Method=Data->Arc.NewLhd.Method;
		D->FileAttr=Data->Arc.NewLhd.FileAttr;
		D->CmtSize=0;
		D->CmtState=0;
	}
	catch (int ErrCode)
	{
		return(RarErrorToDll(ErrCode));
	}
	return(0);
}


int PASCAL RARReadHeaderEx(HANDLE hArcData,struct RARHeaderDataEx *D, string sParfileName)
{
	DataSet *Data=(DataSet *)hArcData;
	try
	{
		if ((Data->HeaderSize=Data->Arc.SearchBlock(FILE_HEAD))<=0)
		{
			if (Data->Arc.Volume && Data->Arc.GetHeaderType()==ENDARC_HEAD &&
				(Data->Arc.EndArcHead.Flags & EARC_NEXT_VOLUME))
				if (MergeArchive(Data->Arc,NULL,false,'L'))
				{
					Data->Arc.Seek(Data->Arc.CurBlockPos,SEEK_SET);
					return(RARReadHeaderEx(hArcData,D,sParfileName));
				}
				else
					return(ERAR_EOPEN);
			return(Data->Arc.BrokenFileHeader ? ERAR_BAD_DATA:ERAR_END_ARCHIVE);
		}
		if (Data->OpenMode==RAR_OM_LIST && (Data->Arc.NewLhd.Flags & LHD_SPLIT_BEFORE))
		{
			if (RARProcessFile(hArcData,RAR_SKIP,NULL,NULL,sParfileName)==0)
				return(RARReadHeaderEx(hArcData,D,sParfileName));
		}
		strncpy(D->ArcName,Data->Arc.FileName,sizeof(D->ArcName));
		if (*Data->Arc.FileNameW)
			strncpyw(D->ArcNameW,Data->Arc.FileNameW,sizeof(D->ArcNameW));
		else
			CharToWide(Data->Arc.FileName,D->ArcNameW);
		strncpy(D->FileName,Data->Arc.NewLhd.FileName,sizeof(D->FileName));
		if (*Data->Arc.NewLhd.FileNameW)
			strncpyw(D->FileNameW,Data->Arc.NewLhd.FileNameW,sizeof(D->FileNameW));
		else
		{
#ifdef _WIN_32
			char AnsiName[NM];
			OemToChar(Data->Arc.NewLhd.FileName,AnsiName);
			CharToWide(AnsiName,D->FileNameW);
#else
			CharToWide(Data->Arc.NewLhd.FileName,D->FileNameW);
#endif
		}
		D->Flags=Data->Arc.NewLhd.Flags;
		D->PackSize=Data->Arc.NewLhd.PackSize;
		D->PackSizeHigh=Data->Arc.NewLhd.HighPackSize;
		D->UnpSize=Data->Arc.NewLhd.UnpSize;
		D->UnpSizeHigh=Data->Arc.NewLhd.HighUnpSize;
		D->HostOS=Data->Arc.NewLhd.HostOS;
		D->FileCRC=Data->Arc.NewLhd.FileCRC;
		D->FileTime=Data->Arc.NewLhd.FileTime;
		D->UnpVer=Data->Arc.NewLhd.UnpVer;
		D->Method=Data->Arc.NewLhd.Method;
		D->FileAttr=Data->Arc.NewLhd.FileAttr;
		D->CmtSize=0;
		D->CmtState=0;
	}
	catch (int ErrCode)
	{
		return(RarErrorToDll(ErrCode));
	}
	return(0);
}


int PASCAL ProcessFile(HANDLE hArcData,int Operation,char *DestPath,char *DestName,wchar *DestPathW,wchar *DestNameW, string sParfileName)
{
	DataSet *Data=(DataSet *)hArcData;
	try
	{
		Data->Arc.m_sParfileName = sParfileName;

		if (Data->OpenMode==RAR_OM_LIST || Operation==RAR_SKIP && !Data->Arc.Solid)
		{
			if (/*Data->OpenMode==RAR_OM_LIST && */Data->Arc.Volume &&
				Data->Arc.GetHeaderType()==FILE_HEAD &&
				(Data->Arc.NewLhd.Flags & LHD_SPLIT_AFTER)!=0)
				if (MergeArchive(Data->Arc,NULL,false,'L'))
				{
					Data->Arc.Seek(Data->Arc.CurBlockPos,SEEK_SET);
					return(0);
				}
				else
					return(ERAR_EOPEN);
			Data->Arc.SeekToNext();
		}
		else
		{
			if (DestPath!=NULL || DestName!=NULL)
			{
				OemToChar(NullToEmpty(DestPath),Data->Cmd.ExtrPath);
				OemToChar(NullToEmpty(DestName),Data->Cmd.ArcName);
				AddEndSlash(Data->Cmd.ExtrPath);
			}
			else
			{
				*Data->Cmd.ExtrPath=0;
			}

			if (DestPathW!=NULL || DestNameW!=NULL)
			{
				strncpyw(Data->Cmd.ExtrPathW,NullToEmpty(DestPathW),NM-2);
				AddEndSlash(Data->Cmd.ExtrPathW);
			}
			else
			{
				*Data->Cmd.ExtrPathW=0;
			}

			strcpy(Data->Cmd.Command,Operation==RAR_EXTRACT ? "X":"T");
			Data->Cmd.Test=Operation!=RAR_EXTRACT;
			bool Repeat=false;

			Data->Extract.ExtractCurrentFile(&Data->Cmd,Data->Arc,Data->HeaderSize,Repeat);

			while (Data->Arc.ReadHeader()!=0 && Data->Arc.GetHeaderType()==NEWSUB_HEAD)
			{
				Data->Extract.ExtractCurrentFile(&Data->Cmd,Data->Arc,Data->HeaderSize,Repeat);
				Data->Arc.SeekToNext();
			}
			Data->Arc.Seek(Data->Arc.CurBlockPos,SEEK_SET);
		}
	}
	catch (int ErrCode)
	{
		return(RarErrorToDll(ErrCode));
	}
	catch(...)
	{
		return -1;
	}
	return 0;
}


int PASCAL RARProcessFile(HANDLE hArcData,int Operation,char *DestPath,char *DestName, string sParfileName)
{
	return(ProcessFile(hArcData,Operation,DestPath,DestName,NULL,NULL,sParfileName));
}


int PASCAL RARProcessFileW(HANDLE hArcData,int Operation,wchar *DestPath,wchar *DestName, string sParfileName)
{
	return(ProcessFile(hArcData,Operation,NULL,NULL,DestPath,DestName, sParfileName));
}


void PASCAL RARSetChangeVolProc(HANDLE hArcData,CHANGEVOLPROC ChangeVolProc)
{
	DataSet *Data=(DataSet *)hArcData;
}


void PASCAL RARSetCallback(HANDLE hArcData,UNRARCALLBACK Callback,LONG UserData)
{
	DataSet *Data=(DataSet *)hArcData;
}


void PASCAL RARSetProcessDataProc(HANDLE hArcData,PROCESSDATAPROC ProcessDataProc)
{
	DataSet *Data=(DataSet *)hArcData;
}


void PASCAL RARSetPassword(HANDLE hArcData,char *Password)
{
	DataSet *Data=(DataSet *)hArcData;
	strncpy(Data->Cmd.Password,Password,sizeof(Data->Cmd.Password));
}


int PASCAL RARGetDllVersion()
{
	return(RAR_DLL_VERSION);
}


static int RarErrorToDll(int ErrCode)
{
	switch(ErrCode)
	{
	case FATAL_ERROR:
		return(ERAR_EREAD);
	case CRC_ERROR:
		return(ERAR_BAD_DATA);
	case WRITE_ERROR:
		return(ERAR_EWRITE);
	case OPEN_ERROR:
		return(ERAR_EOPEN);
	case CREATE_ERROR:
		return(ERAR_ECREATE);
	case MEMORY_ERROR:
		return(ERAR_NO_MEMORY);
	case PASSWORD_ERROR:
		return(ERAR_PASSWORD);
	case SUCCESS:
		return(0);
	default:
		return(ERAR_UNKNOWN);
	}
}
