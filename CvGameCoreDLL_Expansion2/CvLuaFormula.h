#pragma once
#include "CvEnums.h"

class CvLuaFormula : public CvBaseInfo
{
public:
    bool CacheResults(Database::Results& kResults, CvDatabaseUtility& kUtility) override;
    std::string GetFormula() const;

private:
    std::string m_strFormula;
};

class CvLuaFormulaXMLEntries {
public:
    std::vector<CvLuaFormula*>& GetEntries();
    CvLuaFormula* GetEntry(LuaFormulaTypes eFormula);
    int GetNumEntries();
private:
    std::vector<CvLuaFormula*> m_vEntries;
};
