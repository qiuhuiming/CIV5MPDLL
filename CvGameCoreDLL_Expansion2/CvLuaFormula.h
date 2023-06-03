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

class CvGlobals;

namespace lua {
    template <typename T>
    struct Result {
        bool ok = true;
        T result;

        static Result Error()
        {
            Result result;
            result.ok = false;
            return result;
        }

        static Result Ok(T value)
        {
            Result result;
            result.ok = true;
            result.result = value;
            return result;
        }
    };

    class EvaluatorFactory;

    template <typename FirstArg, typename... OtherArgs>
    inline void Push(lua_State* l, FirstArg first, OtherArgs... args)
    {
        Push(l, first);
        Push(l, args...);
    }

    inline void Push(lua_State* l, int value)
    {
        lua_pushinteger(l, value);
    }

    inline void Push(lua_State* l, double value)
    {
        lua_pushnumber(l, value);
    }

    inline void Push(lua_State* l, bool value)
    {
        lua_pushboolean(l, value);
    }

    inline bool Pop(lua_State* l, int* out_value)
    {
        if (!lua_isnumber(l, -1))
        {
            lua_pop(l, 1);
            return false;
        }

        if (out_value)
        {
            *out_value = lua_tointeger(l, -1);
        }
        lua_pop(l, 1);
        return true;
    }

    inline bool Pop(lua_State* l, double* out_value)
    {
        if (!lua_isnumber(l, -1))
        {
            lua_pop(l, 1);
            return false;
        }

        if (out_value)
        {
            *out_value = lua_tonumber(l, -1);
        }
        lua_pop(l, 1);
        return true;
    }

    inline bool Pop(lua_State* l, bool* out_value)
    {
        if (!lua_isboolean(l, -1))
        {
            lua_pop(l, 1);
            return false;
        }

        if (out_value)
        {
            *out_value = lua_toboolean(l, -1);
        }
        lua_pop(l, 1);
        return true;
    }

    class Evaluator {
    private:
        friend class EvaluatorFactory;

        void LogMsg(const char* format, ...)
        {
            const size_t kBuffSize = 1024;
            static char buf[kBuffSize];
            const uint uiFlags = 0;    // Default (0) is to not write to console and to time stamp

            va_list vl;
            va_start(vl, format);
            vsprintf_s(buf, format, vl);
            va_end(vl);

            LOGFILEMGR.GetLog("embed_lua.log", uiFlags)->Msg(buf);
        }

    public:
        template <typename T, typename... Args>
        Result<T> Evaluate(Args... args)
        {
            if (m_pL == nullptr || m_pLuaFormula == nullptr)
            {
                return Result<T>::Error();
            }

            lua_getglobal(m_pL, m_pLuaFormula->GetType());
            if (!lua_isfunction(m_pL, -1))
            {
                LogMsg("%s: cannot find global function", m_pLuaFormula->GetType());
                lua_pop(m_pL, 1);
                return Result<T>::Error();
            }

            Push(m_pL, args...);

            int code = lua_pcall(m_pL, sizeof...(Args), 1, 0); // only support single return value now
            if (code != 0)
            {
                const char* err_msg = lua_tostring(m_pL, -1);
                if (err_msg != nullptr)
                {
                    LogMsg("%s: pcall failed", m_pLuaFormula->GetType());
                }
                lua_pop(m_pL, 1);
                return Result<T>::Error();
            }

            T result = 0;
            if (!Pop(m_pL, &result))
            {
                LogMsg("%s: pop failed", m_pLuaFormula->GetType());
            }
            return Result<T>::Ok(result);
        }

        Evaluator() = default;
        ~Evaluator()
        {
            if (m_pL != nullptr)
            {
                lua_close(m_pL);
                m_pL = nullptr;
            }
        }

        void Swap(Evaluator* other)
        {
            if (this == other)
            {
                return;
            }

            lua_State* tmp_l = this->m_pL;
            CvLuaFormula* tmp_pLuaFormula = this->m_pLuaFormula;

            this->m_pL = other->m_pL;
            this->m_pLuaFormula = other->m_pLuaFormula;
            other->m_pL = tmp_l;
            other->m_pLuaFormula = tmp_pLuaFormula;
        }

        Evaluator& operator=(const Evaluator& other) = delete;

    private:
        CvLuaFormula* m_pLuaFormula = nullptr; // weak pointer;
        lua_State* m_pL = nullptr;
    };

    class EvaluatorFactory
    {
    public:
        bool Init(CvLuaFormula* pFormula, Evaluator* outEvaluator)
        {
            if (pFormula == nullptr || outEvaluator == nullptr)
            {
                return false;
            }

            lua_State* l = luaL_newstate();
            luaL_openlibs(l);

            int code = luaL_loadstring(l, pFormula->GetFormula().c_str());
            if (code != 0)
            {
                lua_close(l);
                return false;
            }
            if (!lua_isfunction(l, -1))
            {
                lua_close(l);
                return false;
            }
            lua_setglobal(l, pFormula->GetType());

            outEvaluator->m_pLuaFormula = pFormula;
            outEvaluator->m_pL = l;
        }
    };

    class EvaluatorManager {
    public:
        Evaluator* GetEvaluator(const char* type);
        Evaluator* GetEvaluator(LuaFormulaTypes eType);

        bool Init(CvGlobals* gc);
    private:
        CvGlobals* m_pGlobal = nullptr; // weak pointer
        std::tr1::unordered_map<LuaFormulaTypes, Evaluator> m_data;
    };
}
