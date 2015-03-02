//============================================================
//
//  sdldir.c - SDL core directory access functions
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_WIN32
/*
 * 再一次检测了这个宏命令，不过在Ubuntu14上并没有用到
 */
#include "../windows/windir.c"
#else

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifdef SDLMAME_LINUX
#define __USE_LARGEFILE64
#endif
#ifndef SDLMAME_BSD
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 500
#endif //SDLMAME_BSD

//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef __USE_BSD
#define __USE_BSD   // to get DT_xxx on Linux
#endif
#undef _POSIX_C_SOURCE  // to get DT_xxx on OS X
#include <dirent.h>

#include "osdcore.h"
#include "sdlos.h"

/*
 * 斜杠反斜杠的定义，宏定义的长度太长了吧
 */
#if defined(SDLMAME_WIN32) || defined(SDLMAME_OS2)
#define PATHSEPCH '\\'
#define INVPATHSEPCH '/'
#else
#define PATHSEPCH '/'
#define INVPATHSEPCH '\\'
#endif

#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2) || defined(SDLMAME_HAIKU) || defined(SDLMAME_EMSCRIPTEN)
/*
 * 只有是以上宏定义有其一做这样的替换，可见以上系统都是32位
 */
typedef struct dirent sdl_dirent; //dirent来自于dirent.h
typedef struct stat sdl_stat;
#define sdl_readdir readdir
#define sdl_stat_fn stat
#else
typedef struct dirent64 sdl_dirent;
typedef struct stat64 sdl_stat;
#define sdl_readdir readdir64
#define sdl_stat_fn stat64
#endif

#define HAS_DT_XXX defined(SDLMAME_LINUX) || defined(SDLMAME_BSD) || defined(SDLMAME_DARWIN)

struct osd_directory
/*
 * 这种结构体是基于dirent 、 DIR两种Linux系统定义的结构体构建起来的
 * Linux提供的这两种结构体都是为了目录准备的，这里MAME不仅将两者绑定在一起，还加入了自己定义的
 * 目录入口标志的枚举类型，和路径，会不会有些臃肿？(2015年 3月 2日 星期一 14时16分02秒 CST)
 */
{
	osd_directory_entry ent; //枚举类型来自于osdcore.h,用于区别文件类型
    //这里应该是被替换成了 dirent,该结构体只包含了目录的少量信息,并且这是是指针
	sdl_dirent *data;
	DIR *fd; //linux支持的文件结构，包含在dirent.h文件中,保存了目录文件的信息
	char *path;
};

static char *build_full_path(const char *path, const char *file)
    /*
     * 在C语言中完成了两个以空字符串结尾的字符串的拼接
     */
{
    //osd_malloc_array函数是由malloc函数支持的
	char *ret = (char *) osd_malloc_array(strlen(path)+strlen(file)+2);
	char *p = ret;

	strcpy(p, path);
	p += strlen(path);
	*p++ = PATHSEPCH; //先进行自增然后解指针，表达式相当于在path后面填入一个斜杠
	strcpy(p, file);
	return ret; //此指针一直指向分配内存的头部也就是拼接后字符串的头
}


#if HAS_DT_XXX
static osd_dir_entry_type get_attributes_enttype(int attributes, char *path)
    /*
     * 这个函数的用途就是原来文件标志向osd内部定义的目录入口标志的转换
     * 输入的是一个int型的标志，和一个字符类型的路径，返回一个osd_dir_entry_type类型的枚举
     * 变量，osd_dir_entry_type类型的枚举类型定义在osdcore.h文件中， DT_xxx来自于Linux系统
     * 内部，在下面我给出了注释
     * 还有在符号链接类型下的处理,在这样情况下就需要文件的路径了
     */
{
	switch ( attributes )
	{
        //This a directory
		case DT_DIR:
            //返回一个枚举类型
			return ENTTYPE_DIR;

        //This is a regular file
		case DT_REG:
			return ENTTYPE_FILE;

        //This is symbole link
		case DT_LNK:
		{
            //这里没有用sdl_stat这个名称，和win32没关系了?2015年 3月 2日 
            //星期一 13时24分05秒 CST
            //这个结构体储存这文件的详细信息，并且有同名函数调用
			struct stat s;

			if ( stat(path, &s) != 0 )
				return ENTTYPE_OTHER;
			else
                //S_ISDIR 是一个宏函数,逻辑是和_S_IFDIR做&运算,下面这行代码是在弄清楚link
                //文件链接的是一个目录还是一个文件
                //st.mode是stat结构体中存储的文件访问权限记录
                //另外dirent.h本身或者是Linux环境也提供了这个宏函数，用以判断是否是目录文件
				return S_ISDIR(s.st_mode) ? ENTTYPE_DIR : ENTTYPE_FILE;
		}

		default:
			return ENTTYPE_OTHER;
	}
}
#else
/*
 * 注意这里的条件编译，如果不是常见的Linux的派系，将编译如下函数
 */
