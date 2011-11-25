// StringFormat.h
// �ַ�����ʽ��������C���Ե�sprintf�������Ӱ�ȫ��
//
//
//
// ���ԣ�
// (1) ��ʹ�õĸ�ʽ��������%d, %f�ȣ������ܵ����׼C��ͬ������ʹ���ϰ�
// (2) �������ݵ����ͣ�ȷ��float������%d������ȵȣ���
// (3) ʹ��ʱ����ָ��buffer�Ĵ�С�������ڴ����
//
//
//
// ʹ�ã�
// (1) һ��ʹ��ʱ��ֻ��Ҫ���ϲ�ӿڼ��ɡ�
//     ʾ����std::string strResult = string_format::Format("today is %04d-%02d-%02d", __FILE__, __LINE__) << 2009 << 9 << 4;
//     ���ߣ�std::string strResult = FORMAT("today is %04d-%02d-%02d") << 2009 << 9 << 4;
// (2) �������Ϊ�������ݣ����Ҳ���Ҫ����const char *���ͣ��������ôд��
//           std::string strTemp = FORMAT("today is %04d-%02d-%02d") << 2009 << 9 << 4;
//           somefunc(strTemp.c_str());
//     ���ߣ�somefunc((FORMAT("today is %04d-%02d-%02d") << 2009 << 9 << 4).str().c_str());
//     ǧ��Ҫ����д��const char *pszTemp = (FORMAT("today is %04d-%02d-%02d") << 2009 << 9 << 4).str().c_str();
//             ������õ�һ��Ұָ�룬����������⡣
// (3) ����Ҫ����Ч��ʱ������ֱ��ʹ��CStringFormatter
//     ʾ����CStringFormatter fmt("today is %04d-%02d-%02d", __FILE__, __LINE__);
//           fmt.AddArgument(2009);
//           fmt.AddArgument(9);
//           fmt.AddArgument(4);
//           assert(fmt.AsString() == "today is 2009-09-04");
//     һ��CStringFormatter������Ա����������������ͬ�ĸ�ʽ���������Ľ������Ӷ�������ܡ�
//
//
//
// ʵ�֣�
// (1) �ڸ��������ʱ������C++��stringstream��
// (2) ���ڿ��ַ������ַ������ڵ�ǰ�汾����δʵ�֡�
//
//
//
// ��ʽ�����﷨�������ο���http://www.cplusplus.com/reference/clibrary/cstdio/sprintf/��
// ������ʵ�����������˹���%I64d������
//
//     %[flags][width][.precision][length]specifier
//     ����[]���������Ϊ��ѡ���ݣ�
//
// ���У�
//     flags������+, -, #, 0, ���߿ո���Щ���ݿ���������ϣ�˳���޹أ�������"%+08d"��ͬʱʹ����0��+��
//         +��ʾ��һ��������š�������3�������Ϊ+3��������������������Ч�����ַ����ַ�����Ч�����޷�������%u, %x, %X��Ҳ��Ч��
//         -��ʾ������롱��Ĭ���ǡ��Ҷ��롱��
//         #��ʾ��(1) ����%o, %x, %X�������ǰ���0, 0x, 0X
//                (2) ����%f, %e, %E�����һ������С����
//                (3) ����%g, %G�����һ������С���㣬���Ҳ����ĩβ����ȥ����Ĭ�ϻ�ȥ����
//         0��ʾ����Ȳ���ʱ���ַ�'0'��䣨�����Ҷ���ʱ��Ч����
//         �ո��ʾ���ԷǸ�ֵ����һ���ո��Ա��ڸ������Զ��루������������������Ч�����ַ����ַ�����Ч�����޷�������%u, %x, %X��Ҳ��Ч��
//     width��һ�����֣�����*����ʾ������������ٸ��ַ��������㲿���ÿո���䣨���flagsָ����0����0��䣩
//         ���width��*�����ʾ�Ӻ���Ĳ����������ȡ
//     precision��һ�����֣�����*����ʾ��
//         (1) ����������%d, %i, %o, %u, %x, %X����
//             ��ʾ������������ٸ����֡������㲿����0���
//         (2) ����%e, %E, %f����ʾС���������λ���������룩
//         (3) ����%g, %G����ʾ��༸λ��Ч���֣��������룩
//         (4) ����%s����ʾ�����������ַ�
//         (5) ����%c�����κ�Ч��
//         ���precision��*�����ʾ�Ӻ���Ĳ����������ȡ
//     length������h, l, L, I64
//         h��ʾ��������������%d, %i, %o, %u, %x, %X������short����unsigned short������
//         l��ʾ��������������%d, %i, %o, %u, %x, %X������long����unsigned long������
//                          ��%c, %s������wchar_t����������Ŀǰ�汾û��ʵ�֣�
//         L��ʾ������������������%f, %e, %E, %g, %G������long double������
//         I64��ʾ��64λ������%d������__int64����%u������unsigned __int64
//     specifier�������������ݣ�
//         c: �Ӳ���������ȡһ���ַ������
//         d: �Ӳ���������ȡһ�������ŵ���������ʮ�������
//         i: ͬ��
//         e: �Ӳ���������ȡһ��������������ѧ�����������������ָ��֮����e�ָ
//         E: �Ӳ���������ȡһ��������������ѧ�����������������ָ��֮����E�ָ
//         f: �Ӳ���������ȡһ������������ʮ�������
//         g: �Ӳ���������ȡһ��������������%e��%f���������ʽ֮����ѡ��ȡ�϶�����Ϊʵ���������Ĭ�ϻ�ȥ��ĩβ���㣩
//         G: �Ӳ���������ȡһ��������������%E��%f���������ʽ֮����ѡ��ȡ�϶�����Ϊʵ���������Ĭ�ϻ�ȥ��ĩβ���㣩
//         o: �Ӳ���������ȡһ�������ŵ����������˽������
//         s: �Ӳ���������ȡһ���ַ�ָ�룬����Ӹõ�ַ��ʼ���ַ���
//         u: �Ӳ���������ȡһ���޷��ŵ���������ʮ�������
//         x: �Ӳ���������ȡһ���޷��ŵ���������ʮ�������������ĸ��Сд��
//         X: �Ӳ���������ȡһ���޷��ŵ���������ʮ�������������ĸ�ô�д��
//         p: �Ӳ���������ȡһ���������͵�ָ�룬�������ʽ��ƽ̨���ж��壿��
//         n: ��������Ӳ���������ȡһ��int���͵�ָ�룬�ѡ���ǰ�Ѿ�������ַ��������õ�ָ����ָλ��
//         %: ���һ����%�����ٷֺţ��ַ�

#ifndef __TQ_STRINGFORMAT_H__
#define __TQ_STRINGFORMAT_H__

#include <string>



namespace string_format
{

// ������� 0��ʾ������û�д���򾯸� 1000~1999��ʾ���� 2000~2999��ʾ����
enum ErrorCode
{
	NoError						= 0,	// û�д���򾯸�

	Warning_TooManyArguments	= 1000, // ���棺����Ĳ�������

	Error_InvalidFormatString	= 2000, // ����pszFormat�Ƿ������½���ʧ��
	Error_InvalidArgumentType,			// ���󣺴���Ĳ������Ͳ�����Ҫ��
	Error_NotEnoughArguments,			// ���󣺴���Ĳ�������
	Error_NotImplemented,				// ������Ŀǰ�汾����δʵ��

	InternalError				= 3000, // �ڲ�����
};

// ErrorHandler: һ��ָ�����ͣ�����ָ�򡰴���������
// ����err��ʾ������롣�μ�enum ErrorCode
// ����pszMessage��ʾ�����ô����һ���ַ���
typedef void (*ErrorHandler) (ErrorCode err, const char *pszMessage, const char *pszFileName, int iFileLine);

// SetErrorHandler: ���ô���������
// ����������ʱ����ô˺����������п��Լ�¼log�����߲����쳣����Ҫ�Լ�catch��
// ע������ġ����á�����Ӱ����ȫ�ֵģ��µ����ûḲ�Ǿɵ����á�
// ������������ô˺�����������NULLΪ�������ô˺��������ʾ������Ĭ�ϵĴ�����
void SetErrorHandler(ErrorHandler handler);

// GetErrorHandler: ȡ�õ�ǰ�Ĵ��������� ���û�����ô����������򷵻�NULL
ErrorHandler GetErrorHandler();



// CStringFormatter: ����ʵ����
class CStringFormatter
{
public:
	CStringFormatter(const char *pszFormat, const char *pszFileName, int iFileLine);
	~ CStringFormatter();

	CStringFormatter(const CStringFormatter &copy);
	CStringFormatter &operator =(const CStringFormatter &copy);

	bool IsFormatValid() const;

	// �Ѹ�ʽ���õ�������std::string����ʽ���ء�
	// �ڲ����û��棬��˶�ε��ò����ж�ο�����
	// �ڵ��ôκ���֮ǰ����ȷ�������������Format���������������������ȷ����������ƥ�䣬��������ת��ErrorHandler
	const std::string &AsString() const;

	// ����Ѿ���������ݣ��Ա����½��и�ʽ��
	void ClearArguments();

	// �ṩ���ɰ汾�����أ��Ա��ܹ�������ֲ�ͬ����������
	// ˵��������C��׼��printf����������֧�ֿɱ䳤�����ģ��ڴ�����ʱ���������¹���
	//       ���ִ����ŵġ����������ͣ�����char, short, int, ����enum�ȡ��������������Ŷ����long long����__int64�����ڴ���ʱȫ��תΪlong����
	//       �����޷��ŵġ�������������֮���ƣ��ڴ���ʱȫ��תΪunsigned long����
	//       float�����ڴ���ʱȫ��תΪdouble����
	//       ���������long long��VC6��__int64����unsigned long long��VC6��unsigned __int64����long double���ڴ���ʱ����ԭ�����ݣ���ת��
	// Ϊ�ˣ����int�����غ����short��������ʵ��ʵ��ʱ��ȫһ�¡�
	// ���⣬VC6��wchar_t��ʵ����unsigned short����˲������ṩ����
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



// CFormatHelper: �ϲ�ӿ���
// ��FORMAT����ʹ������࣬Ч������
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



// FORMAT: ������д��
// ����Ϊ�����ⲿʹ�÷���Ŷ���ġ�����������ĺ����ͻ��������ʱȡ����������
// StringFormat�Ȿ��û��ʹ�������
#define FORMAT(fmt) string_format::Format(fmt, __FILE__, __LINE__)

} // end of namespace string_format

#endif
