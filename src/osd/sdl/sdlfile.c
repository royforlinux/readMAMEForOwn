//============================================================
//
//  sdlfile.c - SDL file access functions
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_WIN32
#include "../windows/winfile.c"
#include "../windows/winutil.c"
#else

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifdef SDLMAME_LINUX
#define __USE_LARGEFILE64
#endif

#ifdef SDLMAME_WIN32
#define _FILE_OFFSET_BITS 64
#endif

#ifndef SDLMAME_BSD
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 500
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

// MAME headers
#include "sdlfile.h"


//============================================================
//  GLOBAL IDENTIFIERS
//============================================================

extern const char *sdlfile_socket_identifier;
extern const char *sdlfile_ptty_identifier;

//============================================================
//  CONSTANTS
//============================================================

#if defined(SDLMAME_WIN32) || defined(SDLMAME_OS2)
#define PATHSEPCH '\\'
#define INVPATHSEPCH '/'
#else
#define PATHSEPCH '/'
#define INVPATHSEPCH '\\'
#endif

#define NO_ERROR    (0)

//============================================================
//  Prototypes
//============================================================

static UINT32 create_path_recursive(char *path);

//============================================================
//  error_to_file_error
//  (does filling this out on non-Windows make any sense?)
//============================================================

file_error error_to_file_error(UINT32 error)
    /*
     * 标准错误和自定义类型错误的转换
     * 返回的枚举类型元素在osdcore.h文件中有定义
     */
{
	switch (error)
	{
	case ENOENT:
	case ENOTDIR:
		return FILERR_NOT_FOUND;

	case EACCES:
	case EROFS:
	#ifndef SDLMAME_WIN32
	case ETXTBSY:
	#endif
	case EEXIST:
	case EPERM:
	case EISDIR:
	case EINVAL:
		return FILERR_ACCESS_DENIED;

	case ENFILE:
	case EMFILE:
		return FILERR_TOO_MANY_FILES;

	default:
		return FILERR_FAILURE;
	}
}


//============================================================
//  osd_open
//============================================================

