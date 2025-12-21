#pragma once

#include <Windows.h>

DWORD GetCRC32(const char* buffer, size_t count);
DWORD GetCaseCRC32(const char* buf, size_t len);
DWORD GetHFILECRC32(HANDLE hFile);
DWORD GetFileCRC32(const char* c_szFileName);
DWORD GetFileSize(const char* c_szFileName);
