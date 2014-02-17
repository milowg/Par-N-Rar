//
//	11/1/05 gmilow - Modified 
#include "stdafx.h"
#include "rar.hpp"

static bool UserBreak;

ErrorHandler::ErrorHandler()
{
	Clean();
}

void ErrorHandler::Clean()
{
	ExitCode=SUCCESS;
	ErrCount=0;
	EnableBreak=true;
	Silent=false;
	DoShutdown=false;
}


void ErrorHandler::MemoryError(string sParfileName)
{
	MemoryErrorMsg(sParfileName);
	Throw(MEMORY_ERROR);
}


void ErrorHandler::OpenError(string sParfileName, const char *FileName)
{
#ifndef SILENT
	OpenErrorMsg(sParfileName,FileName);
	Throw(OPEN_ERROR);
#endif
}


void ErrorHandler::CloseError(string sParfileName, const char *FileName)
{
#ifndef SILENT
	if (!UserBreak)
	{
		ErrMsg(sParfileName, NULL,St(MErrFClose),FileName);
		SysErrMsg(sParfileName);
	}
#endif
#if !defined(SILENT) || defined(RARDLL)
	Throw(FATAL_ERROR);
#endif
}


void ErrorHandler::ReadError(string sParfileName, const char *FileName)
{
#ifndef SILENT
	ReadErrorMsg(sParfileName, NULL,FileName);
#endif
#if !defined(SILENT) || defined(RARDLL)
	Throw(FATAL_ERROR);
#endif
}


bool ErrorHandler::AskRepeatRead(vector<string> *pvRarFiles, string sParfileName, const char *FileName)
{
#if !defined(SILENT) && !defined(SFX_MODULE) && !defined(_WIN_CE)
	if (!Silent)
	{
		mprintf(sParfileName,"\n");
		Log(sParfileName,NULL,St(MErrRead),FileName);
		return(Ask(pvRarFiles, sParfileName, St(MRetryAbort))==1);
	}
#endif
	return(false);
}


void ErrorHandler::WriteError(string sParfileName, const char *ArcName,const char *FileName)
{
#ifndef SILENT
	WriteErrorMsg(sParfileName, ArcName,FileName);
#endif
#if !defined(SILENT) || defined(RARDLL)
	Throw(WRITE_ERROR);
#endif
}


#ifdef _WIN_32
void ErrorHandler::WriteErrorFAT(string sParfileName, const char *FileName)
{
#if !defined(SILENT) && !defined(SFX_MODULE)
	SysErrMsg(sParfileName);
	ErrMsg(sParfileName, NULL,St(MNTFSRequired),FileName);
#endif
#if !defined(SILENT) && !defined(SFX_MODULE) || defined(RARDLL)
	Throw(WRITE_ERROR);
#endif
}
#endif


bool ErrorHandler::AskRepeatWrite(vector<string> *pvRarFiles, string sParfileName, const char *FileName)
{
#if !defined(SILENT) && !defined(_WIN_CE)
	if (!Silent)
	{
		mprintf(sParfileName,"\n");
		Log(sParfileName,NULL,St(MErrWrite),FileName);
		return(Ask(pvRarFiles, sParfileName,St(MRetryAbort))==1);
	}
#endif
	return(false);
}


void ErrorHandler::SeekError(string sParfileName, const char *FileName)
{
#ifndef SILENT
	if (!UserBreak)
	{
		ErrMsg(sParfileName, NULL,St(MErrSeek),FileName);
		SysErrMsg(sParfileName);
	}
#endif
#if !defined(SILENT) || defined(RARDLL)
	Throw(FATAL_ERROR);
#endif
}


void ErrorHandler::MemoryErrorMsg(string sParfileName)
{
#ifndef SILENT
	ErrMsg(sParfileName, NULL,St(MErrOutMem));
#endif
}


void ErrorHandler::OpenErrorMsg(string sParfileName, const char *FileName)
{
	OpenErrorMsg(sParfileName,NULL,FileName);
}


void ErrorHandler::OpenErrorMsg(string sParfileName, const char *ArcName,const char *FileName)
{
#ifndef SILENT
	Log(sParfileName,ArcName && *ArcName ? ArcName:NULL,St(MCannotOpen),FileName);
	Alarm();
	SysErrMsg(sParfileName);
#endif
}


void ErrorHandler::CreateErrorMsg(string sParfileName, const char *FileName)
{
	CreateErrorMsg(sParfileName,NULL,FileName);
}


void ErrorHandler::CreateErrorMsg(string sParfileName, const char *ArcName,const char *FileName)
{
#ifndef SILENT
	Log(sParfileName,ArcName && *ArcName ? ArcName:NULL,St(MCannotCreate),FileName);
	Alarm();
#if defined(_WIN_32) && !defined(_WIN_CE) && !defined(SFX_MODULE) && defined(MAXPATH)
	if (GetLastError()==ERROR_PATH_NOT_FOUND)
	{
		int NameLength=strlen(FileName);
		if (!IsFullPath(FileName))
		{
			char CurDir[NM];
			GetCurrentDirectory(sizeof(CurDir),CurDir);
			NameLength+=strlen(CurDir)+1;
		}
		if (NameLength>MAXPATH)
		{
			Log(m_sParfileName,ArcName && *ArcName ? ArcName:NULL,St(MMaxPathLimit),MAXPATH);
		}
	}
#endif
	SysErrMsg(sParfileName);
#endif
}


