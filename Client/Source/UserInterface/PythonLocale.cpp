/*
* blackdragonx61 / Mali
* 24.10.2022
*/

/*
* Refactored by KaptanYosun @MMOTutkunlari.com
* 21.01.2026
*/

#include "StdAfx.h"
#include "PythonLocale.h"
#include "PythonNonPlayer.h"
#include "PythonSkill.h"
#include "../EterPack/EterPackManager.h"
#include "../GameLib/ItemManager.h"

CPythonLocale::CPythonLocale() = default;

CPythonLocale::~CPythonLocale() = default;

/* - CLIENT_LOCALE_STRING [REFACTOR] ------------------- */
const std::array<CPythonLocale::LocaleLoadDesc, CPythonLocale::LOCALE_STRING_MAX> CPythonLocale::ms_loadDesc =
{
    {
        { 2, 0, 1, "LoadLocaleString" },       /* LOCALE_STRING */
        { 2, 0, 1, "LoadQuestLocaleString" },  /* LOCALE_QUEST_STRING */
        { 3, 1, 2, "LoadOXQuizLocaleString" }, /* LOCALE_OXQUIZ_STRING */
    }
};

const std::array<CPythonLocale::LocaleReplaceDesc, CPythonLocale::LOCALE_STRING_MAX> CPythonLocale::ms_replaceDesc =
{
    {
        { "[LS;",  0, "ReplaceLocaleString" },       /* LOCALE_STRING */
        { "[LC;",  0, "ReplaceQuestLocaleString" },  /* LOCALE_QUEST_STRING */
        { "[LOX;", 1, "ReplaceOXQuizLocaleString" }, /* LOCALE_OXQUIZ_STRING */
    }
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

static bool ContainsPrintfStyleFormat(std::string_view s)
{
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] != '%')
            continue;

        // Allow escaped percent "%%"
        if (i + 1 < s.size() && s[i + 1] == '%')
        {
            ++i; // skip the second '%'
            continue;
        }

        // We saw a '%' that is NOT '%%'
        // Try to parse a printf-style specifier

        std::size_t j = i + 1;

        // 1) Skip flags: -+ #0
        while (j < s.size() && (s[j] == '-' || s[j] == '+' || s[j] == ' ' ||
        s[j] == '#' || s[j] == '0'))
        {
            ++j;
        }

        // 2) Skip width: digits
        while (j < s.size() && std::isdigit(static_cast<unsigned char>(s[j])))
        {
            ++j;
        }

        // 3) Skip precision: .digits
        if (j < s.size() && s[j] == '.')
        {
            ++j;

            while (j < s.size() && std::isdigit(static_cast<unsigned char>(s[j])))
            {
                ++j;
            }
        }

        // 4) Skip length modifiers: h, hh, l, ll, z, t, j, L
        if (j < s.size())
        {
            if (s[j] == 'h' || s[j] == 'l')
            {
                char c = s[j];
                ++j;

                if (j < s.size() && s[j] == c) // hh or ll
                    ++j;
            }

            else if (s[j] == 'z' || s[j] == 't' || s[j] == 'j' || s[j] == 'L')
            {
                ++j;
            }
        }

        // 5) Conversion specifier
        if (j < s.size())
        {
            switch (s[j])
            {
                case 'd':
                case 'i':
                case 'u':
                case 'o':
                case 'x':
                case 'X':
                case 'f':
                case 'F':
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                case 'a':
                case 'A':
                case 'c':
                case 's':
                case 'p':
                case 'n':
                    return true; // real printf-style format detected

                default:
                    break;
            }
        }
    }

    return false;
}

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

