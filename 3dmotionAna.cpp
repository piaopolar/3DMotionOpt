#pragma warning(disable : 4786)
#include <windows.h>
#include "Mmsystem.h"
#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "StringFormat.h"

const char *pszTransFile = "Trans.ini";
const char *pszSectionTime = "Time";
const char *pszKeyAmount = "Amount";
const char *pszNewMotionFile = "New3DMotion.ini";
const int TIME_LIMIT = 5;

const int _MAX_STRING = 1024;

// =====================================================================================================================
// =======================================================================================================================
DWORD TimeGet(void)
{
	return timeGetTime();
}

// =====================================================================================================================
// =======================================================================================================================
bool GetIndexInfo(__int64 i64Index, int &rLook, int &rWeapon, int &rMotion)
{
	rLook = i64Index / 10000000;
	rWeapon = i64Index / 10000 % 1000;
	rMotion = i64Index % 10000;
	return true;
}

// =====================================================================================================================
// =======================================================================================================================
__int64 ComboIndexInfo(int nLook, int nWeapon, int nMotion)
{
	return(__int64) 10000000 * nLook + nWeapon * 10000 + nMotion;
}

// =====================================================================================================================
// =======================================================================================================================
bool ReadOrgFile(const char *psz3dmotion, std::map<__int64, std::string> &mapOrgInfo)
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
		} else {
			printf("error line [%s]\n", szTmp);
		}
	}

	fclose(pFile);

	printf("Org File Index size %d\n", mapOrgInfo.size());
	return true;
}

// =====================================================================================================================
// =======================================================================================================================
bool ReadReducedFile(const char *pszTrans,
					 const char *pszNewMotion,
					 std::map<__int64, std::string> &mapNewIndex,
					 std::map<int, int> mapLookTrans[TIME_LIMIT],
					 std::map<int, int> mapWeaponTrans[TIME_LIMIT],
					 std::map<int, int> mapMotionTrans[TIME_LIMIT])
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FILE *pFile = fopen(pszNewMotion, "r");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (NULL == pFile) {
		printf("open %s failed\n", pszNewMotion);
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~
	char szLine[_MAX_STRING];
	//~~~~~~~~~~~~~~~~~~~~~

	while (fgets(szLine, sizeof(szLine), pFile)) {
		if (szLine[0] == 0 || szLine[0] == ';') {
			continue;
		}

		//~~~~~~~~~~~~~~~~~~~~
		__int64 i64Index = 0;
		char szRes[_MAX_STRING];
		//~~~~~~~~~~~~~~~~~~~~

		if (2 == sscanf(szLine, "%I64d=%s", &i64Index, szRes)) {
			mapNewIndex[i64Index] = szRes;
		} else {
			printf("error line [%s]\n", szLine);
		}
	}

	fclose(pFile);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int nAmount = GetPrivateProfileInt(pszSectionTime, pszKeyAmount, TIME_LIMIT, pszTransFile);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	pFile = fopen(pszTransFile, "r");
	if (NULL == pFile) {
		printf("open %s failed\n", pszTransFile);
		return false;
	}

	while (fgets(szLine, sizeof(szLine), pFile)) {
		if (!strstr(szLine, "[Data]")) {
			continue;
		}
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<int, int> mapTmp;
	std::map<int, int> *pMap = &mapTmp;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	while (fgets(szLine, sizeof(szLine), pFile)) {

		//~~~~~~~~~~~~~~~~~~~~
		char szTmp[_MAX_STRING];
		int nTime;
		//~~~~~~~~~~~~~~~~~~~~

		// std::string strTitleArr[TRANS_TYPE] = { "look", "Weapon", "Motion" };
		if (szLine[0] == '[' && 2 == sscanf(szLine + 1, "%s %d", szTmp, &nTime)) {
			if (nTime >= TIME_LIMIT) {
				printf("error time %d, line [%s]", nTime, szLine);
				return false;
			}

			if (strstr(szTmp, "look")) {
				pMap = &mapLookTrans[nTime];
			} else if (strstr(szTmp, "Weapon")) {
				pMap = &mapWeaponTrans[nTime];
			} else if (strstr(szTmp, "Motion")) {
				pMap = &mapMotionTrans[nTime];
			}

			continue;
		}

		//~~~~~~~~~~~
		int nTransScr;
		int nTransDest;
		//~~~~~~~~~~~

		if (2 == sscanf(szLine, "%d %d", &nTransScr, &nTransDest)) {
			(*pMap)[nTransScr] = nTransDest;
		}
	}

	fclose(pFile);
	return true;
}

// =====================================================================================================================
// =======================================================================================================================
__int64 GetMotionIndexByRuduced(__int64 i64Index,
								const std::map<__int64, std::string> &mapNewIndex,
								int nTimeMax,
								std::map<int, int> mapLookTrans[TIME_LIMIT],
								std::map<int, int> mapWeaponTrans[TIME_LIMIT],
								std::map<int, int> mapMotionTrans[TIME_LIMIT])
{
	for (int iTime = 0;; ++iTime) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		const std::map<__int64, std::string>::const_iterator it = mapNewIndex.find(i64Index);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (it != mapNewIndex.end()) {
			return it->first;
		}

		if (iTime >= nTimeMax) {
			return -1;
		}

		//~~~~~~~~~~~~
		int nLook = 0;
		int nWeapon = 0;
		int nMotion = 0;
		//~~~~~~~~~~~~

		GetIndexInfo(i64Index, nLook, nWeapon, nMotion);
		nLook = mapLookTrans[iTime][nLook];
		nWeapon = mapWeaponTrans[iTime][nWeapon];
		nMotion = mapMotionTrans[iTime][nMotion];
		i64Index = ComboIndexInfo(nLook, nWeapon, nMotion);
	}

	return -2;
}

