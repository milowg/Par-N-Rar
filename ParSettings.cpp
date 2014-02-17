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

#include "StdAfx.h"
#include ".\parsettings.h"

//v1.12 const UINT ParSettings::m_uiClassVersion = 2;
const UINT ParSettings::m_uiClassVersion = 1;

IMPLEMENT_SERIAL( ParSettings, CObject, ParSettings::m_uiClassVersion );

void ParSettings::Serialize( CArchive &ar )
{	
	CObject::Serialize( ar );

	if ( ar.IsStoring() )
	{			
		ar << m_uiClassVersion;		
		ar << m_bFileValidated;
		SerializeVector(ar, m_vVerified);	
	}
	else
	{	
		UINT uiFileVersion;
		ar >> uiFileVersion;

		if ( uiFileVersion == m_uiClassVersion )
		{
			ar >> m_bFileValidated;
			//v1.12 SerializeVector(ar, m_vVerified);	
		}
		else
		{
			AfxMessageBox( "Wrong settings file (.PNR) version!", MB_OK );
		}
	}
}

void ParSettings::SerializeVector(CArchive& ar, vector<string>& v)
{
	CString cstr;
	if (ar.IsStoring()) 
	{
		// Must cast to a DWORD because archives don't support integers
		ar << (DWORD)v.size();
		for (vector<string>::iterator i = v.begin(); i != v.end(); i++) 
		{
			cstr = (*i).c_str();
			ar << cstr;
		}
	}
	else 
	{
		DWORD size;
		ar >> size;

		string str;
		while (size--) 
		{
			ar >> cstr;
			string s = cstr.GetBuffer(0);
			v.push_back(s);
		}
	}
}
