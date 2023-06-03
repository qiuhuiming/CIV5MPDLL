#include "CvGameCoreDLLPCH.h"
#include "CvLuaFormula.h"

bool CvLuaFormula::CacheResults(Database::Results& kResults, CvDatabaseUtility& kUtility)
{
	if (!CvBaseInfo::CacheResults(kResults, kUtility))
	{
		return false;
	}

	const char* text = kResults.GetText("Formula");
	if (text == nullptr)
	{
		m_strFormula = "";
	}
	else
	{
		m_strFormula = text;
	}

	return true;
}

std::string CvLuaFormula::GetFormula() const
{
	return m_strFormula;
}

std::vector<CvLuaFormula*>& CvLuaFormulaXMLEntries::GetEntries()
{
	return m_vEntries;
}
CvLuaFormula* CvLuaFormulaXMLEntries::GetEntry(LuaFormulaTypes eFormula)
{
	if (eFormula < 0 || eFormula >= m_vEntries.size())
	{
		return nullptr;
	}

	return m_vEntries[eFormula];
}
int CvLuaFormulaXMLEntries::GetNumEntries()
{
	return m_vEntries.size();
}