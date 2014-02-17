#ifndef _RAR_FILESTR_
#define _RAR_FILESTR_

bool ReadTextFile(vector<string> *pvRarFiles, string sParfileName, char *Name,StringList *List,bool Config,
                  bool AbortOnError=false,bool ConvertToAnsi=false,
                  bool Unquote=false,bool SkipComments=false);

#endif