file_error osd_open(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
    /*
     * 所做的一切只是为了打开文件，所以要依照传入的路径检测是否存在，是否是设备文件
     * 如果是socket就用打开socket的方式，如果是设备文件，就用打开设备文件的方式，如果失败
     * 就构造出完整的路径之后再次打开，如果实在打不开，只能执行清理程序，因为可能打开程序
     * 时候的权限不够
     */
{
	UINT32 access;
	const char *src;
	char *dst;
	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU)
	struct stat st;
	#else
	struct stat64 st;
	#endif
	char *tmpstr, *envstr;
	int i, j;
	file_error filerr = FILERR_NONE;

	tmpstr = NULL;

	// allocate a file object, plus space for the converted filename
    /*
     * 对传入的file变量解一级指针，仍然是个指针变量，并且用这个指针指向了分配的空间头
     * 分配的变量是osd_file结构体的大小加上path字符串的大小，但是最后强转成osd_file的
     * 指针,分配的空间分配的osd_file的空间，和其路径的字符串的空间
     */
	*file = (osd_file *) osd_malloc_array(sizeof(**file) + sizeof(char) * strlen(path));
	if (*file == NULL)
	{
		filerr = FILERR_OUT_OF_MEMORY;
		goto error;
	}

	if (sdl_check_socket_path(path))
	{
        /*
         * 使用到了未定义名字的枚举类型的枚举元素
         * 使用打开socket的函数打开后转入清理程序
         */
		(*file)->type = SDLFILE_SOCKET;
		filerr = sdl_open_socket(path, openflags, file, filesize);
		goto error;
	}

	if (strlen(sdlfile_ptty_identifier) > 0 && strncmp(path, sdlfile_ptty_identifier, strlen(sdlfile_ptty_identifier)) == 0)
        /*
         * 检测是否是设备文件，sdlfile_ptty_identifier在sdlptty_unix.c文件中定义为
         * /dev/pts,所以上面是在检测是否存在这一个标志用来从设备中读取文件
         * 设置文件类型后，之久用sdl_open_ptty函数读取就好
         */
	{
		(*file)->type = SDLFILE_PTTY;
		filerr = sdl_open_ptty(path, openflags, file, filesize);
		goto error;
	}

	(*file)->type = SDLFILE_FILE;

	// convert the path into something compatible
	dst = (*file)->filename;
    /*
     * 这儿for循环用于转换反斜杠和斜杠
     */
	for (src = path; *src != 0; src++)
		*dst++ = (*src == INVPATHSEPCH) ? PATHSEPCH : *src;
    /*
     * 以空字符结尾
     */
	*dst++ = 0;

	// select the file open modes
    // 选择文件打开的模式
	if (openflags & OPEN_FLAG_WRITE)
	{
		access = (openflags & OPEN_FLAG_READ) ? O_RDWR : O_WRONLY;
		access |= (openflags & OPEN_FLAG_CREATE) ? (O_CREAT | O_TRUNC) : 0;
	}
	else if (openflags & OPEN_FLAG_READ)
	{
		access = O_RDONLY;
	}
	else
	{
		filerr = FILERR_INVALID_ACCESS;
		goto error;
	}

    /*
     * 虽然给(*file)分配的内存空间大于osd_file所需要的空间，但是最后任然强转成了
     * osd_file的一级指针，所以仍然可以这样使用osd_file结构体中的信息
     */
	tmpstr = (char *) osd_malloc_array(strlen((*file)->filename)+1);
	strcpy(tmpstr, (*file)->filename);

	// does path start with an environment variable?
	if (tmpstr[0] == '$')
	{
		char *envval;
		envstr = (char *) osd_malloc_array(strlen(tmpstr)+1);

		strcpy(envstr, tmpstr);

		i = 0;
		while (envstr[i] != PATHSEPCH && envstr[i] != 0 && envstr[i] != '.')
		{
			i++;
		}

		envstr[i] = '\0';

		envval = osd_getenv(&envstr[1]);
		if (envval != NULL)
		{
			j = strlen(envval) + strlen(tmpstr) + 1;
			osd_free(tmpstr);
			tmpstr = (char *) osd_malloc_array(j);

			// start with the value of $HOME
			strcpy(tmpstr, envval);
			// replace the null with a path separator again
			envstr[i] = PATHSEPCH;
			// append it
			strcat(tmpstr, &envstr[i]);
		}
		else
			fprintf(stderr, "Warning: osd_open environment variable %s not found.\n", envstr);
		osd_free(envstr);
	}

	#if defined(SDLMAME_WIN32) || defined(SDLMAME_OS2)
	access |= O_BINARY;
	#endif

	// attempt to open the file
	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU)
    /*
     * Linux提供的函数
     * 返回的是文件描述信息，如果失败返回-1
     */
	(*file)->handle = open(tmpstr, access, 0666);
	#else
	(*file)->handle = open64(tmpstr, access, 0666);
	#endif
	if ((*file)->handle == -1)
        /*
         * 如果打开失败
         * 如果需要构造绝对路径，则利用create_path_recursive函数构建它
         * 该函数是循环构建的，其中的逻辑很巧妙
         */
	{
		// create the path if necessary
		if ((openflags & OPEN_FLAG_CREATE) && (openflags & OPEN_FLAG_CREATE_PATHS))
		{
			char *pathsep = strrchr(tmpstr, PATHSEPCH);
			if (pathsep != NULL)
			{
				int error;

				// create the path up to the file
				*pathsep = 0;
				error = create_path_recursive(tmpstr);
				*pathsep = PATHSEPCH;

				// attempt to reopen the file
				if (error == NO_ERROR)
				{
					#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU)
                    /*
                     * 再次打开，这次每个路径都已经创建，不会出错，权限666
                     */
					(*file)->handle = open(tmpstr, access, 0666);
					#else
					(*file)->handle = open64(tmpstr, access, 0666);
					#endif
				}
			}
		}

		// if we still failed, clean up and osd_free
        // 所有努力都换来失败的时候，说明你权限不够，关闭重新启动吧
		if ((*file)->handle == -1)
		{
			osd_free(*file);
			*file = NULL;
			osd_free(tmpstr);
			return error_to_file_error(errno);
		}
	}

	// get the file size
	#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU)
    /*
     * (*file)->handle是个int值,fstat函数的第一参数是文件句柄，是一个int值
     */
	fstat((*file)->handle, &st);
	#else
	fstat64((*file)->handle, &st);
	#endif

	*filesize = (UINT64)st.st_size;

error:
	// cleanup
	if (filerr != FILERR_NONE && *file != NULL)
	{
		osd_free(*file);
		*file = NULL;
	}
	if (tmpstr)
		osd_free(tmpstr);
	return filerr;
}


//============================================================
//  osd_read
//============================================================

file_error osd_read(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	ssize_t result;

	switch (file->type)
	{
		case SDLFILE_FILE:
#if defined(SDLMAME_DARWIN) || defined(SDLMAME_BSD) || defined(SDLMAME_EMSCRIPTEN)
            //pread是个Linux系统函数，从一个一直的句柄和偏移开始读字节到已知字节数
			result = pread(file->handle, buffer, count, offset);
			if (result < 0)
#elif defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_OS2)
			lseek(file->handle, (UINT32)offset&0xffffffff, SEEK_SET);
			result = read(file->handle, buffer, count);
			if (result < 0)
#elif defined(SDLMAME_UNIX)
			result = pread64(file->handle, buffer, count, offset);
			if (result < 0)
#else
#error Unknown SDL SUBARCH!
#endif
				return error_to_file_error(errno);

			if (actual != NULL)
			*actual = result;

			return FILERR_NONE;

		case SDLFILE_SOCKET:
			return sdl_read_socket(file, buffer, offset, count, actual);

		case SDLFILE_PTTY:
			return sdl_read_ptty(file, buffer, offset, count, actual);

		default:
			return FILERR_FAILURE;
	}
}


