//	11/1/05 gmilow - Modified 

#include "stdafx.h"
#include "rar.hpp"

static void ListFileHeader(string sParfileName, FileHeader &hd,bool Verbose,bool Technical,bool &TitleShown,bool Bare);
static void ListSymLink(Archive &Arc);
static void ListFileAttr(string sParfileName, uint A,int HostOS);
static void ListOldSubHeader(Archive &Arc);
static void ListNewSubHeader(CommandData *Cmd,Archive &Arc,bool Technical);

void ListArchive(vector<string> *pvRarFiles, string sParfileName, CommandData *Cmd)
{
	Int64 SumPackSize=0,SumUnpSize=0;
	uint ArcCount=0,SumFileCount=0;
	bool Technical=(Cmd->Command[1]=='T');
	bool Bare=(Cmd->Command[1]=='B');
	bool Verbose=(*Cmd->Command=='V');

	char ArcName[NM];
	wchar ArcNameW[NM];

	while (Cmd->GetArcName(ArcName,ArcNameW,sizeof(ArcName)))
	{
		Archive Arc(pvRarFiles, sParfileName,Cmd);
		if (!Arc.WOpen(ArcName,ArcNameW))
			continue;
		bool FileMatched=true;
		while (1)
		{
			Int64 TotalPackSize=0,TotalUnpSize=0;
			uint FileCount=0;
			if (Arc.IsArchive(true))
			{
				if (!Arc.IsOpened())
					break;
				bool TitleShown=false;
				if (!Bare)
				{
					Arc.ViewComment();
					mprintf(sParfileName,"\n");
					if (Arc.Solid)
						mprintf(sParfileName,St(MListSolid));
					if (Arc.SFXSize>0)
						mprintf(sParfileName,St(MListSFX));
					if (Arc.Volume)
						if (Arc.Solid)
							mprintf(sParfileName,St(MListVol1));
						else
							mprintf(sParfileName,St(MListVol2));
					else
						if (Arc.Solid)
							mprintf(sParfileName,St(MListArc1));
						else
							mprintf(sParfileName,St(MListArc2));
					mprintf(sParfileName," %s\n",Arc.FileName);
					if (Technical)
					{
						if (Arc.Protected)
							mprintf(sParfileName,St(MListRecRec));
						if (Arc.Locked)
							mprintf(sParfileName,St(MListLock));
					}
				}
				while(Arc.ReadHeader()>0)
				{
					switch(Arc.GetHeaderType())
					{
					case FILE_HEAD:
						IntToExt(Arc.NewLhd.FileName,Arc.NewLhd.FileName);
						if ((FileMatched=Cmd->IsProcessFile(Arc.NewLhd))==true)
						{
							ListFileHeader(Arc.m_sParfileName,Arc.NewLhd,Verbose,Technical,TitleShown,Bare);
							if (!(Arc.NewLhd.Flags & LHD_SPLIT_BEFORE))
							{
								TotalUnpSize+=Arc.NewLhd.FullUnpSize;
								FileCount++;
							}
							TotalPackSize+=Arc.NewLhd.FullPackSize;
							if (Technical)
								ListSymLink(Arc);
#ifndef SFX_MODULE
							if (Verbose)
								Arc.ViewFileComment();
#endif
						}
						break;
#ifndef SFX_MODULE
					case SUB_HEAD:
						if (Technical && FileMatched && !Bare)
							ListOldSubHeader(Arc);
						break;
#endif
					case NEWSUB_HEAD:
						if (FileMatched && !Bare)
						{
							if (Technical)
								ListFileHeader(Arc.m_sParfileName,Arc.SubHead,Verbose,true,TitleShown,false);
							ListNewSubHeader(Cmd,Arc,Technical);
						}
						break;
					}
					Arc.SeekToNext();
				}
				if (!Bare)
					if (TitleShown)
					{
						mprintf(sParfileName,"\n");
						for (int I=0;I<79;I++)
							mprintf(sParfileName,"-");
						char UnpSizeText[20];
						itoa(TotalUnpSize,UnpSizeText);

						char PackSizeText[20];
						itoa(TotalPackSize,PackSizeText);

						mprintf(sParfileName,"%5lu %16s %8s %3d%%",FileCount,UnpSizeText,
							PackSizeText,ToPercent(TotalPackSize,TotalUnpSize));
						SumFileCount+=FileCount;
						SumUnpSize+=TotalUnpSize;
						SumPackSize+=TotalPackSize;
#ifndef SFX_MODULE
						if (Arc.EndArcHead.Flags & EARC_VOLNUMBER)
						{
							mprintf(sParfileName,St(MVolumeNumber),Arc.EndArcHead.VolNumber+1);
						}
#endif
						mprintf(sParfileName,"\n");
					}
					else
						mprintf(sParfileName,St(MListNoFiles));

				ArcCount++;

#ifndef NOVOLUME
				if (Cmd->VolSize!=0 && ((Arc.NewLhd.Flags & LHD_SPLIT_AFTER) ||
					Arc.GetHeaderType()==ENDARC_HEAD &&
					(Arc.EndArcHead.Flags & EARC_NEXT_VOLUME)!=0) &&
					MergeArchive(Arc,NULL,false,*Cmd->Command))
				{
					Arc.Seek(0,SEEK_SET);
				}
				else
#endif
					break;
			}
			else
			{
				if (Cmd->ArcNames->ItemsCount()<2 && !Bare)
					mprintf(sParfileName,St(MNotRAR),Arc.FileName);
				break;
			}
		}
	}
	if (ArcCount>1 && !Bare)
	{
		char UnpSizeText[20],PackSizeText[20];
		itoa(SumUnpSize,UnpSizeText);
		itoa(SumPackSize,PackSizeText);
		mprintf(sParfileName,"%5lu %16s %8s %3d%%\n",SumFileCount,UnpSizeText,
			PackSizeText,ToPercent(SumPackSize,SumUnpSize));
	}
}


