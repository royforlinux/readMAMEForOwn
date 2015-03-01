###/makefile文件
第一个载入的新makefile文件在608行，

	include $(SRC)/build/cc_detection.mak

在上述文件中测试了变异所用的编译器，并且载入了新的mak文件

在mame/makefile中有一个_defaule_目标和_all_目标已知,具体的编译输出文件仍然未知

	default: maketree buildtools emulator

	all: default tools

增加的第二个mak文件，在860行，用来编译sdl 也就是OSD层
在makefile定义了许多变量和许多选项，都是用作编译的时候的警告输出，和交叉编译的。
代码行如下
      
	include $(SRC)/osd/$(OSD)/$(OSD).mak

####变量
下面列举一些主要的变量的值
* OS变量并没有定义在这种情况下，就会判断用户有没有设置OS2_SHELL变量，如果声明了，

* TARGETOS变量的值是就被赋值为os2，在没有声明的情况下，OS2_SHELL变量的值为空，makefile就利用

* UNAME变量来获取系统的名称，CPU构架，系统构架，UNAME变量的值由`uname -mps`命令获取，在我的目标系统中

* TARGETOS=Linux

* PTR64=x86_64,用来指向CPU的构架

* OBJ = obj/$(PREFIX)$(OSD)$(SUFFIX)$(SUFFIX64)$(SUFFIXDEBUG)$(SUFFIXPROFILE)
	* 只有OSD变量是等于sdl64
* INCPATH += \
  *	-I$(SRC)/$(TARGET) \
  *	-I$(OBJ)/$(TARGET)/layout \
  *	-I$(SRC)/emu \
  *	-I$(OBJ)/emu \
  *	-I$(OBJ)/emu/layout \
  *	-I$(SRC)/lib/util \
  *	-I$(SRC)/lib \
  *	-I$(SRC)/osd \
  *	-I$(SRC)/osd/$(OSD) \
	
* LDFLAGSEMULATOR =

* OPTIMIZE = 3

* OBJDIRS = $(OBJ) $(OBJ)/$(TARGET)/$(SUBTARGET)
###/src/build/cc_detection.mak
这部分文档的主要功能是根据系统中的CC变量来找出系统所安装的C语言编译器从中我们可以看出
OSX系统的蛋疼之处，直接把系统的gcc指向llvm。
在测试完CC变量之后，这部分的mak会载入不同的几份mak文档来具体的编译文件
再Ubuntu14中载入了/src/build/flags_gcc.mak

####变量
+TEST_GCC = $(shell gcc --version)

###/src/build/flags_gcc.mak
该文件建立了
* TEST_GCC变量来记录系统的gcc编译器版本
* CCOMFLAGS变量来记录编译公用选项,主要是wno之类的waring选项，不用在意
####变量
* TEST_GCC=gcc (Ubuntu 4.8.2-19ubuntu1) 4.8.2
* CCOMFLAGS += -Wno-narrowing -Wno-attributes -Wno-unused-local-typedefs -Wno-unused-variable -Wno-array-bounds

###/src/osd/sdl/sdl.mak

现在进行到src/osd/sdl/sdl.mak文件 这个文件中
这个文件时SDL层的详细makefile

####常见变量
* OSDSRC=src/osd
* OSDOBJ=obj/sdl64/osd
* SDL_LIBER=2变量定义了sdl库的版本，直接体现在sdlx-config上。也就是sdl2-config
* OBJ = obj/$(PREFIX)$(OSD)$(SUFFIX)$(SUFFIX64)$(SUFFIXDEBUG)$(SUFFIXPROFILE),化简为OBJ=obj/sdl
* BASE_TARGETOS = unix
* SYNC_IMPLEMENTATION = tc
* SDL_NETWORK = taptun
* SDLSRC=src/osd/sdl
* SDLOBJ=obj/sdl64/osd/sdl
