#ifndef _RAR_CONSIO_
#define _RAR_CONSIO_

enum {ALARM_SOUND,ERROR_SOUND,QUESTION_SOUND};

enum PASSWORD_TYPE {PASSWORD_GLOBAL,PASSWORD_FILE,PASSWORD_ARCHIVE};

void InitConsoleOptions(MESSAGE_TYPE MsgStream,bool Sound);

#ifndef SILENT
void mprintf(string sParfileName,const char *fmt,...);
void eprintf(string sParfileName,const char *fmt,...);
void Alarm();
void GetPasswordText(string sParfileName, char *Prompt, char *Str,int MaxLength);
unsigned int GetKey(vector<string> *pvRarFiles, string sParfileName);
bool GetPassword(string sParfileName, PASSWORD_TYPE Type,const char *FileName,char *Password,int MaxLength);
int Ask(vector<string> *pvRarFiles, string sParfileName,const char *AskStr);
#endif

int KbdAnsi(char *Addr,int Size);
void OutComment(string sParfileName,char *Comment,int Size);

#ifdef SILENT
inline void mprintf(string sParfileName,const char *fmt,const char *a=NULL,const char *b=NULL) {}
inline void eprintf(string sParfileName,const char *fmt,const char *a=NULL,const char *b=NULL) {}
inline void mprintf(string sParfileName,const char *fmt,int b) {}
inline void eprintf(string sParfileName,const char *fmt,int b) {}
inline void mprintf(string sParfileName,const char *fmt,const char *a,int b) {}
inline void eprintf(string sParfileName,const char *fmt,const char *a,int b) {}
inline void Alarm() {}
inline void GetPasswordText(char *Str,int MaxLength) {}
inline unsigned int GetKey(string sParfileName) {return(0);}
inline bool GetPassword(string sParfileName, PASSWORD_TYPE Type,const char *FileName,char *Password,int MaxLength) {return(false);}
inline int Ask(vector<string> *pvRarFiles, string sParfileName,const char *AskStr) {return(0);}
#endif

#endif
