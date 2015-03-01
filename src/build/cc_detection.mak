ifneq (,$(findstring clang,$(CC)))
	include $(SRC)/build/flags_clang.mak
else
	ifneq (,$(findstring emcc,$(CC)))
		# Emscripten compiler is based on clang
		include $(SRC)/build/flags_clang.mak
	else
		TEST_GCC = $(shell gcc --version)
		# is it Clang symlinked/renamed to GCC (Xcode 5.0 on OS X)?
		ifeq ($(findstring clang,$(TEST_GCC)),clang)
			include $(SRC)/build/flags_clang.mak
		else
			include $(SRC)/build/flags_gcc.mak
		endif
	endif
endif

#这部分文档的主要功能是根据系统中的CC变量来找出系统所安装的C语言编译器从中我们可以看出
#OSX系统的蛋疼之处，直接把系统的gcc指向llvm。
#在测试完CC变量之后，这部分的mak会载入不同的几份mak文档来具体的编译文件
#再Ubuntu14.4中载入了flags_gcc.mak
#但是没有改变CC的值
#SRC的值仍然是src
