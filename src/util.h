#pragma once

#include <string>
using namespace std;

#ifdef LIBUVCCAM_EXPORTS
#   define LIBUVCCAM_API __declspec(dllexport)
#else
#   ifdef LIBUVCCAM_STATIC
#       define LIBUVCCAM_API 
#   else
#       define LIBUVCCAM_API __declspec(dllimport)
#   endif // LIBUVCCAM_STATIC
#endif

#ifndef TString
#   ifdef UNICODE
#       define TString     wstring
#   else
#       define TString     string
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
