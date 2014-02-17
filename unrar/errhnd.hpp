#ifndef _RAR_ERRHANDLER_
#define _RAR_ERRHANDLER_

#define rarmalloc malloc
#define rarcalloc calloc
#define rarrealloc realloc
#define rarfree free
#define rarstrdup strdup



enum { SUCCESS,WARNING,FATAL_ERROR,CRC_ERROR,LOCK_ERROR,WRITE_ERROR,
       OPEN_ERROR,USER_ERROR,MEMORY_ERROR,PASSWORD_ERROR=254,CREATE_ERROR,USER_BREAK=255};

class ErrorHandler
{
  private:
    void ErrMsg(string sParfileName, const char *ArcName,const char *fmt,...);

    int ExitCode;
    int ErrCount;
    bool EnableBreak;
    bool Silent;
    bool DoShutdown;
  public:
    ErrorHandler();
    void Clean();
    void MemoryError(string sParfileName);
    void OpenError(string sParfileName, const char *FileName);
    void CloseError(string sParfileName, const char *FileName);
    void ReadError(string sParfileName, const char *FileName);
    bool AskRepeatRead(vector<string> *pvRarFiles, string sParfileName, const char *FileName);
    void WriteError(string sParfileName, const char *ArcName,const char *FileName);
    void WriteErrorFAT(string sParfileName, const char *FileName);
    bool AskRepeatWrite(vector<string> *pvRarFiles, string sParfileName, const char *FileName);
    void SeekError(string sParfileName, const char *FileName);
    void MemoryErrorMsg(string sParfileName);
    void OpenErrorMsg(string sParfileName, const char *FileName);
    void OpenErrorMsg(string sParfileName, const char *ArcName,const char *FileName);
    void CreateErrorMsg(string sParfileName, const char *FileName);
    void CreateErrorMsg(string sParfileName, const char *ArcName,const char *FileName);
    void ReadErrorMsg(string sParfileName, const char *ArcName,const char *FileName);
    void WriteErrorMsg(string sParfileName, const char *ArcName,const char *FileName);
    void Exit(int ExitCode);
    void SetErrorCode(int Code);
    int GetErrorCode() {return(ExitCode);}
    int GetErrorCount() {return(ErrCount);}
    void SetSignalHandlers(bool Enable);
    void Throw(int Code);
    void SetSilent(bool Mode) {Silent=Mode;};
    void SetShutdown(bool Mode) {DoShutdown=Mode;};
    void SysErrMsg(string sParfileName);
};

#endif