void ErrorHandler::ReadErrorMsg(string sParfileName, const char *ArcName,const char *FileName)
{
#ifndef SILENT
	ErrMsg(sParfileName, ArcName,St(MErrRead),FileName);
	SysErrMsg(sParfileName);
#endif
}


void ErrorHandler::WriteErrorMsg(string sParfileName, const char *ArcName,const char *FileName)
{
#ifndef SILENT
	ErrMsg(sParfileName, ArcName,St(MErrWrite),FileName);
	SysErrMsg(sParfileName);
#endif
}


void ErrorHandler::Exit(int ExitCode)
{
#ifndef SFX_MODULE
	Alarm();
#endif
	Throw(ExitCode);
}


#ifndef GUI
void ErrorHandler::ErrMsg(string sParfileName, const char *ArcName,const char *fmt,...)
{
	safebuf char Msg[NM+1024];
	va_list argptr;
	va_start(argptr,fmt);
	vsprintf(Msg,fmt,argptr);
	va_end(argptr);
#ifdef _WIN_32
	if (UserBreak)
		Sleep(5000);
#endif
	Alarm();
	if (*Msg)
	{
		Log(sParfileName,ArcName,"%s",Msg);
		mprintf(sParfileName, "%s",St(MProgAborted));
	}
}
#endif


void ErrorHandler::SetErrorCode(int Code)
{
	switch(Code)
	{
	case WARNING:
	case USER_BREAK:
		if (ExitCode==SUCCESS)
			ExitCode=Code;
		break;
	case FATAL_ERROR:
		if (ExitCode==SUCCESS || ExitCode==WARNING)
			ExitCode=FATAL_ERROR;
		break;
	default:
		ExitCode=Code;
		break;
	}
	ErrCount++;
}


#if !defined(GUI) && !defined(_SFX_RTL_)
#ifdef _WIN_32
BOOL __stdcall ProcessSignal(DWORD SigType)
#else
#if defined(__sun)
extern "C"
#endif
void _stdfunction ProcessSignal(int SigType)
#endif
{
#ifdef _WIN_32
	if (SigType==CTRL_LOGOFF_EVENT)
		return(TRUE);
#endif
	UserBreak=true;
	//mprintf(sParfileName, St(MBreak));
	for (int I=0;!File::RemoveCreated() && I<3;I++)
	{
#ifdef _WIN_32
		Sleep(100);
#endif
	}
#if defined(USE_RC) && !defined(SFX_MODULE) && !defined(_WIN_CE)
	ExtRes.UnloadDLL();
#endif
	exit(USER_BREAK);
#ifdef _WIN_32
	return(TRUE);
#endif
}
#endif


void ErrorHandler::SetSignalHandlers(bool Enable)
{
	EnableBreak=Enable;
#if !defined(GUI) && !defined(_SFX_RTL_)
#ifdef _WIN_32
	SetConsoleCtrlHandler(Enable ? ProcessSignal:NULL,TRUE);
	//  signal(SIGBREAK,Enable ? ProcessSignal:SIG_IGN);
#else
	signal(SIGINT,Enable ? ProcessSignal:SIG_IGN);
	signal(SIGTERM,Enable ? ProcessSignal:SIG_IGN);
#endif
#endif
}


void ErrorHandler::Throw(int Code)
{
	if (Code==USER_BREAK && !EnableBreak)
		return;
	ErrHandler.Clean();

	throw Code;
}


void ErrorHandler::SysErrMsg(string sParfileName)
{
#if defined(_WIN_32) && !defined(SFX_MODULE) && !defined(SILENT)
#define STRCHR strchr
#define ERRCHAR char
	ERRCHAR  *lpMsgBuf=NULL;
	int ErrType=GetLastError();
	if (ErrType!=0 && FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,ErrType,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,0,NULL))
	{
		ERRCHAR  *CurMsg=lpMsgBuf;
		while (CurMsg!=NULL)
		{
			while (*CurMsg=='\r' || *CurMsg=='\n')
				CurMsg++;
			if (*CurMsg==0)
				break;
			ERRCHAR *EndMsg=STRCHR(CurMsg,'\r');
			if (EndMsg==NULL)
				EndMsg=STRCHR(CurMsg,'\n');
			if (EndMsg!=NULL)
			{
				*EndMsg=0;
				EndMsg++;
			}
			Log(sParfileName,NULL,"%s",CurMsg);
			CurMsg=EndMsg;
		}
	}
	LocalFree( lpMsgBuf );
#endif
}
