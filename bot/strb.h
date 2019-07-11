#ifndef STRB_H
#define STRB_H

#include <stddef.h>
#include <stdarg.h>

#define STR_SIZEOF(len) (sizeof(char)*(len))

typedef enum
{
	STRB_NO_ERR,
	STRB_NULL_ERR,
	STRB_LEN_ERR,
	STRB_RESIZE_FAILED,
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

int strb_index_of_char(strb_t* buffer, int c);
int strb_index_of_str(strb_t* buffer, const char* str);
int strb_index_of_strb(strb_t* b1, strb_t* b2);

int strb_last_index_of_char(strb_t* buffer, int c);

strb_err_code strb_sprintf(strb_t* buffer, const char* format, ...);
strb_err_code strb_append_sprintf(strb_t* buffer, const char* format, ...);

strb_err_code strb_vsprintf(strb_t* buffer, const char* format, va_list args);
strb_err_code strb_append_vsprintf(strb_t* buffer, const char* format, va_list args);

//-------------------------------------------------------------------------------

int index_of_char(const char* str, int c);
int index_of_str(const char* str1, const char* str2);

int last_index_of_char(const char* str, int c);

#endif