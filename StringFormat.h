// StringFormat.h
// 字符串格式化。类似C语言的sprintf，但更加安全。
//
//
//
// 特性：
// (1) 所使用的格式串，例如%d, %f等，尽可能的与标准C相同，减少使用障碍
// (2) 会检查数据的类型（确保float不会以%d输出，等等）。
// (3) 使用时不必指定buffer的大小，避免内存错误。
//
//
//
// 使用：
// (1) 一般使用时，只需要用上层接口即可。
//     示例：std::string strResult = string_format::Format("today is %04d-%02d-%02d", __FILE__, __LINE__) << 2009 << 9 << 4;
//     或者：std::string strResult = FORMAT("today is %04d-%02d-%02d") << 2009 << 9 << 4;
// (2) 如果是作为参数传递，并且参数要求是const char *类型，则可以这么写：
//           std::string strTemp = FORMAT("today is %04d-%02d-%02d") << 2009 << 9 << 4;
//           somefunc(strTemp.c_str());
//     或者：somefunc((FORMAT("today is %04d-%02d-%02d") << 2009 << 9 << 4).str().c_str());
//     千万不要这样写：const char *pszTemp = (FORMAT("today is %04d-%02d-%02d") << 2009 << 9 << 4).str().c_str();
//             这样会得到一个野指针，造成若干问题。
// (3) 在需要考虑效率时，可以直接使用CStringFormatter
//     示例：CStringFormatter fmt("today is %04d-%02d-%02d", __FILE__, __LINE__);
//           fmt.AddArgument(2009);
//           fmt.AddArgument(9);
//           fmt.AddArgument(4);
//           assert(fmt.AsString() == "today is 2009-09-04");
//     一个CStringFormatter对象可以保存起来，避免对相同的格式串作反复的解析，从而提高性能。
//
//
//
// 实现：
// (1) 在浮点数输出时依靠了C++的stringstream。
// (2) 关于宽字符、宽字符串，在当前版本中尚未实现。
//
//
//
// 格式串的语法：基本参考了http://www.cplusplus.com/reference/clibrary/cstdio/sprintf/，
// 但根据实际需求，增加了关于%I64d的描述
//
//     %[flags][width][.precision][length]specifier
//     （用[]括起的内容为可选内容）
//
// 其中：
//     flags可以是+, -, #, 0, 或者空格。这些内容可以任意组合（顺序无关），比如"%+08d"，同时使用了0和+。
//         +表示“一定输出符号”。比如3，则输出为+3（仅对整数、浮点数有效，对字符、字符串无效，对无符号整数%u, %x, %X等也无效）
//         -表示“左对齐”（默认是“右对齐”）
//         #表示：(1) 对于%o, %x, %X，在输出前面加0, 0x, 0X
//                (2) 对于%f, %e, %E，输出一定包含小数点
//                (3) 对于%g, %G，输出一定包含小数点，并且不会把末尾的零去掉（默认会去掉）
//         0表示：宽度不足时用字符'0'填充（仅在右对齐时有效）。
//         空格表示：对非负值，留一个空格，以便于负数可以对齐（仅对整数、浮点数有效，对字符、字符串无效，对无符号整数%u, %x, %X等也无效）
//     width是一个数字，或者*。表示“至少输出多少个字符”。不足部分用空格填充（如果flags指定了0则用0填充）
//         如果width是*，则表示从后面的参数表里面读取
//     precision是一个数字，或者*。表示：
//         (1) 对于整数（%d, %i, %o, %u, %x, %X），
//             表示“至少输出多少个数字”。不足部分用0填充
//         (2) 对于%e, %E, %f，表示小数点后保留几位（四舍五入）
//         (3) 对于%g, %G，表示最多几位有效数字（四舍五入）
//         (4) 对于%s，表示最多输出几个字符
//         (5) 对于%c，无任何效果
//         如果precision是*，则表示从后面的参数表里面读取
//     length可以是h, l, L, I64
//         h表示“短整数”，对%d, %i, %o, %u, %x, %X，当作short或者unsigned short来处理
//         l表示“长整数”，对%d, %i, %o, %u, %x, %X，当作long或者unsigned long来处理
//                          对%c, %s，当作wchar_t类型来处理（目前版本没有实现）
//         L表示“超长浮点数”，对%f, %e, %E, %g, %G，当作long double来处理
//         I64表示“64位”，对%d，当作__int64；对%u，当作unsigned __int64
//     specifier可以是以下内容：
//         c: 从参数表里面取一个字符，输出
//         d: 从参数表里面取一个带符号的整数，按十进制输出
//         i: 同上
//         e: 从参数表里面取一个浮点数，按科学记数法输出（底数和指数之间用e分割）
//         E: 从参数表里面取一个浮点数，按科学记数法输出（底数和指数之间用E分割）
//         f: 从参数表里面取一个浮点数，按十进制输出
//         g: 从参数表里面取一个浮点数，并在%e和%f两种输出方式之间作选择，取较短者作为实际输出。（默认会去掉末尾的零）
//         G: 从参数表里面取一个浮点数，并在%E和%f两种输出方式之间作选择，取较短者作为实际输出。（默认会去掉末尾的零）
//         o: 从参数表里面取一个带符号的整数，按八进制输出
//         s: 从参数表里面取一个字符指针，输出从该地址开始的字符串
//         u: 从参数表里面取一个无符号的整数，按十进制输出
//         x: 从参数表里面取一个无符号的整数，按十六进制输出（字母用小写）
//         X: 从参数表里面取一个无符号的整数，按十六进制输出（字母用大写）
//         p: 从参数表里面取一个任意类型的指针，输出（格式由平台自行定义？）
//         n: 不输出。从参数表里面取一个int类型的指针，把“当前已经输出的字符数”设置到指针所指位置
//         %: 输出一个“%”（百分号）字符

