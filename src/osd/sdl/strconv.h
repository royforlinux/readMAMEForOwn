//============================================================
//
//  strconv.h - SDL string conversion
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __SDLSTRCONV__      
//只有在没有定义这个变量的情况下才包含以下的代码
#define __SDLSTRCONV__

#include "osdcore.h"



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

#ifdef SDLMAME_WIN32
/*
 * 只有在定义SDLMAME_WIN32情况下，才会包含以下片段，才有以下的函数的原型
 */

// the result of these functions has to be released with osd_free()
/*定义了两对互相转换字符宽度的函数*/
CHAR *astring_from_utf8(const char *s);
char *utf8_from_astring(const CHAR *s);

WCHAR *wstring_from_utf8(const char *s);
char *utf8_from_wstring(const WCHAR *s);

#ifdef UNICODE
/*
 * 这真是机智的替换方法啊，用统一的函数对——tstring_from_utf8及utf8_from_tstring进行调用，之后
 * 在对应到a版本（ASNI？）和w版本（宽字符？）
 */
#define tstring_from_utf8   wstring_from_utf8
#define utf8_from_tstring   utf8_from_wstring
#else // !UNICODE
#define tstring_from_utf8   astring_from_utf8
#define utf8_from_tstring   utf8_from_astring
#endif // UNICODE

#endif //SDLMAME_WIN32

#endif // __SDLSTRCONV__