void ListFileHeader(string sParfileName, FileHeader &hd,bool Verbose,bool Technical,bool &TitleShown,bool Bare)
{
	if (!Bare)
	{
		if (!TitleShown)
		{
			if (Verbose)
				mprintf(sParfileName,St(MListPathComm));
			else
				mprintf(sParfileName,St(MListName));
			mprintf(sParfileName,St(MListTitle));
			if (Technical)
				mprintf(sParfileName,St(MListTechTitle));
			for (int I=0;I<79;I++)
				mprintf(sParfileName,"-");
			TitleShown=true;
		}

		if (hd.HeadType==NEWSUB_HEAD)
			mprintf(sParfileName,St(MSubHeadType),hd.FileName);

		mprintf(sParfileName,"%c",(hd.Flags & LHD_PASSWORD) ? '*' : ' ');
	}

	char *Name=hd.FileName;

#ifdef UNICODE_SUPPORTED
	char ConvertedName[NM];
	if ((hd.Flags & LHD_UNICODE)!=0 && *hd.FileNameW!=0 && UnicodeEnabled())
	{
		WideToChar(hd.FileNameW,ConvertedName);
		Name=ConvertedName;
	}
#endif

	if (Bare)
	{
		mprintf(sParfileName,"%s\n",Verbose ? Name:PointToName(Name));
		return;
	}

	if (Verbose)
		mprintf(sParfileName,"%s%12s ",Name,"");
	else
		mprintf(sParfileName,"%-12s",PointToName(Name));

	char UnpSizeText[20],PackSizeText[20];
	if (hd.FullUnpSize==INT64MAX)
		strcpy(UnpSizeText,"?");
	else
		itoa(hd.FullUnpSize,UnpSizeText);
	itoa(hd.FullPackSize,PackSizeText);

	mprintf(sParfileName," %8s %8s ",UnpSizeText,PackSizeText);

	if ((hd.Flags & LHD_SPLIT_BEFORE) && (hd.Flags & LHD_SPLIT_AFTER))
		mprintf(sParfileName," <->");
	else
		if (hd.Flags & LHD_SPLIT_BEFORE)
			mprintf(sParfileName," <--");
		else
			if (hd.Flags & LHD_SPLIT_AFTER)
				mprintf(sParfileName," -->");
			else
				mprintf(sParfileName,"%3d%%",ToPercent(hd.FullPackSize,hd.FullUnpSize));

	char DateStr[50];
	hd.mtime.GetText(DateStr,false);
	mprintf(sParfileName," %s ",DateStr);

	if (hd.HeadType==NEWSUB_HEAD)
		mprintf(sParfileName,"  %c....B  ",(hd.SubFlags & SUBHEAD_FLAGS_INHERITED) ? 'I' : '.');
	else
		ListFileAttr(sParfileName,hd.FileAttr,hd.HostOS);

	mprintf(sParfileName," %8.8lX",hd.FileCRC);
	mprintf(sParfileName," m%d",hd.Method-0x30);
	if ((hd.Flags & LHD_WINDOWMASK)<=6*32)
		mprintf(sParfileName,"%c",((hd.Flags&LHD_WINDOWMASK)>>5)+'a');
	else
		mprintf(sParfileName," ");
	mprintf(sParfileName," %d.%d",hd.UnpVer/10,hd.UnpVer%10);

	static char *RarOS[]={
		"DOS","OS/2","Win95/NT","Unix","MacOS","BeOS","WinCE","","",""
	};

	if (Technical)
		mprintf(sParfileName,"%22s %8s %4s",RarOS[hd.HostOS],
		(hd.Flags & LHD_SOLID) ? St(MYes):St(MNo),
		(hd.Flags & LHD_VERSION) ? St(MYes):St(MNo));
}


