#pragma warning(disable : 4786)

#include <vector>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <algorithm>

//#include "BaseFunc.h"	// �������ڳ���ʱ���μ�s_DefaultErrorHandler����������¼log��Ȼ��CHECK��û��������;��
						// ��ֲʱ�������Ҫlog����ɾȥ���Ժ��Ĺ�����Ӱ�졣

#include "StringFormat.h"

namespace string_format
{

static const std::string EMPTY_STRING = "";
static ErrorHandler s_ErrorHandler = NULL;
static const char* s_pszFileName = NULL;
static int s_iFileLine = 0;

// FORMAT_FLAGS���ڸ�ʽ���п������õı��
enum FORMAT_FLAGS
{
	// flags
	FORCE_SIGN_FLAG		= 1 << 0,	// flags�����ˡ�+��
	LEFT_ALIGN_FLAG		= 1 << 1,	// flags�����ˡ�-��
	MODIFY_STYLE_FLAG	= 1 << 2,	// flags�����ˡ�#��
	FILL_ZERO_FLAG		= 1 << 3,	// flags�����ˡ�0��
	RESERVE_SIGN_FLAG	= 1 << 4,	// flags�����ˡ� ��

	// length
	SHORT_INTEGER		= 1 << 8,	// length����Ϊ��h��
	LONG_INTEGER		= 1 << 9,	// length����Ϊ��l��
	LONG_DOUBLE			= 1 << 10,	// length����Ϊ��L��
	LONG64_INTEGER		= 1 << 11,	// length����Ϊ��I64��

	// ����
	USE_WIDTH			= 1 << 12,	// width����Ϊ����
	SPECIFY_WIDTH		= 1 << 13,	// width����Ϊ��*��
	USE_PRECISION		= 1 << 14,	// precision����Ϊ����
	SPECIFY_PRECISION	= 1 << 15,	// precision����Ϊ��*��
};

// FORMAT_DATA����ʽ������Ҫ��̬�滻�Ĳ��֣������ʽ�����ڴ�
// ����C��׼�����Է�Ϊ�ĸ����֣���˿����ĸ��ֶ�����¼��
struct FORMAT_DATA
{
	int iFlags; // ���õı�ǡ��μ�enum FORMAT_FLAGS
	char cSpecifier;
	int iWidth;
	int iPrecision;
};

// ARG_DATA_TYPE���������������͡�
// ˵��������C��׼��printf����������֧�ֿɱ䳤�����ģ��ڴ�����ʱ���������¹���
//       ���ִ����ŵġ����������ͣ�����char, short, int, ����enum�ȡ��������������Ŷ����long long����__int64�����ڴ���ʱȫ��תΪlong����
//       �����޷��ŵġ�������������֮���ƣ��ڴ���ʱȫ��תΪunsigned long����
//       float�����ڴ���ʱȫ��תΪdouble����
//       ���������long long��VC6��__int64����unsigned long long��VC6��unsigned __int64����long double���ڴ���ʱ����ԭ�����ݣ���ת��
// ��ˣ�û�б�Ҫ�����CHAR_TYPE, SHORT_TYPE, FLOAT_TYPE�ȣ�ʹ��INTEGER_TYPE, DOUBLE_TYPE���㹻��
enum ARG_DATA_TYPE
{
	INTEGER_TYPE		= 1,
	INT64_TYPE,
	DOUBLE_TYPE,
	LONGDOUBLE_TYPE,
	POINTER_TYPE,
	STRING_POINTER_TYPE,
	INTEGER_POINTER_TYPE,
};

// �������ݡ���һ��struct����һ��������
// ����һ��type�ֶα�ʾ�����ľ������ͣ�Ȼ��������ʲô���ͣ����������ݶ�����һ��union֮�С�
struct ARG_DATA
{
	ARG_DATA_TYPE type;
	union
	{
		long integerData;
		__int64 int64Data;
		double doubleData;
		long double longDoubleData;
		const void *pointerData;
		const char *stringPointerData;
		int *integerPointerData;
	};
};

// =====================================================================================================================
// =======================================================================================================================
void SetErrorHandler(ErrorHandler handler)
{
	s_ErrorHandler = handler;
}

// =====================================================================================================================
// =======================================================================================================================
ErrorHandler GetErrorHandler()
{
	return s_ErrorHandler;
}

// =====================================================================================================================
// =======================================================================================================================
static void s_DefaultErrorHandler(ErrorCode err, const char *pszMessage, const char *pszFileName, int iFileLine)
{
	if (!pszMessage) {
		pszMessage = "(empty)";
	}

	if (!pszFileName) {
		pszFileName = "(unknown)";
	}

//	LogMsg("StringFormat error. message: %s, at file: %s, line: %d", pszMessage, pszFileName, iFileLine);
//	DebugMsg("StringFormat error. message: %s, at file: %s, line: %d", pszMessage, pszFileName, iFileLine);
//	CHECK(0); // �������󡣾������οɲ鿴log
}

// =====================================================================================================================
// =======================================================================================================================
void s_RaiseError(ErrorCode err, const char *pszMessage)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ErrorHandler handler = s_ErrorHandler;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (!handler) {
		handler = s_DefaultErrorHandler;
	}

	handler(err, pszMessage, s_pszFileName, s_iFileLine);
}

