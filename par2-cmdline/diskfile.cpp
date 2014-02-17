//  This file is part of par2cmdline (a PAR 2.0 compatible file verification and
//  repair tool). See http://parchive.sourceforge.net for details of PAR 2.0.
//
//  Copyright (c) 2003 Peter Brian Clements
//
//  par2cmdline is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  par2cmdline is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//	11/1/05 gmilow - Modified 

#include "stdafx.h"
#include "par2cmdline.h"
#include "../PnrMessage.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

#define OffsetType __int64
#define MaxOffset 0x7fffffffffffffffI64
#define LengthType unsigned int
#define MaxLength 0xffffffffUL

DiskFile::DiskFile(string sParfileName)
{
	m_sParfileName = sParfileName;
	filename;
	filesize = 0;
	offset = 0;
	hFile = INVALID_HANDLE_VALUE;
	exists = false;
}

DiskFile::~DiskFile(void)
{
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
}

// Create new file on disk and make sure that there is enough
// space on disk for it.
bool DiskFile::Create(string _filename, u64 _filesize)
{
	ostringstream strm;

	assert(hFile == INVALID_HANDLE_VALUE);

	filename = _filename;
	filesize = _filesize;

	// Create the file
	hFile = ::CreateFileA(_filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD error = ::GetLastError();

		strm.str(""); strm << "Could not create \"" << _filename << "\": " << CParNRarApp::ErrorMessage(error);
		CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
		return false;
	}

	if (filesize > 0)
	{
		// Seek to the end of the file
		LONG lowoffset = ((LONG*)&filesize)[0];
		LONG highoffset = ((LONG*)&filesize)[1];

		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, lowoffset, &highoffset, FILE_BEGIN))
		{
			DWORD error = ::GetLastError();

			strm.str(""); strm << "Could not set size of \"" << _filename << "\": " << CParNRarApp::ErrorMessage(error);
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

			::CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			::DeleteFile(_filename.c_str());

			return false;
		}

		// Set the end of the file
		if (!::SetEndOfFile(hFile))
		{
			DWORD error = ::GetLastError();

			strm.str(""); strm << "Could not set size of \"" << _filename << "\": " << CParNRarApp::ErrorMessage(error);
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

			::CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			::DeleteFile(_filename.c_str());

			return false;
		}
	}

	offset = filesize;

	exists = true;
	return true;
}

// Write some data to disk

bool DiskFile::Write(u64 _offset, const void *buffer, size_t length)
{
	ostringstream strm;
	assert(hFile != INVALID_HANDLE_VALUE);

	while (1)
	{
		if (offset != _offset)
		{
			LONG lowoffset = ((LONG*)&_offset)[0];
			LONG highoffset = ((LONG*)&_offset)[1];

			// Seek to the required offset
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, lowoffset, &highoffset, FILE_BEGIN))
			{
				DWORD error = ::GetLastError();
				if (error == ERROR_DISK_FULL)
				{
					CPnrMessage::SendPause("Disk space low. Free up more space and unpause Par-N-Rar.");
					continue;
				}

				strm.str(""); strm <<  "Could not write " << (u64)length << " bytes to \"" << filename << "\" at offset " << _offset << ": " << CParNRarApp::ErrorMessage(error);
				CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

				return false;
			}
			offset = _offset;
		}

		if (length > MaxLength)
		{
			strm.str(""); strm << "Could not write " << (u64)length << " bytes to \"" << filename << "\" at offset " << _offset << ": " << "Write too long";
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

			return false;
		}

		DWORD write = (LengthType)length;
		DWORD wrote;

		// Write the data
		if (!::WriteFile(hFile, buffer, write, &wrote, NULL))
		{
			DWORD error = ::GetLastError();
			if (error == ERROR_DISK_FULL)
			{
				CPnrMessage::SendPause("Disk space low. Free up more space and unpause Par-N-Rar.");
				continue;
			}

			strm.str(""); strm << "Could not write " << (u64)length << " bytes to \"" << filename << "\" at offset " << _offset << ": " << CParNRarApp::ErrorMessage(error);
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

			return false;
		}
		break;
	}
	offset += length;

	if (filesize < offset)
	{
		filesize = offset;
	}

	return true;
}

// Open the file