// =====================================================================================================================
// =======================================================================================================================
std::string GetMotionResByRuduced(__int64 i64Index,
								  const std::map<__int64, std::string> &mapNewIndex,
								  int nTimeMax,
								  std::map<int, int> mapLookTrans[TIME_LIMIT],
								  std::map<int, int> mapWeaponTrans[TIME_LIMIT],
								  std::map<int, int> mapMotionTrans[TIME_LIMIT])
{
	i64Index = GetMotionIndexByRuduced(i64Index, mapNewIndex, nTimeMax, mapLookTrans, mapWeaponTrans, mapMotionTrans);
	if (i64Index == -1) {
		return "no index";
	} else if (i64Index == -2) {
		return "error";
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const std::map<__int64, std::string>::const_iterator it = mapNewIndex.find(i64Index);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (it != mapNewIndex.end()) {
		return it->second;
	}

	return "no res";
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

bool AnaResData(const std::map<__int64, std::string> &mapIndex, std::vector<FORMAT_RES_DATA> &vecResData)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<std::string, std::vector<__int64> > mapRes;
	std::map<__int64, std::string>::const_iterator itIndex;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (itIndex = mapIndex.begin(); itIndex != mapIndex.end(); ++itIndex) {
		mapRes[itIndex->second].push_back(itIndex->first);
	}

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
			fprintf(pFile, "	%I64d	%03I64d,%03I64d,%03I64d,%04I64d	%s\n", *it, *it / 10000000000, *it / 10000000 % 1000,
					*it / 10000 % 1000, *it % 10000, data.strRes.c_str());
		}

		fprintf(pFile, "\n\n");
#endif
	}

	fclose(pFile);

	return true;
}

// =====================================================================================================================
// =======================================================================================================================
bool ReplaceString(std::string &str, const char *pszFind, const char *pszReplace)
{
	if (!pszFind || strlen(pszFind) <= 0) {
		return false;
	}

	if (!pszReplace) {
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int nLenFind = strlen(pszFind);
	int nLenReplace = strlen(pszReplace);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	std::string::size_type sizeIndex = str.find(pszFind);
	while (sizeIndex != std::string::npos) {
		str.replace(sizeIndex, nLenFind, pszReplace);
		sizeIndex = str.find(pszFind, sizeIndex + nLenReplace);
	}

	return true;
}

// =====================================================================================================================
// =======================================================================================================================
__int64 ResPathTransIndex(const std::string &strIndex)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::string strRet = strIndex;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~

	ReplaceString(strRet, ".c3", "");
	ReplaceString(strRet, ".C3", "");
	ReplaceString(strRet, "/", " ");
	ReplaceString(strRet, "c3", "");
	ReplaceString(strRet, "C3", "");

	//~~~~~~~~
	int nLook;
	int nWeapon;
	int nMotion;
	//~~~~~~~~

	if (3 == sscanf(strRet.c_str(), "%d%d%d", &nLook, &nWeapon, &nMotion)) {
		return nLook * (__int64) 10000000 + nWeapon * 10000 + nMotion;
	}

	return 0;
}