// =====================================================================================================================
//    CFormatParser����ʽ�������� ����һ��û��״̬��class������ͨ��һЩ��Ա���������в����� ֻ��һ��public������
// =====================================================================================================================
class CFormatParser
{
public:
	// ����һ����ʽ��������ʽ���ԡ�%����ͷ�������ʽ�μ�����ʽ�����﷨����
	// �������ʧ�ܣ�����-1�� ��������ɹ������ؽ���ʱ����ȡ���ַ�������
	// �����ɹ�ʱ��pFormatData�б�������ȷ�Ľ��������
	int Parse(const char *pszFormat, FORMAT_DATA *pFormatData) const;
private:
	// ����flags���֡� ����˵��������ֵ˵����Parse������ͬ��
	// ����flags�����ǿ���ȱʡ�ģ���˷���ֵ����Ϊ��
	int ParseFlags(const char *pszFormat, FORMAT_DATA *pFormatData) const;

	// ����width���֡� ����˵��������ֵ˵����Parse������ͬ��
	// ����width�����ǿ���ȱʡ�ģ���˷���ֵ����Ϊ��
	int ParseWidth(const char *pszFormat, FORMAT_DATA *pFormatData) const;

	// ����precision���֡� ����˵��������ֵ˵����Parse������ͬ��
	// ����precision�����ǿ���ȱʡ�ģ���˷���ֵ����Ϊ��
	int ParsePrecision(const char *pszFormat, FORMAT_DATA *pFormatData) const;

	// ����length���֡� ����˵��������ֵ˵����Parse������ͬ��
	// ����length�����ǿ���ȱʡ�ģ���˷���ֵ����Ϊ��
	int ParseLength(const char *pszFormat, FORMAT_DATA *pFormatData) const;
};

// =====================================================================================================================
// =======================================================================================================================
int CFormatParser::Parse(const char *pszFormat, FORMAT_DATA *pFormatData) const
{
	if (!pszFormat || *pszFormat != '%' || !pFormatData) {
		return -1;
	}

	memset(pFormatData, 0, sizeof(FORMAT_DATA));

	//~~~~~~~~~~~~
	int iOffset = 1;
	int iResult;
	//~~~~~~~~~~~~

	iResult = ParseFlags(pszFormat + iOffset, pFormatData);
	iOffset += iResult;
	if (iResult < 0) {
		return -1;
	}

	iResult = ParseWidth(pszFormat + iOffset, pFormatData);
	iOffset += iResult;
	if (iResult < 0) {
		return -1;
	}

	iResult = ParsePrecision(pszFormat + iOffset, pFormatData);
	iOffset += iResult;
	if (iResult < 0) {
		return -1;
	}

	iResult = ParseLength(pszFormat + iOffset, pFormatData);
	iOffset += iResult;
	if (iResult < 0) {
		return -1;
	}

	switch (pszFormat[iOffset]) {
	case 'c':
	case 'd':
	case 'u':
	case 'i':
	case 'o':
	case 'x':
	case 'X':
	case 'f':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
	case 'p':
	case 's':
	case 'n':
		pFormatData->cSpecifier = pszFormat[iOffset];
		++iOffset;
		return iOffset;
		break;
	default:
		return -1;
		break;
	}
}

// =====================================================================================================================
// =======================================================================================================================
int CFormatParser::ParseFlags(const char *pszFormat, FORMAT_DATA *pFormatData) const
{
	if (!pszFormat || !pFormatData) {
		return -1;
	}

	for (int iOffset = 0;; ++iOffset) {
		switch (pszFormat[iOffset]) {
		case '+':	pFormatData->iFlags |= FORCE_SIGN_FLAG; break;
		case '-':	pFormatData->iFlags |= LEFT_ALIGN_FLAG; break;
		case '#':	pFormatData->iFlags |= MODIFY_STYLE_FLAG; break;
		case '0':	pFormatData->iFlags |= FILL_ZERO_FLAG; break;
		case ' ':	pFormatData->iFlags |= RESERVE_SIGN_FLAG; break;
		default:	return iOffset; break;
		}
	}
}

// =====================================================================================================================
// =======================================================================================================================
int CFormatParser::ParseWidth(const char *pszFormat, FORMAT_DATA *pFormatData) const
{
	if (!pszFormat || !pFormatData) {
		return -1;
	}

	//~~~~~~~~~~~~~~~~
	char c = *pszFormat;
	//~~~~~~~~~~~~~~~~

	if (c == '*') {
		pFormatData->iFlags |= SPECIFY_WIDTH;
		return 1;
	} else if (c >= '0' && c <= '9') {
		pFormatData->iFlags |= USE_WIDTH;

		//~~~~~~~~~~~
		int iWidth = 0;
		//~~~~~~~~~~~

		for (int iOffset = 0;; ++iOffset) {
			c = pszFormat[iOffset];
			if (c >= '0' && c <= '9') {
				iWidth *= 10;
				iWidth += (c - '0');
			} else {
				pFormatData->iWidth = iWidth;
				return iOffset;
			}
		}
	}

	return 0;
}