bool CPythonLocale::LoadLocaleStringInternal(const char* c_szFileName, ELOCALE_STRING_TYPE type)
{
    auto& map = m_LocaleStringMap[type];

    if (!map.empty())
        return true;

    const auto& desc = ms_loadDesc[type];

    const VOID* pvData;
    CMappedFile kFile;

    if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
    {
        TraceError("CPythonLocale::%s(c_szFileName: %s) - Load Error", desc.errorContext, c_szFileName);
        return false;
    }

    CMemoryTextFileLoader kTextFileLoader;
    kTextFileLoader.Bind(kFile.Size(), pvData);

    CTokenVector kTokenVector;

    for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
    {
        kTokenVector.clear();

        const std::string& line = kTextFileLoader.GetLineString(i);

        if (line.empty())
            continue;

        if (line[0] == '#')
            continue;

        if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
            continue;

        if (kTokenVector.size() != desc.tokenCount)
            continue;

        const std::string& key = kTokenVector[desc.keyIndex];
        const std::string& value = kTokenVector[desc.valueIndex];

        if (ContainsPrintfStyleFormat(value))
        {
            TraceError(
                "\n\n"
                "============================================================\n"
                "[CPythonLocale::%s]\n"
                "Invalid locale format detected (printf-style format is NOT allowed)\n"
                "\n"
                "Key   : %s\n"
                "Value : %s\n"
                "\n"
                "Please replace legacy printf-style specifiers (%%d, %%u, %%lu, etc.)\n"
                "with modern '{}' placeholders.\n"
                "============================================================"
                "\n\n",
                desc.errorContext,
                key.c_str(),
                value.c_str()
            );

            continue;
        }

        map[key] = value;
    }

    Tracenf("[DEBUG] %s loaded %zu entries from %s", desc.errorContext, map.size(), c_szFileName);

    return true;
}

bool CPythonLocale::LoadLocaleString(const char* c_szFileName)
{
    return LoadLocaleStringInternal(c_szFileName, LOCALE_STRING);
}

bool CPythonLocale::LoadQuestLocaleString(const char* c_szFileName)
{
    return LoadLocaleStringInternal(c_szFileName, LOCALE_QUEST_STRING);
}

bool CPythonLocale::LoadOXQuizLocaleString(const char* c_szFileName)
{
    return LoadLocaleStringInternal(c_szFileName, LOCALE_OXQUIZ_STRING);
}

/* ----------------------------------------------------- */

void CPythonLocale::FormatString(std::string& sMessage) const
{
    ReplaceSkillName(sMessage);
    ReplaceMobName(sMessage);
    ReplaceItemName(sMessage);
    ReplaceLocaleString(sMessage);
    ReplaceQuestLocaleString(sMessage);
    ReplaceOXQuizLocaleString(sMessage);
}

void CPythonLocale::FormatString(char* c, size_t size) const
{
    std::string sFormat{ c };
    FormatString(sFormat);

    strncpy(c, sFormat.c_str(), size);
    c[size - 1] = '\0';
}

/* - CLIENT_LOCALE_STRING [REFACTOR] ------------------- */
static std::string FormatStd(std::string_view fmt, const std::vector<std::string>& args)
{
    std::string result(fmt);

    size_t pos = 0;
    size_t argIndex = 0;

    while ((pos = result.find("{}", pos)) != std::string::npos)
    {
        if (argIndex >= args.size())
            break; // too few arguments, leave {} as-is

        result.replace(pos, 2, args[argIndex]);
        pos += args[argIndex].size();
        ++argIndex;
    }

    return result;
}

/* ----------------------------------------------------- */

void CPythonLocale::MultiLineSplit(const std::string& sMessage, TTokenVector& vec) const
{
    if (!sMessage.contains("[ENTER]"))
        return;

    size_t v15 = 0;

    while (true)
    {
        const size_t pos = sMessage.find("[ENTER]", v15);

        if (pos == std::string::npos)
            break;

        vec.push_back(sMessage.substr(v15, pos - v15));
        v15 = pos + 7;
    }

    if (v15 < sMessage.size())
        vec.push_back(sMessage.substr(v15));
}

void CPythonLocale::ReplaceSkillName(std::string& sMessage) const
{
    while (true)
    {
        size_t pos_begin = sMessage.find("[SN;");

        if (pos_begin == std::string::npos)
            break;

        size_t pos_mid = sMessage.find(';', pos_begin);

        if (pos_mid == std::string::npos)
            break;

        size_t pos_end = sMessage.find(']', ++pos_mid);

        if (pos_end == std::string::npos)
            break;

        DWORD dwVnum{};

        try
        {
            dwVnum = std::stoul(sMessage.substr(pos_mid, pos_end - pos_mid));
        }

        catch (const std::exception& ex)
        {
            TraceError("CPythonLocale::ReplaceSkillName: Error: %s", ex.what());
            break;
        }

        CPythonSkill::TSkillData* pSkillData;

        if (!CPythonSkill::Instance().GetSkillData(dwVnum, &pSkillData))
        {
            TraceError("CPythonLocale::ReplaceSkillName: can't find skill vnum: %lu", dwVnum);
            break;
        }

        sMessage.replace(pos_begin, (pos_end + 1) - pos_begin, pSkillData->strName);
    }
}