static osd_dir_entry_type get_attributes_stat(const char *file)
{
    //sdl_stat和sdl_stat_fn都被宏定义替换成stat
	sdl_stat st;
	if(sdl_stat_fn(file, &st))
        //这一步强转成osd_dir_entry_type类型是什么意思？为什么不直接返回对应的类型(ENTTYPE_NONE)?
        // 2015年 3月 2日 星期一 14时30分01秒 CST
		return (osd_dir_entry_type) 0;

	if (S_ISDIR(st.st_mode))
		return ENTTYPE_DIR;
	else
		return ENTTYPE_FILE;
}
#endif

static UINT64 osd_get_file_size(const char *file)
{
	sdl_stat st;
    //stat函数成功的时候返回0，就不会执行if的函数体,e里的逻辑是正确的
	if(sdl_stat_fn(file, &st))
		return 0;
	return st.st_size;
}

//============================================================
//  osd_opendir
//============================================================

osd_directory *osd_opendir(const char *dirname)
{
	osd_directory *dir = NULL;
	char *tmpstr, *envstr;
	int i, j;

    //osd_malloc函数用malloc函数实现，不过加入了宏控制
	dir = (osd_directory *) osd_malloc(sizeof(osd_directory));
	if (dir)
	{
		memset(dir, 0, sizeof(osd_directory));
		dir->fd = NULL;
	}

    /*
     * 因为strlen所获取到的字符串的长度不包含空字符，所以要加1
     * osd_malloc_array 和osd_malloc函数代码相同
     * 而这这么做的目的是把字符拆成数组么？
     */
	tmpstr = (char *) osd_malloc_array(strlen(dirname)+1);
	strcpy(tmpstr, dirname);

	if (tmpstr[0] == '$')
        /*
         * 检测到头字符是美元符号，证明是以环境变量开头
         */
	{
		char *envval;
		envstr = (char *) osd_malloc_array(strlen(tmpstr)+1);

		strcpy(envstr, tmpstr);

		i = 0;
		while (envstr[i] != PATHSEPCH && envstr[i] != INVPATHSEPCH && envstr[i] != 0 && envstr[i] != '.')
            /*
             * 这个循环是把i挪到字符串的第一个斜杠、反斜杠、空字符或者点处
             */
		{
			i++;
		}

        //强制转换成空字符
		envstr[i] = '\0';

        /* 
         * 这个函数由getenv实现，目的是获取环境变量的值,这词调用很精巧，把原来的路径分隔符替换
         * 成空字符，在把首元素的指针传入函数中，那么传入的就是从首元素到空字符的字符串
         */
		envval = osd_getenv(&envstr[1]);
		if (envval != NULL)
		{
            /*
             * 计算总的空间长度
             * 释放原来的空间
             * 把指针指向新空间
             */
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
            /* 这是不允许环境变量的值为空么？( 2015年 3月 2日 星期一 18时14分09秒 CST) */
			fprintf(stderr, "Warning: osd_opendir environment variable %s not found.\n", envstr);
		osd_free(envstr);
	}

	dir->fd = opendir(tmpstr);
	dir->path = tmpstr;

	if (dir && (dir->fd == NULL))
	{
		osd_free(dir->path);
		osd_free(dir);
		dir = NULL;
	}

	return dir;
}


//============================================================
//  osd_readdir
//============================================================

const osd_directory_entry *osd_readdir(osd_directory *dir)
{
	char *temp;
	dir->data = sdl_readdir(dir->fd);

	if (dir->data == NULL)
		return NULL;

	dir->ent.name = dir->data->d_name;
	temp = build_full_path(dir->path, dir->data->d_name);//返回了文件的绝对路径
	#if HAS_DT_XXX
	dir->ent.type = get_attributes_enttype(dir->data->d_type, temp);
	#else
	dir->ent.type = get_attributes_stat(temp);
	#endif
	dir->ent.size = osd_get_file_size(temp);
	osd_free(temp);
	return &dir->ent;
}


//============================================================
//  osd_closedir
//============================================================

void osd_closedir(osd_directory *dir)
{
	if (dir->fd != NULL)
		closedir(dir->fd);
	osd_free(dir->path);
	osd_free(dir);
}

#endif