// =====================================================================================================================
// =======================================================================================================================
int CalMapMaxSecondFirst(const std::map<int, int> &mapCount)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int nMax = 0;
	int nFirst = 0;
	std::map<int, int>::const_iterator it = mapCount.begin();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (; it != mapCount.end(); ++it) {
		if (it->second > nMax) {
			nMax = it->second;
			nFirst = it->first;
		}
	}

	return nFirst;
}

// =====================================================================================================================
// =======================================================================================================================
bool CalcTransTable(const std::vector<FORMAT_RES_DATA> &vecResData,
					std::map<int, int> &mapLookTrans,
					std::map<int, int> &mapWeaponTrans,
					std::map<int, int> &mapMotionTrans)
{
	mapLookTrans.clear();
	mapWeaponTrans.clear();
	mapMotionTrans.clear();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::vector<FORMAT_RES_DATA>::const_iterator it = vecResData.begin();
	std::map<int, std::map<int, int> > mapLookTransCount;
	std::map<int, std::map<int, int> > mapWeaponTransCount;
	std::map<int, std::map<int, int> > mapMotionTransCount;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (; it != vecResData.end(); ++it) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		const FORMAT_RES_DATA &rData = *it;
		const std::vector<__int64> &rVecIndex = rData.vecIndex;
		__int64 i64IndexRes = ResPathTransIndex(rData.strRes);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (i64IndexRes > 0) {

			//~~~~~~~~~~~~~~~
			int nResLook = 0;
			int nResWeapon = 0;
			int nResMotion = 0;
			//~~~~~~~~~~~~~~~

			GetIndexInfo(i64IndexRes, nResLook, nResWeapon, nResMotion);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			std::vector<__int64>::const_iterator itIndex = rVecIndex.begin();
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			for (; itIndex != rVecIndex.end(); ++itIndex) {

				//~~~~~~~~~~~~~~~~~~~~~~~~
				int nIndexLook = 0;
				int nIndexWeapon = 0;
				int nIndexMotion = 0;
				__int64 i64Index = *itIndex;
				//~~~~~~~~~~~~~~~~~~~~~~~~

				GetIndexInfo(i64Index, nIndexLook, nIndexWeapon, nIndexMotion);
				++mapLookTransCount[nIndexLook][nResLook];
				++mapWeaponTransCount[nIndexWeapon][nResWeapon];
				++mapMotionTransCount[nIndexMotion][nResMotion];
			}
		}
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<int, std::map<int, int> >::const_iterator itMap;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (itMap = mapLookTransCount.begin(); itMap != mapLookTransCount.end(); ++itMap) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		std::map<int, int>::const_iterator it;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		for (it = itMap->second.begin(); it != itMap->second.end(); ++it) {
			mapLookTrans[itMap->first] = CalMapMaxSecondFirst(itMap->second);
		}
	}

	for (itMap = mapWeaponTransCount.begin(); itMap != mapWeaponTransCount.end(); ++itMap) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		std::map<int, int>::const_iterator it;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		for (it = itMap->second.begin(); it != itMap->second.end(); ++it) {
			mapWeaponTrans[itMap->first] = CalMapMaxSecondFirst(itMap->second);
		}
	}

	for (itMap = mapMotionTransCount.begin(); itMap != mapMotionTransCount.end(); ++itMap) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		std::map<int, int>::const_iterator it;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		for (it = itMap->second.begin(); it != itMap->second.end(); ++it) {
			mapMotionTrans[itMap->first] = CalMapMaxSecondFirst(itMap->second);
		}
	}

	return true;
}

