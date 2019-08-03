#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "strb.h"

typedef void (*save_t)(strb_t* dest, const char* src, size_t n, size_t old_size, size_t new_size);

strb_t* strb_new(const char* str)
{
    strb_t* buffer = NULL;
    buffer = (strb_t*)malloc(sizeof(*buffer)); // sizeof(strb_t)
    if (buffer == NULL)
        goto err;

    if (str == NULL)
        buffer->length = 1;
    else
        buffer->length = strlen(str) + 1; // 字符串以'\0'结尾，实际长度 = strlen(str) + 1

    buffer->str = STR_MALLOC(buffer->length);
    if (buffer->str == NULL)
        goto err;

    if (str == NULL)
        memcpy(buffer->str, "", STR_SIZEOF(buffer->length));
    else
        memcpy(buffer->str, str, STR_SIZEOF(buffer->length));

    return buffer;

err:
    strb_free(buffer);
    return NULL;
}

void strb_free(strb_t* buffer)
{
    if (buffer == NULL)
        return;

    if (buffer->str != NULL)
    {
        free(buffer->str);
        buffer->str = NULL;
    }

    free(buffer);
}

static void _after(strb_t* dest, const char* src, size_t n, size_t old_size, size_t new_size)
{
    memmove(dest->str + n, dest->str, STR_SIZEOF(n));
    memcpy(dest->str, src, STR_SIZEOF(n));
    *(dest->str + new_size - 1) = '\0';
}

static void _before(strb_t* dest, const char* src, size_t n, size_t old_size, size_t new_size)
{
    memcpy(dest->str + (old_size - 1), src, STR_SIZEOF(n));
    *(dest->str + new_size - 1) = '\0';
}

static int _resize(strb_t* buffer, size_t new_size)
{
    if (buffer == NULL)
        goto err;

    char* ptr = (char*)realloc(buffer->str, STR_SIZEOF(new_size));

    if (ptr == NULL)
        goto err;

    buffer->str = ptr;
    buffer->length = new_size;

    return 0;

err:
    return 1;
}

static strb_err_code _cat(strb_t* dest, const char* src, size_t n, save_t func)
{
    if (src == NULL)
        return STRB_NULL_ERR;

    if (n > strlen(src))
        return STRB_LEN_ERR;

    if (dest == NULL)
        //dest = strb_new(NULL);
        return STRB_NEW_FAILED;

    int old_size = dest->length,
        new_size = old_size + n;

    if (_resize(dest, new_size))
        return STRB_RESIZE_FAILED;

    func(dest, src, n, old_size, new_size);

    return STRB_NO_ERR;
}

strb_err_code strb_cat(strb_t* dest, const char* src)
{
    return  _cat(dest, src, strlen(src), _before);
}

strb_err_code strb_ncat(strb_t* dest, const char* src, size_t n)
{
    return _cat(dest, src, n, _before);
}

strb_err_code strb_precat(strb_t* dest, const char* src)
{
    return _cat(dest, src, strlen(src), _after);
}

strb_err_code strb_prencat(strb_t* dest, const char* src, size_t n)
{
    return _cat(dest, src, n, _after);
}

strb_err_code strb_cat_buffer(strb_t* dest, strb_t* src)
{
    return _cat(dest, src->str, src->length - 1, _before);
}

strb_err_code strb_ncat_buffer(strb_t* dest, strb_t* src, size_t n)
{
    return _cat(dest, src->str, n, _before);
}

strb_err_code strb_precat_buffer(strb_t* dest, strb_t* src)
{
    return _cat(dest, src->str, src->length - 1, _after);
}

strb_err_code strb_prencat_buffer(strb_t* dest, strb_t* src, size_t n)
{
    return _cat(dest, src->str, n, _after);
}

int strb_strlen(strb_t* buffer)
{
    if (buffer == NULL)
        return -1;

    return strlen(buffer->str);
}

int strb_indexof_char(strb_t* buffer, int c)
{
    if (buffer == NULL)
        return -1;

    return indexof_char(buffer->str, c);
}

int strb_indexof_str(strb_t* buffer, const char* str)
{
    if (buffer == NULL)
        return -1;

    return indexof_str(buffer->str, str);
}

int strb_indexof_strb(strb_t* b1, strb_t* b2)
{
    if (b1 == NULL || b2 == NULL)
        return -1;

    return indexof_str(b1->str, b2->str);
}

int strb_last_indexof_char(strb_t* buffer, int c)
{
    if (buffer == NULL)
        return -1;

    return last_indexof_char(buffer->str, c);
}

strb_err_code strb_sprintf(strb_t* buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    strb_err_code err_code = strb_vsprintf(buffer, format, args);

    va_end(args);
    return err_code;
}

strb_err_code strb_append_sprintf(strb_t* buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    strb_err_code err_code = strb_append_vsprintf(buffer, format, args);

    va_end(args);
    return err_code;
}

strb_err_code strb_vsprintf(strb_t* buffer, const char* format, va_list args)
{
    if (buffer == NULL)
        //buffer = strb_new(NULL);
        return STRB_NEW_FAILED;

    int len = vsnprintf(NULL, 0, format, args),
        new_size = len + 1;

    if (_resize(buffer, new_size))
        return STRB_RESIZE_FAILED;

    vsnprintf(buffer->str, new_size, format, args);

    return STRB_NO_ERR;
}

strb_err_code strb_append_vsprintf(strb_t* buffer, const char* format, va_list args)
{
    if (buffer == NULL)
        //buffer = strb_new(NULL);
        return STRB_NEW_FAILED;

    int len = vsnprintf(NULL, 0, format, args),
        old_size = buffer->length;

    if (_resize(buffer, old_size + len))
        return STRB_RESIZE_FAILED;

    vsnprintf(buffer->str + old_size - 1, len + 1, format, args);

    return STRB_NO_ERR;
}

//-------------------------------------------------------------------------------

int indexof_char(const char* str, int c)
{
    char* ptr = strchr(str, c);
    return ptr == NULL ? -1 : ptr - str;
}

int indexof_str(const char* str1, const char* str2)
{
    char* ptr = strstr(str1, str2);
    return ptr == NULL ? -1 : ptr - str1;
}

int last_indexof_char(const char* str, int c)
{
    char* ptr = strrchr(str, c);
    return ptr == NULL ? -1 : ptr - str;
}

char* substr(const char* str, int beginIndex, int endIndex)
{
    int len = endIndex - beginIndex;
    char* ptr = STR_MALLOC(len + 1);
    if (ptr == NULL)
        return NULL;

    memcpy(ptr, str + beginIndex, STR_SIZEOF(len));
    *(ptr + len) = '\0';
    return ptr;
}