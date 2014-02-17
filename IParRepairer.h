#pragma once
#include "ParSettings.h"

class IParRepairer
{
public:
	IParRepairer(string sParfileName);
	
	bool m_bSkipVerify;

protected:
	string m_sParfileName;			//The name of the file	(ex: File.par)

public:
	virtual ~IParRepairer(void);

	virtual set<string> GetContainedFiles(string filename) = 0;
	virtual vector<string> GetOtherFiles(string filename) = 0;
	virtual bool Repair(string sDir, string sFile, vector<string> &vFiles, vector<string> &vRenamedFiles, vector<string> &vVerifiedFiles, ParSettings &fileSettings, vector<string> &vDoneFiles, bool bVerifyOnly) = 0;
};
