#ifndef STRB_H
#define STRB_H

#include <stddef.h>

#define STR_SIZEOF(len) (sizeof(char)*(len))

typedef enum
{
	STRB_NO_ERR,
	STRB_NULL_ERR,
	STRB_LEN_ERR,
	STRB_RESIZE_FAILED,
	STRB_NEW_FAILED,
} strb_err_code;

typedef struct
{
	char* str;
	size_t length;
} strb_t;

strb_t* strb_new(const char* str);
void strb_free(strb_t* buffer);

strb_err_code strb_cat(strb_t* dest, const char* src);
strb_err_code strb_ncat(strb_t* dest, const char* src, size_t n);

strb_err_code strb_precat(strb_t* dest, const char* src);
strb_err_code strb_prencat(strb_t* dest, const char* src, size_t n);

strb_err_code strb_cat_buffer(strb_t* dest, strb_t* src);
strb_err_code strb_ncat_buffer(strb_t* dest, strb_t* src, size_t n);

strb_err_code strb_precat_buffer(strb_t* dest, strb_t* src);
strb_err_code strb_prencat_buffer(strb_t* dest, strb_t* src, size_t n);

int strb_strlen(strb_t* buffer);
int strb_index_of(strb_t* buffer, int c);

#endif