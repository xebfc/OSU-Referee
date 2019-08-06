#include <wchar.h>
#include <stdlib.h>
#include <glib.h>

#include "wcsb.h"

#define WCS_MALLOC(len) ( \
    (wchar_t*)malloc(WCS_SIZEOF(len)) \
)

wchar_t* filename_from_utf8(char* str)
{
    wchar_t* wcs;
    int len;

#ifdef _WIN32

    len = _MULTI_BYTE_TO_WIDE_CHAR(CP_UTF8, str, NULL, 0); // 已经包含字符结尾符'\0'
    wcs = WCS_MALLOC(len);
    _MULTI_BYTE_TO_WIDE_CHAR(CP_UTF8, str, wcs, len);

#else

    gchar* ptr = _G_FILENAME_FROM_UTF8(str);

    len = mbstowcs(NULL, ptr, 0) + 1;
    wcs = WCS_MALLOC(len);
    mbstowcs(wcs, ptr, len);

    g_free(ptr);

#endif

    return wcs;
}

int indexof_wc(const wchar_t* ws, wchar_t wc)
{
    wchar_t* ptr = wcschr(ws, wc);
    return ptr == NULL ? -1 : ptr - ws;
}

int indexof_ws(const wchar_t* ws1, const wchar_t* ws2)
{
    wchar_t* ptr = wcsstr(ws1, ws2);
    return ptr == NULL ? -1 : ptr - ws1;
}

int last_indexof_wc(const wchar_t* ws, wchar_t wc)
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