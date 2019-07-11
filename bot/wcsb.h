#ifndef WCSB_H
#define WCSB_H

#include <wchar.h>
#include <windows.h>
#include <gmodule.h>
#include <stddef.h>

#define WCS_SIZEOF(len) (sizeof(wchar_t)*(len))
#define _MULTI_BYTE_TO_WIDE_CHAR(CodePage,lpMultiByteStr,lpWideCharStr,cchWideChar) (MultiByteToWideChar(CodePage,0,lpMultiByteStr,-1,lpWideCharStr,cchWideChar))
#define _WIDE_CHAR_TO_MULTI_BYTE(CodePage,lpWideCharStr,lpMultiByteStr,cchMultiByte) (WideCharToMultiByte(CodePage,0,lpWideCharStr,-1,lpMultiByteStr,cchMultiByte,NULL,NULL))
#define _G_LOCALE_TO_UTF8(opsysstring) (g_locale_to_utf8(opsysstring,-1,NULL,NULL,NULL))
#define _G_LOCALE_FROM_UTF8() (g_locale_from_utf8(utf8string,-1,NULL,NULL,NULL))
#define _G_FILENAME_FROM_UTF8(utf8string) (g_filename_from_utf8(utf8string,-1,NULL,NULL,NULL))

wchar_t* from_utf8(char* str);

typedef struct
{
	wchar_t* wcs;
	size_t length;
} wcsb_t;

#endif