// =====================================================================================================================
// =======================================================================================================================
bool SimpleReduce(const std::vector<FORMAT_RES_DATA> &vecResData,
				  std::map<__int64, std::string> &mapNewIndex,
				  std::map<int, int> &mapLookTrans,
				  std::map<int, int> &mapWeaponTrans,
				  std::map<int, int> &mapMotionTrans)
{
	mapNewIndex.clear();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::vector<FORMAT_RES_DATA>::const_iterator it = vecResData.begin();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (; it != vecResData.end(); ++it) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		const FORMAT_RES_DATA &rData = *it;
		const std::vector<__int64> &rVecIndex = rData.vecIndex;
		__int64 i64IndexRes = ResPathTransIndex(rData.strRes);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (i64IndexRes <= 0) {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			std::vector<__int64>::const_iterator itIndex = rVecIndex.begin();
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			for (; itIndex != rVecIndex.end(); ++itIndex) {

				//~~~~~~~~~~~~~~~~~~~~~~~~
				__int64 i64Index = *itIndex;
				//~~~~~~~~~~~~~~~~~~~~~~~~

				mapNewIndex[i64Index] = rData.strRes;
			}
		} else {
			mapNewIndex[i64IndexRes] = rData.strRes;

			//~~~~~~~~~~~~~~~
			int nResLook = 0;
			int nResWeapon = 0;
			int nResMotion = 0;
			//~~~~~~~~~~~~~~~

			GetIndexInfo(i64IndexRes, nResLook, nResWeapon, nResMotion);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			std::vector<__int64>::const_iterator itIndex = rVecIndex.begin();
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			for (; itIndex != rVecIndex.end(); ++itIndex) {

				//~~~~~~~~~~~~~~~~~~~~~~~~
				int nIndexLook = 0;
				int nIndexWeapon = 0;
				int nIndexMotion = 0;
				__int64 i64Index = *itIndex;
				//~~~~~~~~~~~~~~~~~~~~~~~~

				GetIndexInfo(i64Index, nIndexLook, nIndexWeapon, nIndexMotion);

				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				std::map<int, int>::const_iterator itFind;
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

				itFind = mapLookTrans.find(nIndexLook);
				if (itFind == mapLookTrans.end()) {
					mapLookTrans[nIndexLook] = nResLook;
				}

				if (mapLookTrans[nIndexLook] != nResLook) {
					mapNewIndex[i64Index] = rData.strRes;
					continue;
				}

				itFind = mapWeaponTrans.find(nIndexWeapon);
				if (itFind == mapWeaponTrans.end()) {
					mapWeaponTrans[nIndexWeapon] = nResWeapon;
				}

				if (mapWeaponTrans[nIndexWeapon] != nResWeapon) {
					mapNewIndex[i64Index] = rData.strRes;
					continue;
				}

				itFind = mapMotionTrans.find(nIndexMotion);
				if (itFind == mapMotionTrans.end()) {
					mapMotionTrans[nIndexMotion] = nResMotion;
				}

				if (mapMotionTrans[nIndexMotion] != nResMotion) {
					mapNewIndex[i64Index] = rData.strRes;
				}
			}
		}
	}

	printf("SimpleReduce End %d %d %d %d\n", mapNewIndex.size(), mapLookTrans.size(), mapWeaponTrans.size(),
		   mapMotionTrans.size());
	return true;
}

