#include <stdlib.h>
#include <string.h>

#include "strb.h"

typedef void (*save_t)(strb_t* dest, const char* src, size_t n, size_t old_size);

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

	buffer->str = (char*)malloc(STR_SIZEOF(buffer->length));
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

static void _after(strb_t* dest, const char* src, size_t n, size_t old_size)
{
	memmove(dest->str + n, dest->str, STR_SIZEOF(n));
	memcpy(dest->str, src, STR_SIZEOF(n));
	*(dest->str + (old_size - 1) + n) = '\0';
}

static void _before(strb_t* dest, const char* src, size_t n, size_t old_size)
{
	memcpy(dest->str + (old_size - 1), src, STR_SIZEOF(n));
	*(dest->str + (old_size - 1) + n) = '\0';
}

static strb_err_code _cat(strb_t* dest, const char* src, size_t n, save_t fun)
{
	if (src == NULL)
		return STRB_NULL_ERR;

	if (n > strlen(src))
		return STRB_LEN_ERR;

	if (dest == NULL)
	{
		dest = strb_new(NULL);
		if (dest == NULL)
			return STRB_NEW_FAILED;
	}

	int old_size, new_size = dest->length + n;
	char* ptr = (char*)realloc(dest->str, STR_SIZEOF(new_size));
	if (ptr == NULL)
		return STRB_RESIZE_FAILED;
	dest->str = ptr;

	old_size = dest->length;
	dest->length = new_size;
	fun(dest, src, n, old_size);
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

int strb_index_of(strb_t* buffer, int c)
{
	char* ptr = strchr(buffer->str, c);
	return ptr - buffer->str;
}