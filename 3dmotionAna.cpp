#pragma warning(disable : 4786)
#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "StringFormat.h"

#define PRINT_DETAIL
#define CALC_REDUCE

const char *pszTransFile = "./BodyMotionTrans.ini";
const char *pszNewMotionFile = "New3Dmotion.ini";

const int _MAX_STRING = 1024;

// =====================================================================================================================
// =======================================================================================================================
bool GetIndexInfo(__int64 i64Index, int &rLook, int &rWeaponMotion)
{
	rLook = i64Index / 10000000;
	rWeaponMotion = i64Index % 10000000;
	return true;
}

// =====================================================================================================================
// =======================================================================================================================
__int64 ComboIndexInfo(int nLook, int nWeaponMotion)
{
	return(__int64) 10000000 * nLook + nWeaponMotion;
}

// =====================================================================================================================
// =======================================================================================================================
bool ReadIndexFile(const char *psz3dmotion, std::map<__int64, std::string> &mapOrgInfo)
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
bool ReadReducedFile(const char *pszTrans, std::map<int, int> &mapLookTrans, std::map<int, int> &mapWeaponMotionTrans)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FILE *pFile = fopen(pszTransFile, "r");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (NULL == pFile) {
		printf("open %s failed\n", pszTransFile);
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	char szLine[_MAX_STRING];
	std::map<int, int> mapTmp;
	std::map<int, int> *pMap = &mapTmp;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	while (fgets(szLine, sizeof(szLine), pFile)) {

		//~~~~~~~~~~~~~~~~~~~~
		char szTmp[_MAX_STRING];
		//~~~~~~~~~~~~~~~~~~~~

		if (szLine[0] == '*' && 1 == sscanf(szLine + 1, "%s", szTmp)) {
			if (strstr(szTmp, "look")) {
				pMap = &mapLookTrans;
			} else if (strstr(szTmp, "WeaponMotion")) {
				pMap = &mapWeaponMotionTrans;
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
__int64 GetMotionReducedIndex(__int64 i64Index,
							  const std::map<int, int> &mapLookTrans,
							  const std::map<int, int> &mapWeaponMotionTrans)
{
	//~~~~~~~~~~~~~~~~~~
	int nLook = 0;
	int nWeaponMotion = 0;
	//~~~~~~~~~~~~~~~~~~

	GetIndexInfo(i64Index, nLook, nWeaponMotion);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const std::map<int, int>::const_iterator itLook = mapLookTrans.find(nLook);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (itLook == mapLookTrans.end()) {
		return 0;
	}

	nLook = itLook->second;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const std::map<int, int>::const_iterator itWeapon = mapWeaponMotionTrans.find(nWeaponMotion);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (itWeapon == mapWeaponMotionTrans.end()) {
		return 0;
	}

	nWeaponMotion = itWeapon->second;
	i64Index = ComboIndexInfo(nLook, nWeaponMotion);
	return i64Index;
}

// =====================================================================================================================
// =======================================================================================================================
__int64 GetMotionIndexByRuduced(__int64 i64Index,
								const std::map<__int64, std::string> &mapNewIndex,
								const std::map<int, int> &mapLookTrans,
								const std::map<int, int> &mapWeaponMotionTrans)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<__int64, std::string>::const_iterator it = mapNewIndex.find(i64Index);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (it != mapNewIndex.end()) {
		return i64Index;
	}

	i64Index = GetMotionReducedIndex(i64Index, mapLookTrans, mapWeaponMotionTrans);
	it = mapNewIndex.find(i64Index);
	if (it != mapNewIndex.end()) {
		return i64Index;
	}

	return -1;
}

// =====================================================================================================================
// =======================================================================================================================
std::string GetMotionResByRuduced(__int64 i64Index,
								  const std::map<__int64, std::string> &mapNewIndex,
								  const std::map<int, int> &mapLookTrans,
								  const std::map<int, int> &mapWeaponMotionTrans)
{
	i64Index = GetMotionIndexByRuduced(i64Index, mapNewIndex, mapLookTrans, mapWeaponMotionTrans);
	if (i64Index == -1) {
		return "no index";
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
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::string strRet = strIndex;
	const char *pPosMonster = strstr(strRet.c_str(), "monster");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (pPosMonster) {
		ReplaceString(strRet, "monster", "");
		ReplaceString(strRet, "N/", " ");
	}

	ReplaceString(strRet, "One-Handed/Axe", "450");
	ReplaceString(strRet, "One-Handed/Sword", "420");
	ReplaceString(strRet, "Shield-With/Sword", "742");
	ReplaceString(strRet, "Shield-With/Axe", "745");
	ReplaceString(strRet, "Shield-With/Dagger", "749");
	ReplaceString(strRet, "Two-Handed/Axe", "651");
	ReplaceString(strRet, "Two-Handed/Dagger", "691");
	ReplaceString(strRet, "Two-Handed/Sword", "621");

	ReplaceString(strRet, ".c3", "");
	ReplaceString(strRet, ".C3", "");
	ReplaceString(strRet, "/", " ");
	ReplaceString(strRet, "c3", "");
	ReplaceString(strRet, "C3", "");

	//~~~~~~~~~~~~
	int nLook = 0;
	int nWeapon = 0;
	int nMotion = 0;
	//~~~~~~~~~~~~

	if (pPosMonster) {
		if (2 == sscanf(strRet.c_str(), "%d%d", &nLook, &nMotion)) {
			return nLook * (__int64) 10000000 + nWeapon * 10000 + nMotion;
		}
	} else if (3 == sscanf(strRet.c_str(), "%d%d%d", &nLook, &nWeapon, &nMotion)) {
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
					std::map<int, int> &mapWeaponMotionTrans)
{
	mapLookTrans.clear();
	mapWeaponMotionTrans.clear();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::vector<FORMAT_RES_DATA>::const_iterator it = vecResData.begin();
	std::map<int, std::map<int, int> > mapLookTransCount;
	std::map<int, std::map<int, int> > mapWeaponMotionTransCount;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (; it != vecResData.end(); ++it) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		const FORMAT_RES_DATA &rData = *it;
		const std::vector<__int64> &rVecIndex = rData.vecIndex;
		__int64 i64IndexRes = ResPathTransIndex(rData.strRes);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (i64IndexRes > 0) {

			//~~~~~~~~~~~~~~~~~~~~~
			int nResLook = 0;
			int nResWeaponMotion = 0;
			//~~~~~~~~~~~~~~~~~~~~~

			GetIndexInfo(i64IndexRes, nResLook, nResWeaponMotion);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			std::vector<__int64>::const_iterator itIndex = rVecIndex.begin();
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			for (; itIndex != rVecIndex.end(); ++itIndex) {

				//~~~~~~~~~~~~~~~~~~~~~~~~
				int nIndexLook = 0;
				int nIndexWeaponMotion = 0;
				__int64 i64Index = *itIndex;
				//~~~~~~~~~~~~~~~~~~~~~~~~

				GetIndexInfo(i64Index, nIndexLook, nIndexWeaponMotion);
				++mapLookTransCount[nIndexLook][nResLook];
				++mapWeaponMotionTransCount[nIndexWeaponMotion][nResWeaponMotion];
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

	for (itMap = mapWeaponMotionTransCount.begin(); itMap != mapWeaponMotionTransCount.end(); ++itMap) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		std::map<int, int>::const_iterator it;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		for (it = itMap->second.begin(); it != itMap->second.end(); ++it) {
			mapWeaponMotionTrans[itMap->first] = CalMapMaxSecondFirst(itMap->second);
		}
	}

	return true;
}

// =====================================================================================================================
// =======================================================================================================================
bool SimpleReduce(const std::map<__int64, std::string> &rMapOrgInfo,
				  const std::vector<FORMAT_RES_DATA> &vecResData,
				  std::map<__int64, std::string> &mapNewIndex,
				  std::map<int, int> &mapLookTrans,
				  std::map<int, int> &mapWeaponMotionTrans)
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
		bool bIndexResOk = true;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (i64IndexRes <= 0) {
			bIndexResOk = false;
		}

		if (bIndexResOk) {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			std::map<__int64, std::string>::const_iterator it = rMapOrgInfo.find(i64IndexRes);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			if (it == rMapOrgInfo.end() || it->second != rData.strRes) {
				bIndexResOk = false;
			}
		}

		if (!bIndexResOk) {

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

			//~~~~~~~~~~~~~~~~~~~~~
			int nResLook = 0;
			int nResWeaponMotion = 0;
			//~~~~~~~~~~~~~~~~~~~~~

			GetIndexInfo(i64IndexRes, nResLook, nResWeaponMotion);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			std::vector<__int64>::const_iterator itIndex = rVecIndex.begin();
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			for (; itIndex != rVecIndex.end(); ++itIndex) {

				//~~~~~~~~~~~~~~~~~~~~~~~~
				int nIndexLook = 0;
				int nIndexWeaponMotion = 0;
				__int64 i64Index = *itIndex;
				//~~~~~~~~~~~~~~~~~~~~~~~~

				GetIndexInfo(i64Index, nIndexLook, nIndexWeaponMotion);

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

				itFind = mapWeaponMotionTrans.find(nIndexWeaponMotion);
				if (itFind == mapWeaponMotionTrans.end()) {
					mapWeaponMotionTrans[nIndexWeaponMotion] = nResWeaponMotion;
				}

				if (mapWeaponMotionTrans[nIndexWeaponMotion] != nResWeaponMotion) {
					mapNewIndex[i64Index] = rData.strRes;
				}
			}
		}
	}

	printf("SimpleReduce End %d %d %d\n", mapNewIndex.size(), mapLookTrans.size(), mapWeaponMotionTrans.size());
	return true;
}

// =====================================================================================================================
// =======================================================================================================================
bool OutputSimpleReduceResult(const std::map<__int64, std::string> &mapNewIndex,
							  const std::map<int, int> &mapLookTrans,
							  const std::map<int, int> &mapWeaponMotionTrans)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FILE *pFile = fopen(pszTransFile, "w");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (NULL == pFile) {
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const int TRANS_TYPE = 2;
	std::string strTitleArr[TRANS_TYPE] = { "Look", "WeaponMotion" };
	const std::map<int, int> mapTransArr[TRANS_TYPE] = { mapLookTrans, mapWeaponMotionTrans };
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (int i = 0; i < TRANS_TYPE; ++i) {
		fprintf(pFile, "*%s\n", strTitleArr[i].c_str());

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
bool Check(const std::map<__int64, std::string> &mapOrgInfo,
		   const std::map<__int64, std::string> &mapNewIndex,
		   std::map<int, int> mapLookTrans,
		   std::map<int, int> mapWeaponMotionTrans)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int nCount = 0;
	std::map<__int64, std::string>::const_iterator it = mapOrgInfo.begin();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	for (; it != mapOrgInfo.end(); ++it) {

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		__int64 i64Index = it->first;
		std::string strRes = it->second;
		std::string strResCmp = GetMotionResByRuduced(i64Index, mapNewIndex, mapLookTrans, mapWeaponMotionTrans);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if (strRes != strResCmp) {

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			__int64 i64IndexInfo = GetMotionIndexByRuduced(i64Index, mapNewIndex, mapLookTrans, mapWeaponMotionTrans);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			printf("check error Index: %I64d, Res1 %s IndexReduce %I64d Res2 %s", i64Index, strRes.c_str(),
				   i64IndexInfo, strResCmp.c_str());
			return false;
		}

		++nCount;
		if (nCount % 50000 == 0) {
			printf("checked %d/%d\n", nCount, mapOrgInfo.size());
		}
	}

	printf("check suc\n");
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
	ReadIndexFile(szTmp, mapOrgInfo);
	mapIndex = mapOrgInfo;

#ifdef CALC_REDUCE
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		std::vector<FORMAT_RES_DATA> vecResData;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		AnaResData(mapIndex, vecResData);
		printf("IndexSize %d ResSize %d\n", mapIndex.size(), vecResData.size());

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		std::map<int, int> mapLookTrans;
		std::map<int, int> mapWeaponMotionTrans;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		CalcTransTable(vecResData, mapLookTrans, mapWeaponMotionTrans);
		SimpleReduce(mapOrgInfo, vecResData, mapIndex, mapLookTrans, mapWeaponMotionTrans);
		OutputSimpleReduceResult(mapIndex, mapLookTrans, mapWeaponMotionTrans);
	}
#endif

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<int, int> mapLookTrans;
	std::map<int, int> mapWeaponMotionTrans;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	ReadReducedFile(pszTransFile, mapLookTrans, mapWeaponMotionTrans);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::map<__int64, std::string> mapNewIndex;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	ReadIndexFile(pszNewMotionFile, mapNewIndex);

	Check(mapOrgInfo, mapNewIndex, mapLookTrans, mapWeaponMotionTrans);
	return 0;
}