// =====================================================================================================================
// =======================================================================================================================
bool OutputSimpleReduceResult(const std::map<__int64, std::string> &mapNewIndex,
							  const std::map<int, int> &mapLookTrans,
							  const std::map<int, int> &mapWeaponTrans,
							  const std::map<int, int> &mapMotionTrans,
							  int iTime)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FILE *pFile = fopen(pszTransFile, iTime <= 1 ? "w" : "w+");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (NULL == pFile) {
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~
	char szTime[_MAX_STRING];
	//~~~~~~~~~~~~~~~~~~~~~

	_snprintf(szTime, sizeof(szTime), "%d", iTime);
	WritePrivateProfileString(pszSectionTime, pszKeyAmount, szTime, pszTransFile);

	if (iTime == 1) {
		fprintf(pFile, "[Data]\n");
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const int TRANS_TYPE = 3;
	std::string strTitleArr[TRANS_TYPE] = { "look", "Weapon", "Motion" };
	const std::map<int, int> mapTransArr[TRANS_TYPE] = { mapLookTrans, mapWeaponTrans, mapMotionTrans };
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (int i = 0; i < TRANS_TYPE; ++i) {
		fprintf(pFile, "[%s %d]\n", strTitleArr[i].c_str(), iTime);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		const std::map<int, int> &mapTrans = mapTransArr[i];
		std::map<int, int>::const_iterator it = mapTrans.begin();
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		for (; it != mapTrans.end(); ++it) {
			fprintf(pFile, "%d %d\n", it->first, it->second);
		}
	}

	fclose(pFile);
	pFile = fopen(pszNewMotionFile, "w");
	if (NULL == pFile) {
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<__int64, std::string>::const_iterator it = mapNewIndex.begin();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (; it != mapNewIndex.end(); ++it) {
		fprintf(pFile, "%I64d=%s\n", it->first, it->second.c_str());
	}

	fclose(pFile);
	return true;
}

// =====================================================================================================================
// =======================================================================================================================
int main()
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	char szTmp[_MAX_STRING];
	std::map<__int64, std::string> mapOrgInfo;
	std::map<__int64, std::string> mapIndex;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	gets(szTmp);
	ReadOrgFile(szTmp, mapOrgInfo);
	mapIndex = mapOrgInfo;

	for (int iTime = 1; iTime <= TIME_LIMIT; ++iTime) {
		printf("Time %d: ", iTime);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~
		const int LIMIT_SIZE = 100000;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (mapIndex.size() <= LIMIT_SIZE) {
			printf("IndexSize %d <= %d, done\n", iTime, mapIndex.size(), LIMIT_SIZE);
			break;
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		std::vector<FORMAT_RES_DATA> vecResData;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		AnaResData(mapIndex, vecResData);
		printf("IndexSize %d ResSize %d\n", mapIndex.size(), vecResData.size());

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		std::map<int, int> mapLookTrans;
		std::map<int, int> mapWeaponTrans;
		std::map<int, int> mapMotionTrans;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		CalcTransTable(vecResData, mapLookTrans, mapWeaponTrans, mapMotionTrans);
		SimpleReduce(vecResData, mapIndex, mapLookTrans, mapWeaponTrans, mapMotionTrans);
		OutputSimpleReduceResult(mapIndex, mapLookTrans, mapWeaponTrans, mapMotionTrans, iTime);
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<__int64, std::string> mapNewIndex;
	std::map<int, int> mapLookTrans[TIME_LIMIT];
	std::map<int, int> mapWeaponTrans[TIME_LIMIT];
	std::map<int, int> mapMotionTrans[TIME_LIMIT];
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	ReadReducedFile(pszNewMotionFile, pszNewMotionFile, mapNewIndex, mapLookTrans, mapWeaponTrans, mapMotionTrans);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int nAmount = GetPrivateProfileInt(pszSectionTime, pszKeyAmount, TIME_LIMIT, pszTransFile);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

		for (i = 0; i < COUNTS; ++i) {
			GetMotionIndexByRuduced(45000100, mapNewIndex, nAmount, mapLookTrans, mapWeaponTrans, mapMotionTrans);
		}

		dwTime2 = TimeGet();
		strInfo += (FORMAT("%d ") << dwTime2 - dwTime);
		dwTime = dwTime2;

		for (i = 0; i < COUNTS; ++i) {
			GetMotionIndexByRuduced(i, mapNewIndex, nAmount, mapLookTrans, mapWeaponTrans, mapMotionTrans);
		}

		dwTime2 = TimeGet();
		strInfo += (FORMAT("%d ") << dwTime2 - dwTime);
		dwTime = dwTime2;

		printf("Time Cost %s", strInfo.c_str());
	}

	return 0;
}
