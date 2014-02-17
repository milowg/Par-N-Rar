//
//	11/1/05 gmilow - Modified 
#include "stdafx.h"
#include "rar.hpp"
#include "dll.hpp"
#include "../PnrMessage.h"
#include "../GetPasswordForm.h"

static void RawPrint(string sParfileName, char *Msg, MESSAGE_TYPE MessageType);
static MESSAGE_TYPE MsgStream=MSG_STDOUT;
static bool Sound=false;
const int MaxMsgSize=2*NM+2048;

void InitConsoleOptions(MESSAGE_TYPE MsgStream,bool Sound)
{
	::MsgStream=MsgStream;
	::Sound=Sound;
}

#if !defined(GUI) && !defined(SILENT)
void mprintf(string sParfileName, const char *fmt,...)
{
	if (MsgStream==MSG_NULL)
		return;
	safebuf char Msg[MaxMsgSize];
	va_list argptr;
	va_start(argptr,fmt);
	vsprintf(Msg,fmt,argptr);
	RawPrint(sParfileName,Msg,MsgStream);
	va_end(argptr);
}
#endif


#if !defined(GUI) && !defined(SILENT)
void eprintf(string sParfileName, const char *fmt,...)
{
	if (MsgStream==MSG_NULL)
		return;
	safebuf char Msg[MaxMsgSize];
	va_list argptr;
	va_start(argptr,fmt);
	vsprintf(Msg,fmt,argptr);
	RawPrint(sParfileName,Msg,MSG_STDERR);
	va_end(argptr);
}
#endif


#if !defined(GUI) && !defined(SILENT)
void RawPrint(string sParfileName, char *Msg,MESSAGE_TYPE MessageType)
{
#ifdef _WIN_32
	CharToOem(Msg,Msg);

	char OutMsg[MaxMsgSize],*OutPos=OutMsg;
	for (int I=0;Msg[I]!=0;I++)
	{
		if (Msg[I]=='\n' && (I==0 || Msg[I-1]!='\r'))
			*(OutPos++)='\r';
		*(OutPos++)=Msg[I];
	}
	*OutPos=0;
	strcpy(Msg,OutMsg);
#endif
#if defined(_UNIX) || defined(_EMX)
	char OutMsg[MaxMsgSize],*OutPos=OutMsg;
	for (int I=0;Msg[I]!=0;I++)
		if (Msg[I]!='\r')
			*(OutPos++)=Msg[I];
	*OutPos=0;
	strcpy(Msg,OutMsg);
#endif

	switch(MessageType)
	{
	case MSG_STDOUT:
		CPnrMessage::SendParAddDetails(sParfileName, Msg, true);
		break;
	case MSG_STDERR:
		CPnrMessage::SendParAddDetails(sParfileName, Msg, true);
		break;
	default:
		return;
	}
}
#endif


#ifndef SILENT
void Alarm()
{
#ifndef SFX_MODULE
	if (Sound)
		putchar('\007');
#endif
}
#endif

void GetPasswordText(string sParfileName, char *Prompt, char *Str,int MaxLength)
{
	CGetPasswordDlg dlg(theApp.GetMainWnd());

	dlg.SetLimit(MaxLength);
	dlg.SetPrompt( Prompt );
	if (dlg.DoModal() == IDCANCEL)
	{
		if (MaxLength > 0)
			Str[0] = 0;

		CPnrMessage::SendParAddDetails(sParfileName, "Password required but dialog was cancelled or timeout expired.", true);
		throw ERAR_PASSWORD;		
		return;
	}

	memset(Str, 0, MaxLength-1);
	strncpy(Str, dlg.m_cstrPassword.GetBuffer(0), dlg.m_cstrPassword.GetLength());
	RemoveLF(Str);
}

#if !defined(GUI) && !defined(SILENT)
unsigned int GetKey(vector<string> *pvRarFiles, string sParfileName)
{
#ifdef SILENT
	return(0);
#else
	char Str[80];
#ifdef __GNUC__
	fgets(Str,sizeof(Str),stdin);
	return(Str[0]);
#else
	File SrcFile(pvRarFiles, sParfileName);
	SrcFile.SetHandleType(FILE_HANDLESTD);
	SrcFile.Read(Str,sizeof(Str));
	return(Str[0]);
#endif
#endif
}
#endif


