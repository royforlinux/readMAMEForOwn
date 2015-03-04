>随笔记，未整理

###基础知识
+ #pragmaonce
在C和C++编程语言中，#pragma once 是一个非标准但是被广泛支持的前置处理符号，会让所在的文件在一个单独的编译中只被包含一次。以此方式，#pragma once 提供类似include防范的目的，但是拥有较少的代码且能避免名称的碰撞。和ifndef相比各有优缺点，如果用了这
这个特性，你就必须依赖于你的编译器，这也是C语言的标准曾一度把它列为已淘汰之列，不过在gcc3.4版本之后
你就不用担心这个问题了。已淘汰标签已经去除。
+ 关于makefile的隐含规则
在makefile中，可以运用很多隐含规则，并且，makefile的隐含规则的算法能够有效的找到你需要的编译器,需要注意
的是这样的导出的动作，并不是默认参数，有很多参数可以加在其中。详见《GUNmake手册》。
+ 左移或右移N位运算其实其实就是除或乘2的N次幂。
###
- src/
	- emu/
		- rendersw.inc
	      >该文件主要声明了类模板`softwave_render`在该类模板中，主要用了
		  >内联函数以达到简单的画面的目的，详细函数说明及注释，见目录文件

		- render.h/render.c
			> 该文件中声明了许多结构体，并且在文件中对画面的渲染做了解释
- docs/
	- m6502.txt
		>该文件中主要讲了很多的关于`m6502`系列CPU的操作，讲了如何用cpu类来模拟CPU的很多特征

	- mame.txt
		>该文件主要介绍了mame软件的开发目的，和一些声明

	- licence.txt
		>该文件时认证文件

	- licenceinfo.txt
		>该文件给出了全部法律文件的网址

	- SDL.txt
		>该文件主要讲了SDL的config，其中也有OpenGL的config。这两部分有联系

	- config.txt
		>该文件讲了MAME模拟器的配置，这些配置可以写进MAME.ini文件中

	- floppy.txt
		>该文件主要讲了新型的软盘(floppy disk)驱动。具体这个新型软盘有什么效果还未知，但是该文件
		>中确实讲了软盘驱动的一些可能有用的函数

	- hlsl.txt
		>该文件讲了HLSL配置，具体这个配置讲的什么，还未知

	- imgtool.txt 
		>首先imgtool是个镜像操作工具，是用来转换镜像类型的。而该文件就是介绍这个工具的用法。
		>该工具是MESS项目的一部分，MESS项目是什么我现在还不知道

	- newvideo.txt
		>该文件介绍了MAME的新的Video系统及其配置选项

	- nscsi.txt
		>该文件讲述了新的SCSI总线子系统，主要讲了这个驱动类中的一些函数接口
