#include <stdlib.h>

#include "wcsb.h"

#define WCS_MALLOC(len) ((wchar_t*)malloc(WCS_SIZEOF(len)))

wchar_t* from_utf8(char* str)
{
	wchar_t* wcs;
	int len;

#ifdef _WIN32

	len = _MULTI_BYTE_TO_WIDE_CHAR(CP_UTF8, str, NULL, 0);
	wcs = WCS_MALLOC(len);
	_MULTI_BYTE_TO_WIDE_CHAR(CP_UTF8, str, wcs, len);

#else

	char* fn = _G_FILENAME_FROM_UTF8(str);

#endif

	return wcs;
}