#ifndef __TQ_STRINGFORMAT_H__
#define __TQ_STRINGFORMAT_H__

#include <string>



namespace string_format
{

// 错误代码 0表示正常，没有错误或警告 1000~1999表示警告 2000~2999表示错误
enum ErrorCode
{
	NoError						= 0,	// 没有错误或警告

	Warning_TooManyArguments	= 1000, // 警告：传入的参数过多

	Error_InvalidFormatString	= 2000, // 错误：pszFormat非法，导致解析失败
	Error_InvalidArgumentType,			// 错误：传入的参数类型不符合要求
	Error_NotEnoughArguments,			// 错误：传入的参数过少
	Error_NotImplemented,				// 错误：在目前版本中尚未实现

	InternalError				= 3000, // 内部错误
};

// ErrorHandler: 一种指针类型，可以指向“错误处理”函数
// 参数err表示错误代码。参见enum ErrorCode
// 参数pszMessage表示描述该错误的一个字符串
typedef void (*ErrorHandler) (ErrorCode err, const char *pszMessage, const char *pszFileName, int iFileLine);

// SetErrorHandler: 设置错误处理函数。
// 当遇到错误时会调用此函数。函数中可以记录log，或者产生异常（需要自己catch）
// 注意这里的“设置”，其影响是全局的，新的设置会覆盖旧的设置。
// 如果从来不调用此函数，或者用NULL为参数调用此函数，则表示“采用默认的错误处理”
void SetErrorHandler(ErrorHandler handler);

// GetErrorHandler: 取得当前的错误处理函数。 如果没有设置错误处理函数，则返回NULL
ErrorHandler GetErrorHandler();



// CStringFormatter: 核心实现类
class CStringFormatter
{
public:
	CStringFormatter(const char *pszFormat, const char *pszFileName, int iFileLine);
	~ CStringFormatter();

	CStringFormatter(const CStringFormatter &copy);
	CStringFormatter &operator =(const CStringFormatter &copy);

	bool IsFormatValid() const;

	// 把格式化好的数据以std::string的形式返回。
	// 内部设置缓存，因此多次调用不会有多次开销。
	// 在调用次函数之前，请确保传入参数（用Format函数传入参数）的数量正确，并且类型匹配，否则会出错并转到ErrorHandler
	const std::string &AsString() const;

	// 清除已经加入的数据，以便重新进行格式化
	void ClearArguments();

	// 提供若干版本的重载，以便能够处理各种不同的数据类型
	// 说明：对于C标准的printf函数，它是支持可变长参数的，在传参数时它遵守以下规则：
	//       各种带符号的“整数”类型（包括char, short, int, 各种enum等。但不包括后来才定义的long long，即__int64），在传递时全部转为long类型
	//       各种无符号的“整数”类型与之类似，在传递时全部转为unsigned long类型
	//       float类型在传递时全部转为double类型
	//       后来定义的long long（VC6的__int64）、unsigned long long（VC6的unsigned __int64）、long double，在传递时按照原样传递，不转化
	// 为此，针对int的重载和针对short的重载其实在实现时完全一致。
	// 另外，VC6中wchar_t其实就是unsigned short，因此不另行提供重载
	void AddArgument(int data);
	void AddArgument(unsigned int data);
	void AddArgument(short data);
	void AddArgument(unsigned short data);
	void AddArgument(long data);
	void AddArgument(unsigned long data);
	void AddArgument(char data);
	void AddArgument(signed char data);
	void AddArgument(unsigned char data);
	void AddArgument(__int64 data);
	void AddArgument(unsigned __int64 data);
	void AddArgument(float data);
	void AddArgument(double data);
	void AddArgument(long double data);
	void AddArgument(const void *data);
	void AddArgument(const char *data);
	void AddArgument(int *data);

private:
	class CImpl;
	CImpl *m_pImpl;
};



// CFormatHelper: 上层接口类
// 用FORMAT宏来使用这个类，效果更好
class CFormatHelper
{
public:
	CFormatHelper (const char *pszFormat, const char *pszFileName, int iFileLine);

	CFormatHelper & operator << (int data);
	CFormatHelper & operator << (unsigned int data);
	CFormatHelper & operator << (short data);
	CFormatHelper & operator << (unsigned short data);
	CFormatHelper & operator << (long data);
	CFormatHelper & operator << (unsigned long data);
	CFormatHelper & operator << (char data);
	CFormatHelper & operator << (signed char data);
	CFormatHelper & operator << (unsigned char data);
	CFormatHelper & operator << (__int64 data);
	CFormatHelper & operator << (unsigned __int64 data);
	CFormatHelper & operator << (float data);
	CFormatHelper & operator << (double data);
	CFormatHelper & operator << (long double data);
	CFormatHelper & operator << (const void *data);
	CFormatHelper & operator << (const char *data);
	CFormatHelper & operator << (int *data);

	const std::string &str() const;
	operator const std::string &() const;

private:
	CStringFormatter m_formatter;
};

typedef CFormatHelper Format;



// FORMAT: 定义缩写。
// 这是为了在外部使用方便才定义的。如果与其它的宏相冲突，可以随时取消定义或改名
// StringFormat库本身并没有使用这个宏
#define FORMAT(fmt) string_format::Format(fmt, __FILE__, __LINE__)

} // end of namespace string_format

#endif