//============================================================
//  osd_write
//============================================================

file_error osd_write(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	UINT32 result;

	switch (file->type)
	{
		case SDLFILE_FILE:
#if defined(SDLMAME_DARWIN) || defined(SDLMAME_BSD) || defined(SDLMAME_EMSCRIPTEN)
			result = pwrite(file->handle, buffer, count, offset);
			if (!result)
#elif defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_OS2)
			lseek(file->handle, (UINT32)offset&0xffffffff, SEEK_SET);
			result = write(file->handle, buffer, count);
			if (!result)
#elif defined(SDLMAME_UNIX)
			result = pwrite64(file->handle, buffer, count, offset);
			if (!result)
#else
#error Unknown SDL SUBARCH!
#endif
		return error_to_file_error(errno);

			if (actual != NULL)
			*actual = result;
			return FILERR_NONE;

		case SDLFILE_SOCKET:
			return sdl_write_socket(file, buffer, offset, count, actual);

		case SDLFILE_PTTY:
			return sdl_write_ptty(file, buffer, offset, count, actual);

		default:
			return FILERR_FAILURE;
	}
}


//============================================================
//  osd_truncate
//============================================================

file_error osd_truncate(osd_file *file, UINT64 offset)
{
	UINT32 result;

	switch (file->type)
	{
		case SDLFILE_FILE:
			result = ftruncate(file->handle, offset);
			if (!result)
				return error_to_file_error(errno);
			return FILERR_NONE;

		default:
			return FILERR_FAILURE;
	}
}


//============================================================
//  osd_close
//============================================================

file_error osd_close(osd_file *file)
{
	// close the file handle and free the file structure
	switch (file->type)
	{
		case SDLFILE_FILE:
			close(file->handle);
			osd_free(file);
			return FILERR_NONE;

		case SDLFILE_SOCKET:
			return sdl_close_socket(file);

		case SDLFILE_PTTY:
			return sdl_close_ptty(file);

		default:
			return FILERR_FAILURE;
	}
}

//============================================================
//  osd_rmfile
//============================================================

file_error osd_rmfile(const char *filename)
{
	if (unlink(filename) == -1)
	{
		return error_to_file_error(errno);
	}

	return FILERR_NONE;
}

//============================================================
//  create_path_recursive
//============================================================

static UINT32 create_path_recursive(char *path)
{
	char *sep = strrchr(path, PATHSEPCH);
	UINT32 filerr;
	struct stat st;

	// if there's still a separator, and it's not the root, nuke it and recurse
	if (sep != NULL && sep > path && sep[0] != ':' && sep[-1] != PATHSEPCH)
	{
		*sep = 0;
		filerr = create_path_recursive(path);
		*sep = PATHSEPCH;
        //检测是否需要创建该路径
		if (filerr != NO_ERROR)
			return filerr;
	}

	// if the path already exists, we're done
	if (!stat(path, &st))
		return NO_ERROR;

	// create the path
	#ifdef SDLMAME_WIN32
	if (mkdir(path) != 0)
	#else
	if (mkdir(path, 0777) != 0)
	#endif
		return error_to_file_error(errno);
	return NO_ERROR;
}

//============================================================
//  osd_get_physical_drive_geometry
//============================================================

int osd_get_physical_drive_geometry(const char *filename, UINT32 *cylinders, UINT32 *heads, UINT32 *sectors, UINT32 *bps)
{
	return FALSE;       // no, no way, huh-uh, forget it
}

//============================================================
//  osd_is_path_separator
//  检测是否是分隔符
//============================================================

static int osd_is_path_separator(char c)
{
	return (c == '/') || (c == '\\');
}

//============================================================
//  osd_is_absolute_path
//  检测是否是绝对路径
//============================================================

int osd_is_absolute_path(const char *path)
{
	int result;

	if (osd_is_path_separator(path[0]))
		result = TRUE;
#if !defined(SDLMAME_WIN32) && !defined(SDLMAME_OS2)
	else if (path[0] == '.')
		result = TRUE;
#else
	#ifndef UNDER_CE
	else if (*path && path[1] == ':')
		result = TRUE;
	#endif
#endif
	else
		result = FALSE;

	return result;
}
#endif