bool DiskFile::Open(string _filename, u64 _filesize)
{
	ostringstream strm;
	assert(hFile == INVALID_HANDLE_VALUE);

	filename = _filename;
	filesize = _filesize;

	hFile = ::CreateFileA(_filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD error = ::GetLastError();

		switch (error)
		{
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			break;
		default:
			strm.str(""); strm << "Could not open \"" << _filename << "\": " << CParNRarApp::ErrorMessage(error);
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
		}

		return false;
	}

	offset = 0;
	exists = true;

	return true;
}

// Read some data from disk

bool DiskFile::Read(u64 _offset, void *buffer, size_t length)
{
	ostringstream strm;
	assert(hFile != INVALID_HANDLE_VALUE);

	if (offset != _offset)
	{
		LONG lowoffset = ((LONG*)&_offset)[0];
		LONG highoffset = ((LONG*)&_offset)[1];

		// Seek to the required offset
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, lowoffset, &highoffset, FILE_BEGIN))
		{
			DWORD error = ::GetLastError();

			strm.str(""); strm << "Could not read " << (u64)length << " bytes from \"" << filename << "\" at offset " << _offset << ": " << CParNRarApp::ErrorMessage(error);
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

			return false;
		}
		offset = _offset;
	}

	if (length > MaxLength)
	{
		strm.str(""); strm << "Could not read " << (u64)length << " bytes from \"" << filename << "\" at offset " << _offset << ": " << "Read too long";
		CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

		return false;
	}

	DWORD want = (LengthType)length;
	DWORD got;

	// Read the data
	if (!::ReadFile(hFile, buffer, want, &got, NULL))
	{
		DWORD error = ::GetLastError();

		strm.str(""); strm << "Could not read " << (u64)length << " bytes from \"" << filename << "\" at offset " << _offset << ": " << CParNRarApp::ErrorMessage(error);
		CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

		return false;
	}

	offset += length;

	return true;
}

void DiskFile::Close(void)
{
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
}

string DiskFile::GetCanonicalPathname(string filename)
{
	char fullname[MAX_PATH];
	char *filepart;

	// Resolve a relative path to a full path
	int length = ::GetFullPathName(filename.c_str(), sizeof(fullname), fullname, &filepart);
	if (length <= 0 || sizeof(fullname) < length)
		return filename;

	// Make sure the drive letter is upper case.
	fullname[0] = toupper(fullname[0]);

	// Translate all /'s to \'s
	char *current = strchr(fullname, '/');
	while (current)
	{
		*current++ = '\\';
		current  = strchr(current, '/');
	}

	// Copy the root directory to the output string
	string longname(fullname, 3);

	// Start processing at the first path component
	current = &fullname[3];
	char *limit = &fullname[length];

	// Process until we reach the end of the full name
	while (current < limit)
	{
		char *tail;

		// Find the next \, or the end of the string
		(tail = strchr(current, '\\')) || (tail = limit);
		*tail = 0;

		// Create a wildcard to search for the path
		string wild = longname + current;
		WIN32_FIND_DATA finddata;
		HANDLE hFind = ::FindFirstFile(wild.c_str(), &finddata);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			// If the component was not found then just copy the rest of the path to the
			// output buffer verbatim.
			longname += current;
			break;
		}
		::FindClose(hFind);

		// Copy the component found to the output
		longname += finddata.cFileName;

		current = tail + 1;

		// If we have not reached the end of the name, add a "\"
		if (current < limit)
			longname += '\\';
	}

	return longname;
}

list<string>* DiskFile::FindFiles(string path, string wildcard)
{
	list<string> *matches = new list<string>;

	wildcard = path + wildcard;
	WIN32_FIND_DATA fd;
	HANDLE h = ::FindFirstFile(wildcard.c_str(), &fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (0 == (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				matches->push_back(path + fd.cFileName);
			}
		} while (::FindNextFile(h, &fd));
		::FindClose(h);
	}

	return matches;
}




























bool DiskFile::Open(void)
{
	string _filename = filename;

	return Open(_filename);
}

bool DiskFile::Open(string _filename)
{
	return Open(_filename, GetFileSize(_filename));
}









// Delete the file

bool DiskFile::Delete(void)
{
	ostringstream strm;
#ifdef WIN32
	assert(hFile == INVALID_HANDLE_VALUE);
#else
	assert(file == 0);
#endif

	if (filename.size() > 0 && 0 == unlink(filename.c_str()))
	{
		return true;
	}
	else
	{
		strm.str(""); strm << "Cannot delete " << filename;
		CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

		return false;
	}
}









