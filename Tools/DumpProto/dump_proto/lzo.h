#pragma once

#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>
#include "Singleton.h"

using BYTE = unsigned char;
using WORD = unsigned short;
using DWORD = unsigned long;
using UINT = unsigned int;

class CLZObject
{
    public:
#pragma pack(4)
        using THeader = struct SHeader
        {
            DWORD	dwFourCC;
            DWORD	dwEncryptSize;		// 암호화된 크기
            DWORD	dwCompressedSize;	// 압축된 데이터 크기
            DWORD	dwRealSize;		// 실제 데이터 크기
        };

#pragma pack()

        CLZObject();
        ~CLZObject();

        void		Clear();

        void		BeginCompress(const void* pvIn, UINT uiInLen);
        bool		Compress();

        bool		BeginDecompress(const void* pvIn);
        bool		Decompress(DWORD * pdwKey = NULL);

        bool		Encrypt(DWORD * pdwKey);
        BYTE* 		Decrypt(DWORD * pdwKey);

        const THeader& 	GetHeader()
        {
            return *m_pHeader;
        }

        BYTE* 		GetBuffer()
        {
            return m_pbBuffer;
        }

        DWORD		GetSize();

    private:
        void		Initialize();

        BYTE* 		m_pbBuffer;
        DWORD		m_dwBufferSize;

        THeader*		m_pHeader;
        const BYTE* 	m_pbIn;
        bool		m_bCompressed;

        static DWORD	ms_dwFourCC;
};

class CLZO : public singleton<CLZO>
{
    public:
        CLZO();
        virtual ~CLZO();

        bool	CompressMemory(CLZObject & rObj, const void* pIn, UINT uiInLen);
        bool	CompressEncryptedMemory(CLZObject & rObj, const void* pIn, UINT uiInLen, DWORD * pdwKey);
        bool	Decompress(CLZObject & rObj, const BYTE * pbBuf, DWORD * pdwKey = NULL);
        BYTE* 	GetWorkMemory();

    private:
        BYTE* 	m_pWorkMem;
};
