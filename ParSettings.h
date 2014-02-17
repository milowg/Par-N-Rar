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

#pragma once

class ParSettings : CObject
{
	DECLARE_SERIAL( ParSettings );
public:
	ParSettings(void)
	{
		m_bFileValidated = false;
	};

	~ParSettings(void){};

	void Serialize( CArchive &ar );
	bool m_bFileValidated;
	string m_sFile;
	string m_sFileNoExt;
	string m_sFilePath;
	string m_sFilePathNoExt;
	vector<string> m_vVerified;

	void SerializeVector(CArchive& ar, vector<string>& v);
	static const UINT m_uiClassVersion;

};