void CPythonLocale::ReplaceMobName(std::string& sMessage) const
{
    while (true)
    {
        size_t pos_begin = sMessage.find("[MN;");

        if (pos_begin == std::string::npos)
            break;

        size_t pos_mid = sMessage.find(';', pos_begin);

        if (pos_mid == std::string::npos)
            break;

        size_t pos_end = sMessage.find(']', ++pos_mid);

        if (pos_end == std::string::npos)
            break;

        DWORD dwVnum{};

        try
        {
            dwVnum = std::stoul(sMessage.substr(pos_mid, pos_end - pos_mid));
        }

        catch (const std::exception& ex)
        {
            TraceError("CPythonLocale::ReplaceMobName: Error: %s", ex.what());
            break;
        }

        const char* c_szName;

        if (!CPythonNonPlayer::Instance().GetName(dwVnum, &c_szName))
        {
            TraceError("CPythonLocale::ReplaceMobName: can't find mob vnum: %lu", dwVnum);
            break;
        }

        sMessage.replace(pos_begin, (pos_end + 1) - pos_begin, c_szName);
    }
}

void CPythonLocale::ReplaceItemName(std::string& sMessage) const
{
    while (true)
    {
        size_t pos_begin = sMessage.find("[IN;");

        if (pos_begin == std::string::npos)
            break;

        size_t pos_mid = sMessage.find(';', pos_begin);

        if (pos_mid == std::string::npos)
            break;

        size_t pos_end = sMessage.find(']', ++pos_mid);

        if (pos_end == std::string::npos)
            break;

        DWORD dwVnum{};

        try
        {
            dwVnum = std::stoul(sMessage.substr(pos_mid, pos_end - pos_mid));
        }

        catch (const std::exception& ex)
        {
            TraceError("CPythonLocale::ReplaceItemName: Error: %s", ex.what());
            break;
        }

        CItemData* pItemData;

        if (!CItemManager::Instance().GetItemDataPointer(dwVnum, &pItemData))
        {
            TraceError("CPythonLocale::ReplaceItemName: can't find item vnum: %lu", dwVnum);
            break;
        }

        sMessage.replace(pos_begin, (pos_end + 1) - pos_begin, pItemData->GetName());
    }
}

/* - CLIENT_LOCALE_STRING [REFACTOR] ------------------- */
void CPythonLocale::ReplaceLocaleStringInternal(std::string& sMessage, ELOCALE_STRING_TYPE type) const
{
    const auto& desc = ms_replaceDesc[type];
    auto& map = m_LocaleStringMap[type];

    while (true)
    {
        size_t pos_1 = sMessage.rfind(desc.tag);

        if (pos_1 == std::string::npos)
            break;

        size_t pos_2 = sMessage.find(';', pos_1);

        if (pos_2 == std::string::npos)
            break;

        ++pos_2;

        size_t pos_3 = sMessage.find('[', pos_2);
        size_t pos_4 = sMessage.find(']', pos_2);

        if (pos_4 == std::string::npos)
            break;

        if (pos_3 != std::string::npos && pos_3 < pos_4)
            pos_4 = sMessage.find(']', pos_4 + 1);

        std::string sArgs = sMessage.substr(pos_2, pos_4 - pos_2);

        CTokenVector tokens;
        SplitLine(sArgs.c_str(), ";", &tokens);

        if (tokens.size() <= desc.keyIndex)
            break;

        const std::string& key = tokens[desc.keyIndex];

        auto it = map.find(key);

        if (it == map.end())
        {
            TraceError("CPythonLocale::%s wrong key : %s", desc.errorContext, key.c_str());
            break;
        }

        if (tokens.size() > desc.keyIndex + 1)
        {
            std::vector<std::string> args(tokens.begin() + desc.keyIndex + 1, tokens.end());

            const std::string formatted = FormatStd(it->second, args);

            sMessage.replace(pos_1, (pos_4 + 1) - pos_1, formatted);
        }

        else
        {
            sMessage.replace(pos_1, (pos_4 + 1) - pos_1, it->second);
        }
    }
}

void CPythonLocale::ReplaceLocaleString(std::string& sMessage) const
{
    ReplaceLocaleStringInternal(sMessage, LOCALE_STRING);
}

void CPythonLocale::ReplaceQuestLocaleString(std::string& sMessage) const
{
    ReplaceLocaleStringInternal(sMessage, LOCALE_QUEST_STRING);
}

void CPythonLocale::ReplaceOXQuizLocaleString(std::string& sMessage) const
{
    ReplaceLocaleStringInternal(sMessage, LOCALE_OXQUIZ_STRING);
}

/* ----------------------------------------------------- */