// =====================================================================================================================
// =======================================================================================================================
int CFormatParser::ParsePrecision(const char *pszFormat, FORMAT_DATA *pFormatData) const
{
	if (!pszFormat || !pFormatData) {
		return -1;
	}

	//~~~~~~~~~~~~~~~~
	char c = *pszFormat;
	//~~~~~~~~~~~~~~~~

	if (c != '.') {
		return 0;
	}

	//~~~~~~~~~~~~
	int iOffset = 1;
	//~~~~~~~~~~~~

	c = pszFormat[iOffset];

	if (c == '*') {
		pFormatData->iFlags |= SPECIFY_PRECISION;
		return 2;
	} else if (c >= '0' && c <= '9') {
		pFormatData->iFlags |= USE_PRECISION;

		//~~~~~~~~~~~~~~~
		int iPrecision = 0;
		//~~~~~~~~~~~~~~~

		for (;; ++iOffset) {
			c = pszFormat[iOffset];
			if (c >= '0' && c <= '9') {
				iPrecision *= 10;
				iPrecision += (c - '0');
			} else {
				pFormatData->iPrecision = iPrecision;
				return iOffset;
			}
		}
	}

	return 0;
}

// =====================================================================================================================
// =======================================================================================================================
int CFormatParser::ParseLength(const char *pszFormat, FORMAT_DATA *pFormatData) const
{
	if (!pszFormat || !pFormatData) {
		return -1;
	}

	//~~~~~~~~~~~~~~~~
	char c = *pszFormat;
	//~~~~~~~~~~~~~~~~

	switch (c) {
	case 'h':
		pFormatData->iFlags |= SHORT_INTEGER;
		return 1;
		break;
	case 'l':
		pFormatData->iFlags |= LONG_INTEGER;
		return 1;
		break;
	case 'L':
		pFormatData->iFlags |= LONG_DOUBLE;
		return 1;
		break;
	case 'I':
		if (strncmp(pszFormat, "I64", 3) == 0) {
			pFormatData->iFlags |= LONG64_INTEGER;
			return 3;
		}

		return 0;
		break;
	default:
		return 0;
		break;
	}
}

// =====================================================================================================================
//    ��������š�
//    t: Ҫ���������
//    iFlags: �μ�FORMAT_FLAGS
//    pStrOutput: Ҫ�����λ��
// =====================================================================================================================
bool s_OutputSign(__int64 t, int iFlags, std::string *pStrOutput)
{
	if (!pStrOutput) {
		return false;
	}

	if (t < 0) {
		(*pStrOutput) += '-';
	} else if (t > 0) {
		if (iFlags & RESERVE_SIGN_FLAG) {
			(*pStrOutput) += ' ';
		} else if (iFlags & FORCE_SIGN_FLAG) {
			(*pStrOutput) += '+';
		}
	} else {
		if (iFlags & RESERVE_SIGN_FLAG) {
			(*pStrOutput) += ' ';
		}
	}

	return true;
}

