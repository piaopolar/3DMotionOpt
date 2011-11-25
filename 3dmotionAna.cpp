#pragma warning(disable : 4786)
#include <windows.h>
#include "Mmsystem.h"
#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "StringFormat.h"

const int _MAX_STRING = 1024;

// =====================================================================================================================
// =======================================================================================================================
DWORD TimeGet(void)
{
	return timeGetTime();
}

// =====================================================================================================================
// =======================================================================================================================
bool ReadOrgFile(const char *psz3dmotion,
				 std::map<__int64, std::string> &mapOrgInfo,
				 std::map<std::string, std::vector<__int64> > &mapRes)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FILE *pFile = fopen(psz3dmotion, "r");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (NULL == pFile) {
		printf("open %s failed\n", psz3dmotion);
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~
	char szTmp[_MAX_STRING];
	//~~~~~~~~~~~~~~~~~~~~

	while (fgets(szTmp, sizeof(szTmp), pFile)) {
		if (szTmp[0] == 0 || szTmp[0] == ';') {
			continue;
		}

		//~~~~~~~~~~~~~~~~~~~~
		__int64 i64Index = 0;
		char szRes[_MAX_STRING];
		//~~~~~~~~~~~~~~~~~~~~

		if (2 == sscanf(szTmp, "%I64d=%s", &i64Index, szRes)) {
			mapOrgInfo[i64Index] = szRes;
			mapRes[szRes].push_back(i64Index);
		} else {
			printf("error line [%s]\n", szTmp);
		}
	}

	fclose(pFile);

	printf("Index size %d   Res size %d\n", mapOrgInfo.size(), mapRes.size());
	return true;
}

struct FORMAT_RES_DATA
{
	int nIndexSize;
	std::string strRes;
	std::vector<__int64> vecIndex;
	bool operator < (const FORMAT_RES_DATA &rData) {
		return nIndexSize > rData.nIndexSize;
	}
};

// =====================================================================================================================
// =======================================================================================================================

bool AnaResData(std::map<std::string, std::vector<__int64> > &mapRes, std::vector<FORMAT_RES_DATA> &vecResData)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<std::string, std::vector<__int64> >::const_iterator it = mapRes.begin();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (; it != mapRes.end(); ++it) {

		//~~~~~~~~~~~~~~~~~
		FORMAT_RES_DATA data;
		//~~~~~~~~~~~~~~~~~

		data.strRes = it->first;
		data.vecIndex = it->second;
		data.nIndexSize = data.vecIndex.size();
		std::sort(data.vecIndex.begin(), data.vecIndex.end());
		vecResData.push_back(data);
	}

	std::sort(vecResData.begin(), vecResData.end());

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FILE *pFile = fopen("ResAna.out", "w");
	std::vector<FORMAT_RES_DATA>::const_iterator itRes = vecResData.begin();
	int nCountRes = 0;
	int nCountIndex = 0;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (; itRes != vecResData.end(); ++itRes) {
		++nCountRes;

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		const FORMAT_RES_DATA &data = *itRes;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		nCountIndex += data.nIndexSize;
		fprintf(pFile, "[%s]	%d	%d	%d\n", data.strRes.c_str(), data.nIndexSize, nCountRes, nCountIndex);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define PRINT_DETAIL
#ifdef PRINT_DETAIL
		std::vector<__int64>::const_iterator it = data.vecIndex.begin();
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		for (; it != data.vecIndex.end(); ++it) {
			fprintf(pFile, "	%I64d	%03I64d,%03I64d,%03I64d,%04I64d\n", *it, *it / 10000000000, *it / 10000000 % 1000,
					*it / 10000 % 1000, *it % 10000);
		}

		fprintf(pFile, "\n\n");
#endif
	}

	fclose(pFile);

	return true;
}

// =====================================================================================================================
// =======================================================================================================================
int main()
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	char szTmp[_MAX_STRING];
	std::map<__int64, std::string> mapOrgInfo;
	std::map<std::string, std::vector<__int64> > mapRes;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	gets(szTmp);
	ReadOrgFile(szTmp, mapOrgInfo, mapRes);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::vector<FORMAT_RES_DATA> vecResData;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	AnaResData(mapRes, vecResData);

	while (gets(szTmp)) {

		//~~~~~~~~~~~~~~~~~~~~~~
		DWORD dwTime = TimeGet();
		std::string strInfo = "";
		DWORD dwTime2 = dwTime;
		const int COUNTS = 100000;
		int i = 0;
		//~~~~~~~~~~~~~~~~~~~~~~

		for (i = 0; i < COUNTS; ++i) {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			std::map<__int64, std::string>::iterator it = mapOrgInfo.find(45000100);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		} dwTime2 = TimeGet();
		strInfo += (FORMAT("%d ") << dwTime2 - dwTime);
		dwTime = dwTime2;

		for (i = 0; i < COUNTS; ++i) {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			std::map<__int64, std::string>::iterator it = mapOrgInfo.find(i);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		} dwTime2 = TimeGet();
		strInfo += (FORMAT("%d ") << dwTime2 - dwTime);
		dwTime = dwTime2;

		printf("Time Cost %s", strInfo.c_str());
	}

	return 0;
}
