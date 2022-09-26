#pragma once

#include <string>

#if defined(WIN32)
#pragma comment(lib, "strmiids.lib")
#endif

#if 1
#ifdef LIBUVCCAM_EXPORTS
#   define LIBUVCCAM_API __declspec(dllexport)
#else
#   ifdef LIBUVCCAM_STATIC
#       define LIBUVCCAM_API 
#   else
#       define LIBUVCCAM_API __declspec(dllimport)
#   endif // LIBUVCCAM_STATIC
#endif
#else
#   define LIBUVCCAM_API
#endif // 0

#ifndef TString
#   ifdef UNICODE
#       define TString     std::wstring
#   else
#       define TString     std::string
#   endif // UNICODE
#endif // !TString

#ifndef TChar
#   ifdef UNICODE
#       define TChar       wchar_t
#   else
#       define TChar       char
#   endif // UNICODE
#endif // !TChar

#ifndef _T
#   ifdef UNICODE
#       define _T(x)       L##x
#   else
#       define _T(x)       x
#   endif // UNICODE
#endif // !_T

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...)     _tprintf(fmt, ## __VA_ARGS__)
#define LOG_INFO(fmt, ...)      _tprintf(fmt, ## __VA_ARGS__)
#define LOG_WARN(fmt, ...)      _tprintf(fmt, ## __VA_ARGS__)
#define LOG_ERROR(fmt, ...)     _tprintf(fmt, ## __VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_ERROR(fmt, ...)
#endif // _DEBUG

