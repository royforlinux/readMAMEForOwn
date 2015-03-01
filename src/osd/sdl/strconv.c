//============================================================
//
//  strconv.c - SDL (POSIX) string conversion
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdlib.h>

// MAMEOS headers
#include "strconv.h"
#include "unicode.h"

#ifdef SDLMAME_WIN32
//============================================================
//  utf8_from_astring
//============================================================
/*
 * 这个函数是在定义宏SDLMAME_WIN32时才被包含编译的，所以在这个函数是用大量的windows.h的API和宏定义
 * 目前（ 2015年 3月 1日 星期日 11时13分06秒 CST ）发现如下，WCHAR 、CHAR 、MultiByteToWideChar
 */
char *utf8_from_astring(const CHAR *astring)
{
	WCHAR *wstring;
	int char_count;
	CHAR *result;
    /*
     * 看来这个MultiByteToWideChar函数并不是那么有用，只能先从本地的ANSI转换到UTF-16，然后才能
     * 转换到UTF-8
     */
	// convert "ANSI code page" string to UTF-16
	char_count = MultiByteToWideChar(CP_ACP, 0, astring, -1, NULL, 0);
	wstring = (WCHAR *)alloca(char_count * sizeof(*wstring));
	MultiByteToWideChar(CP_ACP, 0, astring, -1, wstring, char_count);

	// convert UTF-16 to MAME string (UTF-8)
	char_count = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL);
	result = (CHAR *)osd_malloc_array(char_count * sizeof(*result));
	if (result != NULL)
		WideCharToMultiByte(CP_UTF8, 0, wstring, -1, result, char_count, NULL, NULL);

	return result;
}

//============================================================
//  utf8_from_wstring
//============================================================

char *utf8_from_wstring(const WCHAR *wstring)
{
	int char_count;
	char *result;

	// convert UTF-16 to MAME string (UTF-8)
	char_count = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL);
	result = (char *)osd_malloc_array(char_count * sizeof(*result));
	if (result != NULL)
		WideCharToMultiByte(CP_UTF8, 0, wstring, -1, result, char_count, NULL, NULL);

	return result;
}
#endif

//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(unicode_char *uchar, const char *osdchar, size_t count)
    /*
     * unicode_char在unicode.h文件中被宏定义 UNIT32替换
     * 使用了mbstows函数作为主要的转换函数
     * 本函数输入了三个参数，其中的osdchar是被转换的字符，只不过传入的count是不是应该总为-1
     * 以作为检测的标志，在利用mbstows函数转换之后，又使用count作为判断标志，在用uchar的指针返回
     * 结果，那么uchar的指针是否应该指向一个宽字符型的指针数组？
     * 最后把count作为返回值，用来检测是否成功转换？（2015年3月1日星期日12时43分30秒CST）
     */
{
	wchar_t wch;

	count = mbstowcs(&wch, (char *)osdchar, 1);
	if (count != -1)
		*uchar = wch;
	else
		*uchar = 0;

	return count;
}
