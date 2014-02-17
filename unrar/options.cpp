//
//	11/1/05 gmilow - Modified 
#include "stdafx.h"
#include "rar.hpp"

RAROptions::RAROptions(string sParfileName) : NextVolSizes(sParfileName)
{
	Init();
}


RAROptions::~RAROptions()
{
	memset(this,0,sizeof(RAROptions));
}


void RAROptions::Init()
{
	memset(this,0,sizeof(RAROptions));
	WinSize=0x400000;
	Overwrite=OVERWRITE_ASK;
	Method=3;
	MsgStream=MSG_STDOUT;
	ConvertNames=NAMES_ORIGINALCASE;
	ProcessEA=true;
	xmtime=EXTTIME_HIGH3;
	CurVolNum=0;
}
