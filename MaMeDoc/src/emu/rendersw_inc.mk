此文件主要用作对`software_render`类模板的编写，此模板类主要加入了一些内联函数，从而达到了画面渲染的目的。
该文件按其后缀名是作为一个头文件的，其中主要声明了一个类模板，类结构为
+ class template : softwave_renderer
	+ private member
		1. struct quad_setup_data
	+ ycc_to_rgb(UINT32 ycc)
		> 这个函数是用一个32位无符号整型引入参数，而一般性的YCrCb信号用4:2:2分量，但是下面我们看到直接
		> UINT8类型接受ycc型号只接收到了参数尾部8位，这样么说4:2:2是没有意义的，所以，也可以是4:4:4参数
		> 每个亮度分量只有8为表示，最高256的值，转换过程程序中已经给出。有趣的是，最后这个函数并没有返
		> 回rgb着三个分量的信号，而是返回了一个rgb_t类，其中把这三个分量的信号分别调用了一次
		> `clamp16_shift8`函数，这个函数主要用到是用截数的方法实现了所有数值缩小256倍的功能，至于原理
		> 我还不清楚，算法的讲解，及硬件我是参考的下面的论文。
		> [基于FPGA的YCrCb_RGB色彩空间的转换](http://pan.baidu.com/s/1i3l8ZXF)
		> [YUV与RGB之间的转换_邵丹](http://pan.baidu.com/s/1nt3bK73)
	+ clamp16_shift8(UINT32 x)
		> `static inline UINT32 clamp16_shift8(UINT32 x)
			{
			return ((INT32(x) < 0) ? 0 : (x > 65535 ? 255: x >> 8));
			}`
	+ get_textl_palette16(const render_texinfo &texture,INT32 curu, INT32 curv)
		> 该函数引用了一个render_texinfo对象，实际上时render.h文件中定义的一个结构体，代码如下
		> `// render_texinfo - texture information
struct render_texinfo
{
	void *              base;               // base of the data
	UINT32              rowpixels;          // pixels per row
	UINT32              width;              // width of the image
	UINT32              height;             // height of the image
	const rgb_t *       palette;            // palette for PALETTE16 textures, LUTs for RGB15/RGB32
	UINT32              seqid;              // sequence ID
	UINT64              osddata;            // aux data to pass to osd
};`
