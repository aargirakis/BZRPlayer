//	$Id: file.cpp,v 1.6 1999/12/28 11:14:05 cisc Exp $

#include "headers.h"
//#include "File.h"   //added by blazer for linux cross-compilation
#include "file.h"     //added by blazer for linux cross-compilation
#ifdef __amigaos4__
#include <proto/dos.h>
#endif

// ---------------------------------------------------------------------------
//	�\�z/����
// ---------------------------------------------------------------------------

FileIO::FileIO()
{
	flags = 0;
}

FileIO::FileIO(const char* filename, uint flg)
{
	flags = 0;
	Open(filename, flg);
}

FileIO::~FileIO()
{
	Close();
}

// ---------------------------------------------------------------------------
//	�t�@�C�����J��
// ---------------------------------------------------------------------------

bool FileIO::Open(const char* filename, uint flg)
{
	Close();

	strncpy(path, filename, MAX_PATH);

#ifndef __amigaos4__
	DWORD access = (flg & readonly ? 0 : GENERIC_WRITE) | GENERIC_READ;
	DWORD share = (flg & readonly) ? FILE_SHARE_READ : 0;
	DWORD creation = flg & create ? CREATE_ALWAYS : OPEN_EXISTING;

	hfile = CreateFile(filename, access, share, 0, creation, 0, 0);
#else
	DWORD mode = flg & create ? MODE_NEWFILE : MODE_OLDFILE;

	hfile = IDOS->Open(filename, mode);
#endif
	
	flags = (flg & readonly) | (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	if (!(flags & open))
	{
#ifndef __amigaos4__
		switch (GetLastError()) {
			case ERROR_FILE_NOT_FOUND:		error = file_not_found; break;
			case ERROR_SHARING_VIOLATION:	error = sharing_violation; break;
			default: error = unknown; break;
		}
#else
		switch (IDOS->IoErr()) {
			case ERROR_OBJECT_NOT_FOUND: error = file_not_found; break;
			default: error = unknown; break;
		}
#endif
	}
	SetLogicalOrigin(0);

	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	�t�@�C�����Ȃ��ꍇ�͍쐬
// ---------------------------------------------------------------------------

bool FileIO::CreateNew(const char* filename)
{
	Close();

	strncpy(path, filename, MAX_PATH);

#ifndef __amigaos4__
	DWORD access = GENERIC_WRITE | GENERIC_READ;
	DWORD share = 0;
	DWORD creation = CREATE_NEW;

	hfile = CreateFile(filename, access, share, 0, creation, 0, 0);
#else
	hfile = IDOS->Open(filename, MODE_NEWFILE);
#endif
	
	flags = (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	SetLogicalOrigin(0);

	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	�t�@�C������蒼��
// ---------------------------------------------------------------------------

bool FileIO::Reopen(uint flg)
{
	if (!(flags & open)) return false;
	if ((flags & readonly) && (flg & create)) return false;

	if (flags & readonly) flg |= readonly;

	Close();

#ifndef __amigaos4__
	DWORD access = (flg & readonly ? 0 : GENERIC_WRITE) | GENERIC_READ;
	DWORD share = flg & readonly ? FILE_SHARE_READ : 0;
	DWORD creation = flg & create ? CREATE_ALWAYS : OPEN_EXISTING;

	hfile = CreateFile(path, access, share, 0, creation, 0, 0);
#else
	DWORD mode = flg & create ? MODE_NEWFILE : MODE_OLDFILE;

	hfile = IDOS->Open(path, mode);
#endif
	
	flags = (flg & readonly) | (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	SetLogicalOrigin(0);

	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	�t�@�C�������
// ---------------------------------------------------------------------------

void FileIO::Close()
{
	if (GetFlags() & open)
	{
#ifndef __amigaos4__
		CloseHandle(hfile);
#else
		IDOS->Close(hfile);
#endif
		flags = 0;
	}
}

// ---------------------------------------------------------------------------
//	�t�@�C���k�̓ǂݏo��
// ---------------------------------------------------------------------------

int32 FileIO::Read(void* dest, int32 size)
{
	if (!(GetFlags() & open))
		return -1;
	
#ifndef __amigaos4__
	DWORD readsize;
	if (!ReadFile(hfile, dest, size, &readsize, 0))
		return -1;
	return readsize;
#else
	int32 readsize;
	readsize = IDOS->Read(hfile, dest, size);
	return readsize == -1 ? 0 : readsize;
#endif

}

// ---------------------------------------------------------------------------
//	�t�@�C���ւ̏����o��
// ---------------------------------------------------------------------------

int32 FileIO::Write(const void* dest, int32 size)
{
	if (!(GetFlags() & open) || (GetFlags() & readonly))
		return -1;
	
#ifndef __amigaos4__
	DWORD writtensize;
	if (!WriteFile(hfile, dest, size, &writtensize, 0))
		return -1;
	return writtensize;
#else
	int32 writtensize;
	writtensize = IDOS->Write(hfile, dest, size);
	return writtensize == -1 ? 0 : writtensize;
#endif
}

// ---------------------------------------------------------------------------
//	�t�@�C�����V�[�N
// ---------------------------------------------------------------------------

bool FileIO::Seek(int32 pos, SeekMethod method)
{
	if (!(GetFlags() & open))
		return false;
	
#ifndef __amigaos4__
	DWORD wmethod;
	switch (method)
	{
	case begin:	
		wmethod = FILE_BEGIN; pos += lorigin; 
		break;
	case current:	
		wmethod = FILE_CURRENT; 
		break;
	case end:		
		wmethod = FILE_END; 
		break;
	default:
		return false;
	}

	return 0xffffffff != SetFilePointer(hfile, pos, 0, wmethod);
#else
	int32 mode;
	switch (method) {
		case begin:
			mode = OFFSET_BEGINNING; pos += lorigin;
			break;
		case current:
			mode = OFFSET_CURRENT;
			break;
		case end:
			mode = OFFSET_END;
			break;
		default:
			return false;
	}
	return IDOS->ChangeFilePosition(hfile, pos, mode) != 0;
#endif
}

// ---------------------------------------------------------------------------
//	�t�@�C���̈ʒu�𓾂�
// ---------------------------------------------------------------------------

int32 FileIO::Tellp()
{
	if (!(GetFlags() & open))
		return 0;

#ifndef __amigaos4__
	return SetFilePointer(hfile, 0, 0, FILE_CURRENT) - lorigin;
#else
	return IDOS->GetFilePosition(hfile) - lorigin;
#endif
}

// ---------------------------------------------------------------------------
//	���݂̈ʒu���t�@�C���̏I�[�Ƃ���
// ---------------------------------------------------------------------------

bool FileIO::SetEndOfFile()
{
	if (!(GetFlags() & open))
		return false;
#ifndef __amigaos4__
	return ::SetEndOfFile(hfile) != 0;
#else
	return IDOS->ChangeFilePosition(hfile, 0, OFFSET_END) != 0;
#endif
}
