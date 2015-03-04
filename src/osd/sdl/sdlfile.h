//============================================================
//
//  sdlfile.h - SDL file access functions
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "osdcore.h"
#include "sdlos.h"

//============================================================
//  ENUM DEFINITIONS
//============================================================
/*
 * 没有定义枚举类型名称的枚举类型。如何应用？(2015年 3月 3日 星期二 09时18分48秒 CST)
 */
enum
{
	SDLFILE_FILE = 0,
	SDLFILE_SOCKET,
	SDLFILE_PTTY
};

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_file
{
	int handle;
	int socket;
	int type;
	char    filename[1];
};

//============================================================
//  PROTOTYPES
//============================================================

bool sdl_check_socket_path(const char *path);
/* 
 * file_error是一个枚举类型，定义在osdcore.h中，下面对已供两组八个函数的原型
 * 打开、读取、写入、关闭嵌套字，打开、读取、写入、关闭ptty，还有一个错误转换的函数
 */
file_error sdl_open_socket(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize);
file_error sdl_read_socket(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error sdl_write_socket(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error sdl_close_socket(osd_file *file);

file_error sdl_open_ptty(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize);
file_error sdl_read_ptty(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error sdl_write_ptty(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual);
file_error sdl_close_ptty(osd_file *file);

file_error error_to_file_error(UINT32 error);
