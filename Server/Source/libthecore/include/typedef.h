#pragma once

using QWORD = unsigned long;
using UBYTE = unsigned char;
using sbyte = signed char;
using sh_int = unsigned short;

struct timezone
{
    int     tz_minuteswest; /* minutes west of Greenwich */
    int     tz_dsttime;     /* type of dst correction */
};

using socket_t = SOCKET;

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifdef _WIN64
using ssize_t = __int64;
#else
using ssize_t = _W64 int;
#endif

// Fixed-size integer types
using int32_t = __int32;
using uint32_t = unsigned __int32;
using int64_t = __int64;
using uint64_t = unsigned __int64;

using uint = unsigned int;
