/***************************************************************************

    file2str.c

    Simple file to string converter.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>


/*-------------------------------------------------
    main - primary entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	const char *srcfile, *dstfile, *varname, *type;
	FILE *src, *dst;
	unsigned char *buffer;
	int bytes, offs;
	int terminate = 1;

	/* needs at least three arguments */
	if (argc < 4)
	{
		fprintf(stderr,
			"Usage:\n"
			"  laytostr <source.lay> <output.h> <varname> [<type>]\n"
			"\n"
			"The default <type> is char, with an assumed NULL terminator\n"
		);
		return 0;
	}

	/* extract arguments */
	srcfile = argv[1];
	dstfile = argv[2];
	varname = argv[3];
	type = (argc >= 5) ? argv[4] : "char";
	if (argc >= 5)
		terminate = 0;

	/* open source file */
	src = fopen(srcfile, "rb"); //以只读和二进制方式打开第一个参数所指向的文件
	if (src == NULL)
	{
        /*测试是否打开了文件但是写入的文件时stderr,这是一个宏变量，是FILE*流的错误表达式*/
		fprintf(stderr, "Unable to open source file '%s'\n", srcfile);
		return 1; //为什么发生了错误还要返回1呢？
	}

	/* determine file size */
    /* 通过fseek和ftell获取了src指向文件的大小，并且最后还用fseek函数还原了当前位置锚（文件位置
     * 指示器）*/
	fseek(src, 0, SEEK_END); //改变当前的文件位置指示器到最后，用于以二进制读文件中很有效
	bytes = ftell(src); //返回一个当前文件状态的位置指示器,对于二进制打开的文件，返回byte数
	fseek(src, 0, SEEK_SET);

	/* allocate memory */
	buffer = (unsigned char *)malloc(bytes + 1); //多分配了1byte
	if (buffer == NULL)
    /*
     *错误检查，此次检查是检查是否成功分配了内存，看来
     *1byte是为了防止在空文件情况下程序错误退出用的。因为之前成功检测是否打开文件，所以在
     *错误处理的时候要关闭文件指针
     */
	{
		fclose(src);
		fprintf(stderr, "Out of memory allocating %d byte buffer\n", bytes);
		return 1;
	}

	/* read the source file */
    /*以二进制的形式把文件中的内容读入刚刚分配的内存中，内存中多分配的1byte在最后*/
	fread(buffer, 1, bytes, src);
	buffer[bytes] = 0;   //把多分配的1byte强制赋值为0不过这种数组般地内存访问方式...
	fclose(src);

	/* open dest file */
	dst = fopen(dstfile, "w");
	if (dst == NULL)
	{
		free(buffer);
		fprintf(stderr, "Unable to open output file '%s'\n", dstfile);
		return 1;
	}

	/* write the initial header */
	fprintf(dst, "extern const %s %s[];\n", type, varname);
	fprintf(dst, "const %s %s[] =\n{\n\t", type, varname);

	/* write out the data */
	for (offs = 0; offs < bytes + terminate; offs++) 
        /*
         * 根据输入参数的个数有不同的偏移,如果参数个数是4个，那么terminate=1，也就是
         * 说最后的1byte 0 也写入了dstfile中，但是如果参数多余四个，那么type就是'char'并且
         * terminate=0也就是最后的1byte 0并没有写入dstfile中
         */
	{
        /*这个格式控制好牛逼，中间的都以","来隔开，最后用空字符串结尾，不会多余逗号*/
		fprintf(dst, "0x%02x%s", buffer[offs], (offs != bytes + terminate - 1) ? "," : "");
		if (offs % 16 == 15)
			fprintf(dst, "\n\t");
	}
	fprintf(dst, "\n};\n");

	/* close the files */
	free(buffer);
	fclose(dst);
	return 0;
}