#ifndef SILENT
bool GetPassword(string sParfileName, PASSWORD_TYPE Type,const char *FileName,char *Password,int MaxLength)
{
	Alarm();
	while (true)
	{
		char PromptStr[256];
#if defined(_EMX) || defined(_BEOS)
		strcpy(PromptStr,St(MAskPswEcho));
#else
		strcpy(PromptStr,St(MAskPsw));
#endif
		if (Type!=PASSWORD_GLOBAL)
		{
			strcat(PromptStr,St(MFor));
			strcat(PromptStr,PointToName(FileName));
		}

		//gm - ask for password
		GetPasswordText(sParfileName, PromptStr,Password,MaxLength);

		if (*Password==0 && Type==PASSWORD_GLOBAL)
			return(false);
		if (!strcmp(Password, ""))
			throw ERAR_PASSWORD;

		if (Type==PASSWORD_GLOBAL)
		{
			strcpy(PromptStr,St(MReAskPsw));
			char CmpStr[256];
			GetPasswordText(sParfileName, PromptStr, CmpStr,sizeof(CmpStr));
			if (*CmpStr==0 || strcmp(Password,CmpStr)!=0)
			{
				strcpy(PromptStr,St(MNotMatchPsw));
				/*
				#ifdef _WIN_32
				CharToOem(PromptStr,PromptStr);
				#endif
				*/
				//eprintf(sParfileName,PromptStr);
				memset(Password,0,MaxLength);
				memset(CmpStr,0,sizeof(CmpStr));
				continue;
			}
			memset(CmpStr,0,sizeof(CmpStr));
		}
		break;
	}
	return(true);
}
#endif


#if !defined(GUI) && !defined(SILENT)
int Ask(vector<string> *pvRarFiles, string sParfileName, const char *AskStr)
{
	const int MaxItems=10;
	char Item[MaxItems][40];
	int ItemKeyPos[MaxItems],NumItems=0;

	for (const char *NextItem=AskStr;NextItem!=NULL;NextItem=strchr(NextItem+1,'_'))
	{
		char *CurItem=Item[NumItems];
		strncpy(CurItem,NextItem+1,sizeof(Item[0]));
		char *EndItem=strchr(CurItem,'_');
		if (EndItem!=NULL)
			*EndItem=0;
		int KeyPos=0,CurKey;
		while ((CurKey=CurItem[KeyPos])!=0)
		{
			bool Found=false;
			for (int I=0;I<NumItems && !Found;I++)
				if (loctoupper(Item[I][ItemKeyPos[I]])==loctoupper(CurKey))
					Found=true;
			if (!Found && CurKey!=' ')
				break;
			KeyPos++;
		}
		ItemKeyPos[NumItems]=KeyPos;
		NumItems++;
	}

	for (int I=0;I<NumItems;I++)
	{
		eprintf(sParfileName,I==0 ? (NumItems>4 ? "\n":" "):", ");
		int KeyPos=ItemKeyPos[I];
		for (int J=0;J<KeyPos;J++)
			eprintf(sParfileName,"%c",Item[I][J]);
		eprintf(sParfileName,"[%c]%s",Item[I][KeyPos],&Item[I][KeyPos+1]);
	}
	eprintf(sParfileName," ");
	int Ch=GetKey(pvRarFiles, sParfileName);
#if defined(_WIN_32)
	OemToCharBuff((LPCSTR)&Ch,(LPTSTR)&Ch,1);
#endif
	Ch=loctoupper(Ch);
	for (int I=0;I<NumItems;I++)
		if (Ch==Item[I][ItemKeyPos[I]])
			return(I+1);
	return(0);
}
#endif


int KbdAnsi(char *Addr,int Size)
{
	int RetCode=0;
#ifndef GUI
	for (int I=0;I<Size;I++)
		if (Addr[I]==27 && Addr[I+1]=='[')
		{
			for (int J=I+2;J<Size;J++)
			{
				if (Addr[J]=='\"')
					return(2);
				if (!isdigit(Addr[J]) && Addr[J]!=';')
					break;
			}
			RetCode=1;
		}
#endif
		return(RetCode);
}


void OutComment(string sParfileName, char *Comment,int Size)
{
#ifndef GUI
	if (KbdAnsi(Comment,Size)==2)
		return;
	const int MaxOutSize=0x400;
	for (int I=0;I<Size;I+=MaxOutSize)
	{
		char Msg[MaxOutSize+1];
		int CopySize=Min(MaxOutSize,Size-I);
		strncpy(Msg,Comment+I,CopySize);
		Msg[CopySize]=0;
		mprintf(sParfileName, "%s",Msg);
	}
	mprintf(sParfileName,"\n");
#endif
}