void ListSymLink(Archive &Arc)
{
	if (Arc.NewLhd.HostOS==HOST_UNIX && (Arc.NewLhd.FileAttr & 0xF000)==0xA000)
	{
		char FileName[NM];
		int DataSize=Min(Arc.NewLhd.PackSize,sizeof(FileName)-1);
		Arc.Read(FileName,DataSize);
		FileName[DataSize]=0;
		mprintf(Arc.m_sParfileName,"%22s %s","-->",FileName);
	}
}


void ListFileAttr(string sParfileName, uint A,int HostOS)
{
	switch(HostOS)
	{
	case HOST_MSDOS:
	case HOST_OS2:
	case HOST_WIN32:
	case HOST_MACOS:
		mprintf(sParfileName," %c%c%c%c%c%c%c  ",
			(A & 0x08) ? 'V' : '.',
			(A & 0x10) ? 'D' : '.',
			(A & 0x01) ? 'R' : '.',
			(A & 0x02) ? 'H' : '.',
			(A & 0x04) ? 'S' : '.',
			(A & 0x20) ? 'A' : '.',
			(A & 0x800) ? 'C' : '.');
		break;
	case HOST_UNIX:
	case HOST_BEOS:
		switch (A & 0xF000)
		{
		case 0x4000:
			mprintf(sParfileName,"d");
			break;
		case 0xA000:
			mprintf(sParfileName,"l");
			break;
		default:
			mprintf(sParfileName,"-");
			break;
		}
		mprintf(sParfileName,"%c%c%c%c%c%c%c%c%c",
			(A & 0x0100) ? 'r' : '-',
			(A & 0x0080) ? 'w' : '-',
			(A & 0x0040) ? ((A & 0x0800) ? 's':'x'):((A & 0x0800) ? 'S':'-'),
			(A & 0x0020) ? 'r' : '-',
			(A & 0x0010) ? 'w' : '-',
			(A & 0x0008) ? ((A & 0x0400) ? 's':'x'):((A & 0x0400) ? 'S':'-'),
			(A & 0x0004) ? 'r' : '-',
			(A & 0x0002) ? 'w' : '-',
			(A & 0x0001) ? 'x' : '-');
		break;
	}
}


#ifndef SFX_MODULE
void ListOldSubHeader(Archive &Arc)
{
	switch(Arc.SubBlockHead.SubType)
	{
	case EA_HEAD:
		mprintf(Arc.m_sParfileName,St(MListEAHead));
		break;
	case UO_HEAD:
		mprintf(Arc.m_sParfileName,St(MListUOHead),Arc.UOHead.OwnerName,Arc.UOHead.GroupName);
		break;
	case MAC_HEAD:
		mprintf(Arc.m_sParfileName,St(MListMACHead1),Arc.MACHead.fileType>>24,Arc.MACHead.fileType>>16,Arc.MACHead.fileType>>8,Arc.MACHead.fileType);
		mprintf(Arc.m_sParfileName,St(MListMACHead2),Arc.MACHead.fileCreator>>24,Arc.MACHead.fileCreator>>16,Arc.MACHead.fileCreator>>8,Arc.MACHead.fileCreator);
		break;
	case BEEA_HEAD:
		mprintf(Arc.m_sParfileName,St(MListBeEAHead));
		break;
	case NTACL_HEAD:
		mprintf(Arc.m_sParfileName,St(MListNTACLHead));
		break;
	case STREAM_HEAD:
		mprintf(Arc.m_sParfileName,St(MListStrmHead),Arc.StreamHead.StreamName);
		break;
	default:
		mprintf(Arc.m_sParfileName,St(MListUnkHead),Arc.SubBlockHead.SubType);
		break;
	}
}
#endif


void ListNewSubHeader(CommandData *Cmd,Archive &Arc,bool Technical)
{
	if (Arc.SubHead.CmpName(SUBHEAD_TYPE_CMT) &&
		(Arc.SubHead.Flags & LHD_SPLIT_BEFORE)==0 && !Cmd->DisableComment)
	{
		Array<byte> CmtData(Cmd->m_sParfileName);
		int ReadSize=Arc.ReadCommentData(CmtData);
		if (ReadSize!=0)
		{
			mprintf(Arc.m_sParfileName,St(MFileComment));
			OutComment(Arc.m_sParfileName, (char *)&CmtData[0],ReadSize);
		}
	}
	if (Arc.SubHead.CmpName(SUBHEAD_TYPE_STREAM) &&
		(Arc.SubHead.Flags & LHD_SPLIT_BEFORE)==0)
	{
		int DestSize=Arc.SubHead.SubData.Size()/2;
		wchar DestNameW[NM];
		char DestName[NM];
		if (DestSize<sizeof(DestName))
		{
			RawToWide(&Arc.SubHead.SubData[0],DestNameW,DestSize);
			DestNameW[DestSize]=0;
			WideToChar(DestNameW,DestName);
			mprintf(Arc.m_sParfileName,"\n %s",DestName);
		}
	}
}
