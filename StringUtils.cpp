//  This file is part of Par-N-Rar
//  http://www.milow.net/site/projects/parnrar.html
//
//  Copyright (c) Gil Milow
//
//  Par-N-Rar is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  Par-N-Rar is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
//  This code may not be used to develop a RAR (WinRAR) compatible archiver.
//

#include "stdafx.h"
#include "StringUtils.h"
#include <algorithm>
#include <cctype>

//Taken from par2-cmdline\diskfile.cpp
#define PATHSEP "\\"

void StringUtils::replace_all(string& s, const string& sToFind, const string& sReplace)
{
	if (sToFind.empty())
		return;

	size_t iReplaceLen = sReplace.length();
	for (size_t loc = s.find( sToFind ); loc != wstring::npos; loc = s.find( sToFind, loc+iReplaceLen ) )
	{
		s.replace( loc, sToFind.length(), sReplace );
	}

} 

//Taken from par2-cmdline\diskfile.cpp
void StringUtils::SplitFilename(string filename, string &path, string &name)
{
	string::size_type where;

	if (string::npos != (where = filename.find_last_of('/')) ||
		string::npos != (where = filename.find_last_of('\\')))
	{
		path = filename.substr(0, where+1);
		name = filename.substr(where+1);
	}
	else
	{
		path = "." PATHSEP;
		name = filename;
	}
}

string StringUtils::FileExtension(string filename)
{
	string sExt;
	if (filename.find_last_of('.') != string::npos)
		sExt = filename.substr(filename.find_last_of('.')+1);
	else
		sExt = "";

	transform(sExt.begin(), sExt.end(), sExt.begin(), toupper);
	return sExt;
}

void StringUtils::trim(string &str, const char *delims, bool left, bool right)
{
	if(left)
		str.erase(0, str.find_first_not_of(delims));
	if(right)
		str.erase(str.find_last_not_of(delims) + 1);
}

bool StringUtils::nocase_compare (string s1, string s2)
{
	string s1Upper = s1;
	string s2Upper = s2;
	transform(s1Upper.begin(), s1Upper.end(), s1Upper.begin(), toupper);
	transform(s2Upper.begin(), s2Upper.end(), s2Upper.begin(), toupper);

    return s1Upper < s2Upper;
}

//Returns true if string s is found in vector v
bool StringUtils::find(set<string> &v, string s, bool bIgnoreCase)
{
	set<string>::iterator iter = v.begin();	
	string sUpper = s;
	transform(sUpper.begin(), sUpper.end(), sUpper.begin(), toupper);

	while (iter != v.end())
	{
		string sTemp = (*iter);

		if (bIgnoreCase)
		{
			transform(sTemp.begin(), sTemp.end(), sTemp.begin(), toupper);
	
			if (sTemp == sUpper)
				return true;
		}
		++iter;
	}
	return false;
}