- src
	- osd
		- sdl
			- main.c 
				>windows.h是MingW32编译器自带的文件，同时也是windows的标准文件,还有
				>其他的标准文件都是由编译器自己提供,main.c文件中并没有种真正的main函数，至今
				>（2015-02-28 10：13：18）我们仍然不知道，这个程序从哪里开始执行，但是这个
				>_tmain函数却完成了对于字符的转换工作，虽然具体转换过程并非再次，但
				>是至少他提供了合适的逻辑代码，也就是说具体的转换工作来自于
				>其他的函数，比如utf8_from_tstring函数，但是至少它提供了整个的转换逻辑，并且
				>我还发现在utf8_from_tstring函数所调用的地方，利用编译预处理做了类似函数重载
				>的操作，这些转换操作大部分都需要*SDLMAME_WIN32*这个宏定义的支持，可能我在阅
				>读源码的开始就进入了windows的误区，毕竟我是在ubuntu14上编译成功，或许和我的
				>预期并不好。
				>接着该函数调用了uft8_main函数，这个函数再其出现的文件中也是条件编译的，并且
				>是有可能编译成main函数的，也就是函数入口，可能这个就是我在Ubuntu14上编译程序
				>的程序入口，这也让我想到在win32程序中是不是_tmain函数才是真正的函数入口，利
				>这个差别，完成了代码的可移植呢？但是，win32用该是*int WINAPT winmain*函数是
				>函数的入口，这个_tmain函数到底在什么时候才会被执行呢？
				>最后这个函数返回了rc，该变量接受了调用*utf8_main*时候返回的值.
				>该函数还有田间编译的变量*winalloc_in_main_code*和函数*check_unfreed_mem*
				>这两者都需要MALLOC_DEBUG这个宏定义定义才能编译
			- strconv.c
				>该文件是用来对字符进行转换，在文件中宏定义*SDLMAME_WIN32*有很重要的作用，它影
				>响了很多函数的编译和替换。
				>该文件一共定义了三个函数，前两个函数都是基于windows.h文件的，只是写出了ANSI
				>及宽字符向UTF-8字符集转换的逻辑。最后一个函数是char字符向UTF-8的转换，主要数
				>距转换是用了stdlib.h头文件中的mbstowcs函数处理的,函数同样只是产生了逻辑
			- strconv.h
				>作为strconv.c的头文件。这个文件中宏定义*SDLMAME_WIN32*依然有很大的作用。它
				>决定了文件时候包含对应的函数原型，而宏定义*UNICODE*则决定了是否有对应的函数名
				>替换，在对应的c文件中定了三个函数，由于宏定义的影响，本头文件只对前两个进行了
				>替换操作。
			- sdldir.c
				>是sdl core的目录入口文件，看样子还定义了自己结构体，但是为什么要这么做呢？(2015
				>年 3月 1日 星期日 15时08分57秒 CST)
				>在该文件见用到了DT_xxx的宏定义，这些宏定义都来自Linux自身提供的数据结构，也就是
				>需要*dirent.h*文件和特殊的宏定义，这一点在源文件中已经表明,这些宏定义只有在
				>Linux下环境的*man readdir*能找到。不过至少能在www.man7.org这个网站能够找到你想
				>要的东西。
				>这个文件主要定义了对于目录处理的函数包括，打开，读取数据，关闭操作。这些操作
				>都由系统自带的对应函数进行实现。
				>这个文件中还定义了很多辅助的函数，比如构建绝对路径，获取目录类型标志，并且定义
				>了自己的文件目录入口结构体，不过可能有些繁琐，从这个文件开始(2015年 
				>3月 2日 星期一 18时35分34秒 CST)我忽略windows有关的实现，也就是默认没有
				>SDLMAME_WIN32宏定义
			- sdlfile.h
				>这个文件是对应c文件的头文件，该文件定义了：
				>一个没有名称的枚举类型，其中的枚举元素都是SDLFILE_xxx
				>一个sdlfile结构体
				>九个函数的原型，其中八个是分别用来，打开、读取、写入、关闭嵌套字或PTTY的，一个
				>用来进行错误标识的转换
			- sdlfile.c
				 这个文件主要做了处理osd文件的操作，注意这个文件的概念是广泛的。
				 具体看源文件吧，这个文件是阅读源码以来最大的文件，不过其中的很多逻辑很巧妙
				 例如： create_path_recursive 递归的构造目录以组后构造完整的路径,
				 这个文件中最复杂、代码量最多的就是osd_open函数
	- build
		- file2str.c
			>该文件中又main函数，并且已经标明，是主函数入口。
			>不过似乎这是一个源代码转换的工具软件，目前（2015-02-28 12:05:55)并没有确切的信息
			>可以证实这一点
			>现在可以知道这个文件中代码的做和文件名一样是完成了源文件内容向字符串的转换，代码
			>已byte的格式把源文件的内容写入了dstfile中。并且在文件的开头，注明了varname和type
			>并且每byte间用逗号分隔，尾部也有格式处理,是不是镜像转换工具的代码呢？（2015-02-28
			>13:16:12)
