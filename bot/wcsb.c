#include <wchar.h>
#include <stdlib.h>

#include "wcsb.h"

#define WCS_MALLOC(len) ((wchar_t*)malloc(WCS_SIZEOF(len)))

wchar_t* from_utf8(char* str)
{
	wchar_t* wcs;
	int len;

#ifdef _WIN32

	len = _MULTI_BYTE_TO_WIDE_CHAR(CP_UTF8, str, NULL, 0); // 已经包含字符结尾符'\0'
	wcs = WCS_MALLOC(len);
	_MULTI_BYTE_TO_WIDE_CHAR(CP_UTF8, str, wcs, len);

#else

	char* ptr = _G_LOCALE_FROM_UTF8(str);

	len = mbstowcs(NULL, ptr, 0) + 1;
	wcs = WCS_MALLOC(len);
	mbstowcs(wcs, ptr, len);

	free(ptr);

#endif

	return wcs;
}

int index_of_wc(const wchar_t* ws, wchar_t wc)
{
	wchar_t* ptr = wcschr(ws, wc);
	return ptr == NULL ? -1 : ptr - ws;
}

int index_of_ws(const wchar_t* ws1, const wchar_t* ws2)
{
	wchar_t* ptr = wcsstr(ws1, ws2);
	return ptr == NULL ? -1 : ptr - ws1;
}

int last_index_of_wc(const wchar_t* ws, wchar_t wc)
{
	wchar_t* ptr = wcsrchr(ws, wc);
	return ptr == NULL ? -1 : ptr - ws;
}

wchar_t* subws(const wchar_t* ws, int beginIndex, int endIndex)
{
	int len = endIndex - beginIndex;
	wchar_t* ptr = WCS_MALLOC(len + 1);
	if (ptr == NULL)
		return NULL;

	memcpy(ptr, ws + beginIndex, WCS_SIZEOF(len));
	*(ptr + len) = L'\0';
	return ptr;
}