//string DiskFile::GetPathFromFilename(string filename)
//{
//  string::size_type where;
//
//  if (string::npos != (where = filename.find_last_of('/')) ||
//      string::npos != (where = filename.find_last_of('\\')))
//  {
//    return filename.substr(0, where+1);
//  }
//  else
//  {
//    return "." PATHSEP;
//  }
//}

void DiskFile::SplitFilename(string filename, string &path, string &name)
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

bool DiskFile::FileExists(string filename)
{
	struct stat st;
	return ((0 == stat(filename.c_str(), &st)) && (0 != (st.st_mode & S_IFREG)));
}

u64 DiskFile::GetFileSize(string filename)
{
	struct stat st;
	if ((0 == stat(filename.c_str(), &st)) && (0 != (st.st_mode & S_IFREG)))
	{
		return st.st_size;
	}
	else
	{
		return 0;
	}
}



// Take a filename from a PAR2 file and replace any characters
// which would be illegal for a file on disk
string DiskFile::TranslateFilename(string filename)
{
	string result;

	string::iterator p = filename.begin();
	while (p != filename.end())
	{
		unsigned char ch = *p;

		bool ok = true;
#ifdef WIN32
		if (ch < 32)
		{
			ok = false;
		}
		else
		{
			switch (ch)
			{
			case '"':
			case '*':
			case '/':
			case ':':
			case '<':
			case '>':
			case '?':
			case '\\':
			case '|':
				ok = false;
			}
		}
#else
		if (ch < 32)
		{
			ok = false;
		}
		else
		{
			switch (ch)
			{
			case '/':
				ok = false;
			}
		}
#endif


		if (ok)
		{
			result += ch;
		}
		else
		{
			// convert problem characters to hex
			result += ((ch >> 4) < 10) ? (ch >> 4) + '0' : (ch >> 4) + 'A'-10;
			result += ((ch & 0xf) < 10) ? (ch & 0xf) + '0' : (ch & 0xf) + 'A'-10;
		}

		++p;
	}

	return result;
}

bool DiskFile::Rename(void)
{
	ostringstream strm;
	char newname[_MAX_PATH+1];
	u32 index = 0;

	struct stat st;

	do
	{
		int length = snprintf(newname, _MAX_PATH, "%s.%d", filename.c_str(), ++index);
		if (length < 0)
		{
			strm.str(""); strm << filename << " cannot be renamed.";
			CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);
			return false;
		}
		newname[length] = 0;
	} while (stat(newname, &st) == 0);

	return Rename(newname);
}

bool DiskFile::Rename(string _filename)
{
#ifdef WIN32
	assert(hFile == INVALID_HANDLE_VALUE);
#else
	assert(file == 0);
#endif
	ostringstream strm;
	if (::rename(filename.c_str(), _filename.c_str()) == 0)
	{
		filename = _filename;

		return true;
	}
	else
	{
		strm.str(""); strm << filename << " cannot be renamed to " << _filename;
		CPnrMessage::SendParAddDetails(m_sParfileName, strm.str(), true);

		return false;
	}
}

DiskFileMap::DiskFileMap(void)
{
}

DiskFileMap::~DiskFileMap(void)
{
	map<string, DiskFile*>::iterator fi = diskfilemap.begin();
	while (fi != diskfilemap.end())
	{
		delete (*fi).second;

		++fi;
	}
}

bool DiskFileMap::Insert(DiskFile *diskfile)
{
	string filename = diskfile->FileName();
	assert(filename.length() != 0);

	DiskFile *pDiskfile = Find(filename);
	if (pDiskfile != NULL)
		Remove(pDiskfile);

	pair<map<string,DiskFile*>::const_iterator,bool> location = diskfilemap.insert(pair<string,DiskFile*>(filename, diskfile));

	return location.second;
}

void DiskFileMap::Remove(DiskFile *diskfile)
{
	string filename = diskfile->FileName();
	assert(filename.length() != 0);
	diskfilemap.erase(filename);
}

DiskFile* DiskFileMap::Find(string filename) const
{
	assert(filename.length() != 0);

	map<string, DiskFile*>::const_iterator f = diskfilemap.find(filename);

	return (f != diskfilemap.end()) ?  f->second : 0;
}