// =====================================================================================================================
//    ��������Ϊ�ַ��������
//    t: Ҫ���������
//    iRadix: ���ơ�һ������Ϊ8, 10, 16���ֱ��ʾ�˽���/ʮ����/ʮ������
//    iPrecision: ����Ҫ��������ָ����������������
//    pDigits: ʹ�õ����֡�����ʮ�����ƣ����Դ���"0123456789abcdef"����"0123456789ABCDEF"
//    pStrOutput: Ҫ�����λ��
// =====================================================================================================================
bool s_OutputInteger_Unsigned(unsigned __int64 t,
							  int iRadix,
							  int iPrecision,
							  const char *pDigits,
							  std::string *pStrOutput)
{
	if (!pDigits || !pStrOutput) {
		return false;
	}

	//~~~~~~~~~~~~
	char szBuf[256];
	char *p = szBuf;
	//~~~~~~~~~~~~

	if (t == 0) {
		*p++ = pDigits[0];
	}

	while (t != 0) {
		*p++ = pDigits[t % iRadix];
		t /= iRadix;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const int iActualLength = p - szBuf;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (iPrecision > iActualLength) {
		pStrOutput->append(iPrecision - iActualLength, '0');
	}

	// VC6��std::string��֧��push_back�������������޷�����
	// std::reverse_copy(szBuf, p, std::back_inserter(strBuf));
	// ���ã�
	std::reverse(szBuf, p);
	pStrOutput->append(szBuf, p);

	return true;
}

class CStringFormatter:: CImpl
{
public:
	CImpl(const char *pszFormat);

	bool IsFormatValid() const;

	// �μ�CStringFormatter::AsString
	const std::string & AsString();

	// �μ�CStringFormatter::ClearArguments
	void ClearArguments();

	// �μ�CStringFormatter::AddArgument
	void AddArgument_Integer(long data);
	void AddArgument_Int64(__int64 data);
	void AddArgument_Double(double data);
	void AddArgument_LongDouble(long double data);
	void AddArgument_Pointer(const void *data);
	void AddArgument_SPointer(const char *data);
	void AddArgument_IPointer(int *data);
private:
	enum FORMAT_TYPE
	{
		STATIC_FORMAT,			// ԭ�����������"Hello, World."���Ϊ"Hello, World."
		DYNAMIC_FORMAT,			// ��̬���������"%d"�п������Ϊ"3025"
	};

	typedef std::vector<FORMAT_DATA> FORMAT_DATA_VEC;
	typedef std::vector<FORMAT_TYPE> FORMAT_TYPE_VEC;
	typedef std::vector<ARG_DATA> ARG_DATA_VEC;
	typedef std::vector<std::string> STRING_VEC;

	bool m_bFormatValid;		// ��ʽ���Ƿ���Ч
	bool m_bResultValid;		// ��������ս��m_strResult�Ƿ���Ч
	std::string m_strFormat;	// ��pszFormat����һ�ݣ�����ʱ�۲���ӷ��㡣û�������ô���
	std::string m_strResult;	// ��������ս����
	FORMAT_TYPE_VEC m_vecFormatType;	// ��i���ɷ��Ƕ�̬���Ǿ�̬
	FORMAT_DATA_VEC m_vecFormatData;	// ��%d, %.2f�����ݽ����ã����浽��
	ARG_DATA_VEC m_vecArgData;			// ͨ��AddArgument_XXXϵ�к�����ӵ����ݣ����浽��
	STRING_VEC m_vecSplitedString;		// ��Ҫԭ������Ĵ�������"Hello, %s!"��������vecSplitedString = {"Hello, ", "!"}

	// ������ʽ���������Ƿ�����ɹ������浽m_vecFormatType, m_vecFormatData, m_vecSplitedString����
	// �˺�����ʵ���Է������ã���������״̬����
	// �Ժ���ܽ����캯���еĲ���ȥ�������Ѵ˺�����Ϊpublic
	bool ParseFormatString(const char *pszFormat);

	// ���ݽ����õĸ�ʽ������������Ĳ������������ս�������浽m_strResult��������m_bResultValid��
	void CalcResult();

	// ����һ��%d����%f�����������ӵ�strCurrent��ĩβ
	bool CalcFormat(const FORMAT_DATA & formatData, const ARG_DATA & arg, std::string & strCurrent);
};

// =====================================================================================================================
// =======================================================================================================================
CStringFormatter::CImpl::CImpl(const char *pszFormat)
	: m_bFormatValid(false)
	, m_bResultValid(false)
{
	if (NULL == pszFormat) {
		pszFormat = "";
	}

	if (!ParseFormatString(pszFormat)) {

		//~~~~~~~~~~~~~~~~~
		// ���󣺽�����ʽ��ʧ��
		std::string strError;
		//~~~~~~~~~~~~~~~~~

		strError += "invalid format string: \"";
		strError += pszFormat;
		strError += "\"";
		s_RaiseError(Error_InvalidFormatString, strError.c_str());
	}
}

// =====================================================================================================================
// =======================================================================================================================
bool CStringFormatter::CImpl::IsFormatValid() const
{
	return m_bFormatValid;
}

// =====================================================================================================================
// =======================================================================================================================
const std::string & CStringFormatter::CImpl::AsString()
{
	CalcResult();
	return m_strResult;
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::ClearArguments()
{
	m_bResultValid = false;
	m_strResult = std::string();	// VC6��std::stringû��clear����
	m_vecArgData.clear();

	// �Ż�������ʽ����û���κ�%ʱ�����ս������ֱ��ȷ��
	if (m_vecFormatData.empty()) {
		m_strResult = std::string();
		for (std::vector<std::string>::const_iterator it = m_vecSplitedString.begin(); it != m_vecSplitedString.end(); ++it) {
			m_strResult += *it;
		}

		m_bResultValid = true;
	}
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::AddArgument_Integer(long data)
{
	//~~~~~~~~~
	ARG_DATA arg;
	//~~~~~~~~~

	arg.type = INTEGER_TYPE;
	arg.integerData = data;
	m_vecArgData.push_back(arg);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::AddArgument_Int64(__int64 data)
{
	//~~~~~~~~~
	ARG_DATA arg;
	//~~~~~~~~~

	arg.type = INT64_TYPE;
	arg.int64Data = data;
	m_vecArgData.push_back(arg);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::AddArgument_Double(double data)
{
	//~~~~~~~~~
	ARG_DATA arg;
	//~~~~~~~~~

	arg.type = DOUBLE_TYPE;
	arg.doubleData = data;
	m_vecArgData.push_back(arg);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::AddArgument_LongDouble(long double data)
{
	//~~~~~~~~~
	ARG_DATA arg;
	//~~~~~~~~~

	arg.type = LONGDOUBLE_TYPE;
	arg.longDoubleData = data;
	m_vecArgData.push_back(arg);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::AddArgument_Pointer(const void *data)
{
	//~~~~~~~~~
	ARG_DATA arg;
	//~~~~~~~~~

	arg.type = POINTER_TYPE;
	arg.pointerData = data;
	m_vecArgData.push_back(arg);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::AddArgument_SPointer(const char *data)
{
	//~~~~~~~~~
	ARG_DATA arg;
	//~~~~~~~~~

	arg.type = STRING_POINTER_TYPE;
	arg.stringPointerData = data;
	m_vecArgData.push_back(arg);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::AddArgument_IPointer(int *data)
{
	//~~~~~~~~~
	ARG_DATA arg;
	//~~~~~~~~~

	arg.type = INTEGER_POINTER_TYPE;
	arg.integerPointerData = data;
	m_vecArgData.push_back(arg);
}

// =====================================================================================================================
// =======================================================================================================================
bool CStringFormatter::CImpl::ParseFormatString(const char *pszFormat)
{
	m_bFormatValid = false;
	m_bResultValid = false;
	m_strFormat = std::string();	// VC6��std::stringû��clear����
	m_strResult = std::string();	// VC6��std::stringû��clear����
	m_vecArgData.clear();
	m_vecFormatType.clear();
	m_vecFormatData.clear();
	m_vecSplitedString.clear();

	if (!pszFormat) {
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const char *pszFormatBegin = pszFormat;
	const char *pszFormatEnd = pszFormat + strlen(pszFormat);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	while (pszFormat < pszFormatEnd) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		const char *p = strchr(pszFormat, '%');
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (!p) {
			m_vecFormatType.push_back(STATIC_FORMAT);
			m_vecSplitedString.push_back(std::string(pszFormat));
			break;
		}

		if (p != pszFormat) {
			m_vecFormatType.push_back(STATIC_FORMAT);
			m_vecSplitedString.push_back(std::string(pszFormat, p));
			pszFormat = p;
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		CFormatParser parser;
		FORMAT_DATA formatData;
		int iParseResult = parser.Parse(pszFormat, &formatData);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (iParseResult < 0) {
			++pszFormat;

			if (*pszFormat == '%') {
				m_vecFormatType.push_back(STATIC_FORMAT);
				m_vecSplitedString.push_back("%");
				++pszFormat;
			}

			continue;
		}

		pszFormat += iParseResult;
		m_vecFormatType.push_back(DYNAMIC_FORMAT);
		m_vecFormatData.push_back(formatData);
	}

	if (m_vecFormatType.size() != m_vecSplitedString.size() + m_vecFormatData.size()) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return false;
	}

	// �Ż�������ʽ����û���κ�%ʱ�����ս������ֱ��ȷ��
	if (m_vecFormatData.empty()) {
		m_strResult = std::string();
		for (std::vector<std::string>::const_iterator it = m_vecSplitedString.begin(); it != m_vecSplitedString.end(); ++it) {
			m_strResult += *it;
		}

		m_bResultValid = true;
	}

	m_strFormat = pszFormatBegin;
	m_bFormatValid = true;
	return true;
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::CImpl::CalcResult()
{
	// ����������Ѿ������浽m_strResult�У���ֱ�ӷ��أ��������м���
	if (m_bResultValid) {
		return;
	}

	// �����ʽ���д����޷��õ���ȷ�����ֱ�ӷ���
	if (!m_bFormatValid) {
		return;
	}

	//~~~~~~~~~~~~~~~~~~~~~
	// ��ʱ�ļ�����
	std::string strResult;
	int idxFormatData = 0;
	int idxSplitedString = 0;
	int idxArgData = 0;
	//~~~~~~~~~~~~~~~~~~~~~

	for (FORMAT_TYPE_VEC::const_iterator it = m_vecFormatType.begin(); it != m_vecFormatType.end(); ++it) {
		switch (*it) {
		case STATIC_FORMAT:
			if (idxSplitedString >= static_cast<int>(m_vecSplitedString.size())) {
				s_RaiseError(InternalError, "Internal Error");	// we should never reach here
				return;
			}

			strResult += m_vecSplitedString[idxSplitedString];
			++idxSplitedString;
			break;
		case DYNAMIC_FORMAT:
			if (idxFormatData >= static_cast<int>(m_vecFormatData.size())) {
				s_RaiseError(InternalError, "Internal Error");	// we should never reach here
				return;
			}
			{
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				FORMAT_DATA formatData = m_vecFormatData[idxFormatData];
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

				++idxFormatData;

				if (formatData.iFlags & SPECIFY_WIDTH) {
					if (idxArgData >= static_cast<int>(m_vecArgData.size())) {

						// ���󣺴���Ĳ�����������
						s_RaiseError(Error_NotEnoughArguments, "not enough arguments");
						return;
					}

					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					const ARG_DATA &arg = m_vecArgData[idxArgData];
					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

					++idxArgData;

					if (arg.type != INTEGER_TYPE) {

						// ���󣺴���Ĳ������ʹ�����Ҫһ������
						s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect an integer");
						return;
					}

					formatData.iFlags &= ~SPECIFY_WIDTH;
					formatData.iFlags |= USE_WIDTH;
					formatData.iWidth = arg.integerData;
				}

				if (formatData.iFlags & SPECIFY_PRECISION) {
					if (idxArgData >= static_cast<int>(m_vecArgData.size())) {

						// ���󣺴���Ĳ�����������
						s_RaiseError(Error_NotEnoughArguments, "not enough arguments");
						return;
					}

					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					const ARG_DATA &arg = m_vecArgData[idxArgData];
					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

					++idxArgData;

					if (arg.type != INTEGER_TYPE) {

						// ���󣺴���Ĳ������ʹ�����Ҫһ������
						s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect an integer");
						return;
					}

					formatData.iFlags &= ~SPECIFY_PRECISION;
					formatData.iFlags |= USE_PRECISION;
					formatData.iPrecision = arg.integerData;
				}

				if (idxArgData >= static_cast<int>(m_vecArgData.size())) {

					// ���󣺴���Ĳ�����������
					s_RaiseError(Error_NotEnoughArguments, "not enough arguments");
					return;
				}

				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				const ARG_DATA &arg = m_vecArgData[idxArgData];
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

				++idxArgData;
				if (!CalcFormat(formatData, arg, strResult)) {
					return;
				}
			}
			break;
		default:
			s_RaiseError(InternalError, "Internal Error");		// we should never reach here
			return;
			break;
		}	// end of switch
	}		// end of for

	if (idxArgData < static_cast<int>(m_vecArgData.size())) {

		// ���棺����Ĳ����������ࣨ�������ʵ�ʴ���
		s_RaiseError(Warning_TooManyArguments, "too many arguments");
	}

	// ����ʱ���strResult����Ϊ���ս��m_strResult
	// ʹ��swap����ʹ�ø�ֵ����Ϊ�˸��ߵ�Ч�ʡ�����swap��������Ϊ�ڴ治���ʧ�ܵĿ����ԡ�
	m_strResult.swap(strResult);
	m_bResultValid = true;
}

// =====================================================================================================================
// =======================================================================================================================
bool CStringFormatter::CImpl::CalcFormat(const FORMAT_DATA &formatData, const ARG_DATA &arg, std::string &strCurrent)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::string strBuf;
	std::string strPrefix;
	const int iFlags = formatData.iFlags;
	const char cSpecifier = formatData.cSpecifier;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// ����������������ͬ����
	switch (cSpecifier) {
	case 'c':
		if (arg.type != INTEGER_TYPE) {

			// ���󣺴���Ĳ������ʹ�����Ҫһ������
			s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect an integer");
			return false;
		}

		if (iFlags & LONG_INTEGER) {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			wchar_t wc = (wchar_t) arg.integerData;
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			// ������ʱû��ʵ��%lc
			s_RaiseError(Error_NotImplemented, "wide-char not supported yet");
			return false;
		} else {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~
			char c = (char)arg.integerData;
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~

			strBuf += c;
		}
		break;
	case 's':
		// ��ʾ������0����ת��Ϊ��ָ�롣����ж��е�ǣǿ�����ǿ���ȷ����ȷ�Ĵ����ܹ���ȷ������
		if (arg.type != STRING_POINTER_TYPE && !(arg.type == INTEGER_TYPE && arg.integerData == 0)) {

			// ���󣺴���Ĳ������ʹ�����Ҫһ���ַ�ָ��
			s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect a string-pointer");
			return false;
		}

		if (iFlags & LONG_INTEGER) {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			const wchar_t *ws = (const wchar_t *)arg.stringPointerData;
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			if (!ws) {
				ws = L"(null)";
			}

			// ������ʱû��ʵ��%ls
			s_RaiseError(Error_NotImplemented, "wide-char string not supported yet");
			return false;
		} else {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			const char *s = (const char *)arg.stringPointerData;
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			if (!s) {
				s = "(null)";
			}

			//~~~~~~~~~~~~~~
			const char *s_end;
			//~~~~~~~~~~~~~~

			if (iFlags & USE_PRECISION) {
				s_end = std::find(s, s + formatData.iPrecision, '\0');
			} else {
				s_end = s + strlen(s);
			}

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			const int iActualWidth = s_end - s;
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			if ((iFlags & USE_WIDTH) && formatData.iWidth > iActualWidth) {

				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				const int iFill = formatData.iWidth - iActualWidth;
				const char cFill = (formatData.iFlags & FILL_ZERO_FLAG) ? '0' : ' ';
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

				if (iFlags & LEFT_ALIGN_FLAG) {
					strCurrent.append(s, s_end);
					strCurrent.append(iFill, cFill);
				} else {
					strCurrent.append(iFill, cFill);
					strCurrent.append(s, s_end);
				}
			} else {
				strCurrent.append(s, s_end);
			}

			return true;
		}
		break;
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		{
			if ((iFlags & LONG64_INTEGER) && arg.type != INT64_TYPE) {

				// ���󣺴���Ĳ������ʹ�����Ҫһ��64λ����
				s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect a 64-bit integer");
				return false;
			}

			if (!(iFlags & LONG64_INTEGER) && arg.type != INTEGER_TYPE) {

				// ���󣺴���Ĳ������ʹ�����Ҫһ������
				s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect an integer");
				return false;
			}

			//~~~~~~~~~~~~~~~~
			// �жϽ���
			int iRadix;
			const char *pDigits;
			//~~~~~~~~~~~~~~~~

			switch (cSpecifier) {
			case 'o':	iRadix = 8; pDigits = "01234567"; break;
			case 'x':	iRadix = 16; pDigits = "0123456789abcdef"; break;
			case 'X':	iRadix = 16; pDigits = "0123456789ABCDEF"; break;
			default:	iRadix = 10; pDigits = "0123456789"; break;
			}

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			// �������
			const int iPrecision = (iFlags & USE_PRECISION) ? formatData.iPrecision : -1;
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			switch (cSpecifier) {
			case 'i':
			case 'd':
			case 'o':
				if (iFlags & LONG64_INTEGER) {

					//~~~~~~~~~~~~~~~~~~~~~~~~~
					__int64 data = arg.int64Data;
					//~~~~~~~~~~~~~~~~~~~~~~~~~

					if (!s_OutputSign(data, iFlags, &strPrefix)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}

					if (!s_OutputInteger_Unsigned((unsigned __int64)(data >= 0 ? data : -data), iRadix, iPrecision,
						pDigits, &strBuf)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}
				} else if (iFlags & LONG_INTEGER) {

					//~~~~~~~~~~~~~~~~~~~~~~~~
					long data = arg.integerData;
					//~~~~~~~~~~~~~~~~~~~~~~~~

					if (!s_OutputSign(data, iFlags, &strPrefix)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}

					if (!s_OutputInteger_Unsigned((unsigned long)(data >= 0 ? data : -data), iRadix, iPrecision,
						pDigits, &strBuf)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}
				} else if (iFlags & SHORT_INTEGER) {

					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					short data = (short)arg.integerData;
					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

					if (!s_OutputSign(data, iFlags, &strPrefix)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}

					if (!s_OutputInteger_Unsigned((unsigned short)(data >= 0 ? data : -data), iRadix, iPrecision,
						pDigits, &strBuf)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}
				} else {

					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					int data = (int)arg.integerData;
					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~

					if (!s_OutputSign(data, iFlags, &strPrefix)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}

					if (!s_OutputInteger_Unsigned((unsigned int)(data >= 0 ? data : -data), iRadix, iPrecision, pDigits,
						&strBuf)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}
				}
				break;
			case 'u':
			case 'x':
			case 'X':
				if (iFlags & LONG64_INTEGER) {
					if (!s_OutputInteger_Unsigned((unsigned __int64)arg.int64Data, iRadix, iPrecision, pDigits, &strBuf
						)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}
				} else if (iFlags & LONG_INTEGER) {
					if (!s_OutputInteger_Unsigned((unsigned long)arg.int64Data, iRadix, iPrecision, pDigits, &strBuf)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}
				} else if (iFlags & SHORT_INTEGER) {
					if (!s_OutputInteger_Unsigned((unsigned short)arg.int64Data, iRadix, iPrecision, pDigits, &strBuf
						)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}
				} else {
					if (!s_OutputInteger_Unsigned((unsigned int)arg.int64Data, iRadix, iPrecision, pDigits, &strBuf)) {
						s_RaiseError(InternalError, "Internal Error");	// we should never reach here
						return false;
					}
				}
				break;
			default:
				s_RaiseError(InternalError, "Internal Error");	// we should never reach here
				return false;
				break;
			}

			// ����ʹ����'#'�����Σ�Ϊ�˽���/ʮ������д��0, 0x, 0X
			if (iFlags & MODIFY_STYLE_FLAG) {
				switch (cSpecifier) {
				case 'o':	strPrefix += '0'; break;
				case 'x':	strPrefix += '0'; strPrefix += 'x'; break;
				case 'X':	strPrefix += '0'; strPrefix += 'X'; break;
				default:	// ignore '#' flag
					break;
				}
			}
		}
		break;
	case 'f':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
		if ((iFlags & LONG_DOUBLE) && arg.type != LONGDOUBLE_TYPE) {

			// ���󣺴���Ĳ������ʹ�����Ҫһ��long double
			s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect a long double");
			return false;
		}

		if (!(iFlags & LONG_DOUBLE) && arg.type != DOUBLE_TYPE) {

			// ���󣺴���Ĳ������ʹ�����Ҫһ��������
			s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect a float/double");
			return false;
		}
		{
			//~~~~~~~~~~~~~~~~~~~~~~
			std::ostringstream stream;
			//~~~~~~~~~~~~~~~~~~~~~~

			if (iFlags & USE_WIDTH) {
				stream.width(formatData.iWidth);
			}

			if (iFlags & USE_PRECISION) {
				stream.precision(formatData.iPrecision);
			}

			if (iFlags & FORCE_SIGN_FLAG) {
				stream.setf(std::ios::showpos);
			}

			if (iFlags & LEFT_ALIGN_FLAG) {
				stream.setf(std::ios::left);
			}

			if (iFlags & FILL_ZERO_FLAG) {
				stream.fill('0');
			}

			if (iFlags & MODIFY_STYLE_FLAG) {
				stream.setf(std::ios::showpoint);
			}

			if (cSpecifier == 'E' || formatData.cSpecifier == 'G') {
				stream.setf(std::ios::uppercase);
			}

			if (cSpecifier == 'e' || formatData.cSpecifier == 'E') {
				stream.setf(std::ios::scientific);
			}

			if (cSpecifier == 'f') {
				stream.setf(std::ios::fixed);
			}

			if (iFlags & LONG_DOUBLE) {
				stream << arg.longDoubleData;
			} else {
				stream << arg.doubleData;
			}

			strBuf = stream.str();
		}
		break;
	case 'p':
		{
			//~~~~~~~~~~~~
			const void *ptr;
			//~~~~~~~~~~~~

			switch (arg.type) {
			case POINTER_TYPE:
				ptr = arg.pointerData;
				break;
			case STRING_POINTER_TYPE:
				ptr = arg.stringPointerData;
				break;
			case INTEGER_POINTER_TYPE:
				ptr = arg.integerPointerData;
				break;
			case INTEGER_TYPE:	// ��ʾ������0����ת��Ϊ��ָ�롣����ж��е�ǣǿ�����ǿ���ȷ����ȷ�Ĵ����ܹ���ȷ������
				if (arg.integerData != 0) {

					// ���󣺴���Ĳ������ʹ�����Ҫһ��ָ��
					s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect a pointer");
					return false;
				}

				ptr = NULL;
				break;
			default:
				// ���󣺴���Ĳ������ʹ�����Ҫһ��ָ��
				s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect a pointer");
				return false;
				break;
			}

			if (!s_OutputInteger_Unsigned((unsigned long)ptr, 16, 8, "0123456789ABCDEF", &strBuf)) {
				s_RaiseError(InternalError, "Internal Error");	// we should never reach here
				return false;
			}
		}
		break;
	case 'n':
		{
			//~~~~~
			int *ptr;
			//~~~~~

			switch (arg.type) {
			case INTEGER_POINTER_TYPE:
				ptr = arg.integerPointerData;
				break;
			case INTEGER_TYPE:	// ��ʾ������0����ת��Ϊ��ָ�롣����ж��е�ǣǿ�����ǿ���ȷ����ȷ�Ĵ����ܹ���ȷ������
				if (arg.integerData != 0) {

					// ���󣺴���Ĳ������ʹ�����Ҫһ��intָ��
					s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect a pointer");
					return false;
				}

				ptr = NULL;
				break;
			default:
				// ���󣺴���Ĳ������ʹ�����Ҫһ��intָ��
				s_RaiseError(Error_InvalidArgumentType, "invalid argument type. expect a pointer");
				return false;
				break;
			}

			if (ptr) {
				*(ptr) = (int)strCurrent.length();
			}

			return true;
		}
		break;
	case '%':
		strBuf += '%'; // not used yet, cSpecifier never equal to '%', see CFormatParser::Parse
		break;
	default:
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return false;
		break;
	} // end of switch

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// ����ʵ�����ݡ�
	// ����iWidth�����ʵ�ʵ��ַ���������ָ���Ŀ�ȣ�����䡣
	// ���ʱҪ���Ƕ��뷽ʽ�����������룬����д��ʽ���ݣ�����䣻������Ҷ��룬������䣬��д��ʽ���ݡ�
	// ��ע���Ҷ���ʱ������Կո���䣬�����������������֮�����������䣬�����������������֮ǰ��
	const int iActualWidth = (int)strBuf.length() + (int)strPrefix.length();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if ((iFlags & USE_WIDTH) && formatData.iWidth > iActualWidth) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		const int iFillNum = formatData.iWidth - iActualWidth;
		const bool bFillZero = (formatData.iFlags & FILL_ZERO_FLAG) && !(formatData.iFlags & LEFT_ALIGN_FLAG);
		const char cFillChar = bFillZero ? '0' : ' ';
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (iFlags & LEFT_ALIGN_FLAG) {
			strCurrent.append(strPrefix);
			strCurrent.append(strBuf);
			strCurrent.append(iFillNum, cFillChar);
		} else {
			if (formatData.iFlags & FILL_ZERO_FLAG) {
				strCurrent.append(strPrefix);
				strCurrent.append(iFillNum, cFillChar);
				strCurrent.append(strBuf);
			} else {
				strCurrent.append(iFillNum, cFillChar);
				strCurrent.append(strPrefix);
				strCurrent.append(strBuf);
			}
		}
	} else {
		strCurrent.append(strPrefix);
		strCurrent.append(strBuf);
	}

	return true;
}

// =====================================================================================================================
// =======================================================================================================================
CStringFormatter::CStringFormatter(const char *pszFormat, const char *pszFileName, int iFileLine)
	: m_pImpl(new CImpl(pszFormat))
{
	s_pszFileName = pszFileName;
	s_iFileLine = iFileLine;
}

// =====================================================================================================================
// =======================================================================================================================
CStringFormatter::~CStringFormatter()
{
	try
	{
		delete m_pImpl;
	}
	catch(...) {

		// eat all exceptions
	}
}

// =====================================================================================================================
// =======================================================================================================================
CStringFormatter::CStringFormatter(const CStringFormatter &copy)
	: m_pImpl(new CImpl(*copy.m_pImpl))
{
}

// =====================================================================================================================
// =======================================================================================================================
CStringFormatter &CStringFormatter::operator=(const CStringFormatter &copy)
{
	if (&copy != this) {

		//~~~~~~~~~~~~~~~~~~~~~~
		CImpl *pOldImpl = m_pImpl;
		//~~~~~~~~~~~~~~~~~~~~~~

		if (copy.m_pImpl) {
			m_pImpl = new CImpl(*copy.m_pImpl);
		} else {
			m_pImpl = NULL;
			s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		}

		delete pOldImpl;
	}

	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
bool CStringFormatter::IsFormatValid() const
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return false;
	}

	return m_pImpl->IsFormatValid();
}

// =====================================================================================================================
// =======================================================================================================================
const std::string & CStringFormatter::AsString() const
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return EMPTY_STRING;
	}

	return m_pImpl->AsString();
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::ClearArguments()
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->ClearArguments();
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(int data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer(data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(unsigned int data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer((int)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(short data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer((int)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(unsigned short data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer((int)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(long data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer((int)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(unsigned long data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer((int)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(char data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer((int)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(signed char data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer((int)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(unsigned char data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Integer((int)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(__int64 data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Int64(data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(unsigned __int64 data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Int64((__int64) data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(float data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Double((double)data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(double data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Double(data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(long double data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_LongDouble(data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(const void *data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_Pointer(data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(const char *data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_SPointer(data);
}

// =====================================================================================================================
// =======================================================================================================================
void CStringFormatter::AddArgument(int *data)
{
	if (!m_pImpl) {
		s_RaiseError(InternalError, "Internal Error");	// we should never reach here
		return;
	}

	m_pImpl->AddArgument_IPointer(data);
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper::CFormatHelper(const char *pszFormat, const char *pszFileName, int iFileLine)
	: m_formatter(pszFormat, pszFileName, iFileLine)
{
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (int data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (unsigned int data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (short data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (unsigned short data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (long data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (unsigned long data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (char data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (signed char data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (unsigned char data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (__int64 data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (unsigned __int64 data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (float data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (double data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (long double data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (const void *data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (const char *data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper & CFormatHelper::operator << (int *data)
{
	m_formatter.AddArgument(data);
	return *this;
}

// =====================================================================================================================
// =======================================================================================================================
const std::string &CFormatHelper::str() const
{
	return m_formatter.AsString();
}

// =====================================================================================================================
// =======================================================================================================================
CFormatHelper::operator const std::string&() const
{
	return m_formatter.AsString();
}
} // end of namespace string_format
