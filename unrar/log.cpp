//
//	11/1/05 gmilow - Modified 

#include "stdafx.h"
#include "rar.hpp"

static void WriteToLog(const char *ArcName,const char *Message);

static char LogName[NM];


void Log(string sParfileName, const char *ArcName,const char *Format,...)
{
  safebuf char Msg[2*NM+1024];
  va_list ArgPtr;
  va_start(ArgPtr,Format);
  vsprintf(Msg,Format,ArgPtr);
  va_end(ArgPtr);
  eprintf(sParfileName, "%s",Msg);
}