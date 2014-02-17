#ifndef _RAR_RECVOL_
#define _RAR_RECVOL_

class RecVolumes
{
  private:
    File *SrcFile[256];
    Array<byte> Buf;
	string m_sParfileName;
	vector<string> *m_pvRarFiles;
  public:
    RecVolumes(vector<string> *pvRarFiles, string sParfileName);
    ~RecVolumes();
    void Make(RAROptions *Cmd,char *ArcName,wchar *ArcNameW);
    bool Restore(RAROptions *Cmd,const char *Name,const wchar *NameW,bool Silent);
};

#endif
