/*
* blackdragonx61 / Mali
* 24.10.2022
*/

#pragma once

#include <array>
#include <string>
#include <unordered_map>

class CPythonLocale : public CSingleton<CPythonLocale>
{
    public:
        CPythonLocale();
        virtual ~CPythonLocale();

        enum ELOCALE_STRING_TYPE
        {
            LOCALE_STRING,
            LOCALE_QUEST_STRING,
            LOCALE_OXQUIZ_STRING,

            LOCALE_STRING_MAX
        };

        bool		LoadLocaleString(const char* c_szFileName);
        bool		LoadQuestLocaleString(const char* c_szFileName);
        bool		LoadOXQuizLocaleString(const char* c_szFileName);

        void		FormatString(std::string& sMessage) const;
        void		FormatString(char* c, size_t size) const;

        void		MultiLineSplit(const std::string& sMessage, TTokenVector& vec) const;

    private:
        void		ReplaceSkillName(std::string& sMessage) const;
        void		ReplaceMobName(std::string& sMessage) const;
        void		ReplaceItemName(std::string& sMessage) const;
        void		ReplaceLocaleString(std::string& sMessage) const;
        void		ReplaceQuestLocaleString(std::string& sMessage) const;
        void		ReplaceOXQuizLocaleString(std::string& sMessage) const;

        /* - CLIENT_LOCALE_STRING [REFACTOR] ------------------- */
        bool LoadLocaleStringInternal(const char* c_szFileName, ELOCALE_STRING_TYPE type);

        struct LocaleLoadDesc
        {
            std::size_t tokenCount;   // 2 or 3
            std::size_t keyIndex;     // 0 or 1
            std::size_t valueIndex;   // 1 or 2
            const char* errorContext; // for TraceError
        };

        static const std::array<LocaleLoadDesc, LOCALE_STRING_MAX> ms_loadDesc;

        /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

        void ReplaceLocaleStringInternal(std::string& sMessage, ELOCALE_STRING_TYPE type) const;

        struct LocaleReplaceDesc
        {
            const char* tag;          // "[LS;", "[LC;", "[LOX;"
            std::size_t keyIndex;     // which token is the key? For LS / LC key is token [0], for LOX key is token [1]
            const char* errorContext; // for TraceError
        };

        static const std::array<LocaleReplaceDesc, LOCALE_STRING_MAX> ms_replaceDesc;

        /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

        using LocaleMap = std::unordered_map<std::string, std::string>;
        std::array<LocaleMap, LOCALE_STRING_MAX> m_LocaleStringMap;
        /* ----------------------------------------------------- */
};
