//============================================================
//
//  main.c - Win32 main program
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard windows headers
#define _WIN32_WINNT 0x0400
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>

// MAMEOS headers
#include "strconv.h"

extern int utf8_main(int argc, char *argv[]);



//============================================================
//  main
//============================================================

// undo the command-line #define that maps main to utf8_main in all other cases
#ifndef WINUI
#undef main
#undef wmain
#endif

extern "C" int _tmain(int argc, TCHAR **argv)
{
	int i, rc;
	char **utf8_argv;

#ifdef MALLOC_DEBUG
{
	extern int winalloc_in_main_code;
	winalloc_in_main_code = TRUE;
#endif

	/* convert arguments to UTF-8 */
    /*为传入的参数分配了恰好的内存空间,并且强制转换成了二级指针，并且看到传入的argv参数就是
     * 一个二级指针,也就是说为一个指针变量申请了空间，每个空间中存储的还是指针*/
	utf8_argv = (char **) malloc(argc * sizeof(*argv));
	if (utf8_argv == NULL)
		return 999;// 如果分配得到的指针是空指针，返回错误999,函数结束
	for (i = 0; i < argc; i++)
	{
        //调用函数来自unicode文件.这里对输入的每个字符，进行逐一的转换.
		utf8_argv[i] = utf8_from_tstring(argv[i]);
		if (utf8_argv[i] == NULL)
			return 999; //测试转换是否成功，如果转换得来的是空指针则返回错误代码999，函数结束
	}

	/* run utf8_main */
    //已经完成了参数转换，这次调用另外一个函数，这个函数来自osd/sdl/testkeys.c
	rc = utf8_main(argc, utf8_argv);

	/* free arguments */
	for (i = 0; i < argc; i++)
		osd_free(utf8_argv[i]); //osd_free掉了申请空间中的每个指针所指向的空间
	free(utf8_argv);//最后释放了存贮这些指针所在的空间

#ifdef MALLOC_DEBUG
	{
		void check_unfreed_mem(void);
		check_unfreed_mem();
	}
	winalloc_in_main_code = FALSE;
}
#endif

	return rc; //返回rc的值
}
