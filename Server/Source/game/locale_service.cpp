#include "stdafx.h"
#include "locale_service.h"
#include "constants.h"
#include "banword.h"
#include "utils.h"
#include "mob_manager.h"
#include "config.h"
#include "skill_power.h"

using namespace std;

extern string		g_stQuestDir;
extern set<string> 	g_setQuestObjectDir;

string g_stServiceName;
string g_stServiceBasePath = ".";
string g_stServiceMapPath = "data/map";

string g_stLocale = "euckr";
string g_stLocaleFilename;

BYTE PK_PROTECT_LEVEL = 30;

string 			g_stLocal = "";
eLocalization 	g_eLocalType = LC_NOSET;

int (*check_name)(const char* str) = NULL;
bool LC_InitLocalization(const std::string& szLocal);

int check_name_independent(const char* str)
{
    if (CBanwordManager::instance().CheckString(str, strlen(str)))
    {
        return 0;
    }

    // 몬스터 이름으로는 만들 수 없다.
    char szTmp[256];
    str_lower(str, szTmp, sizeof(szTmp));

    if (CMobManager::instance().Get(szTmp, false))
    {
        return 0;
    }

    return 1;
}

int check_name_latin1(const char* str)
{
    int		code;
    const char*	tmp;

    if (!str || !*str)
    {
        return 0;
    }

    if (strlen(str) < 2)
    {
        return 0;
    }

    for (tmp = str; *tmp; ++tmp)
    {
        // 한글이 아니고 빈칸이면 잘못된 것
        if (isspace(*tmp))
        {
            return 0;
        }

        // 한글이 아니고 숫자라면 적합하다.
        if (isdigit(*tmp))
        {
            continue;
        }

        // 한글이 아니고 영문이라면 적합하다.
        if (isalpha(*tmp))
        {
            continue;
        }

        unsigned char uc_tmp = *tmp;

        if (uc_tmp == 145 || uc_tmp == 146 || uc_tmp == 196
        || uc_tmp == 214 || uc_tmp == 220 || uc_tmp == 223
        || uc_tmp == 228 || uc_tmp == 246 || uc_tmp == 252)
        {
            continue;
        }

        code = *tmp;
        code += 256;

        if (code < 176 || code > 200)
        {
            return 0;
        }

        ++tmp;

        if (!*tmp)
        {
            break;
        }
    }

    return check_name_independent(str);
}

int check_name_alphabet(const char* str)
{
    const char*	tmp;

    if (!str || !*str)
    {
        return 0;
    }

    if (strlen(str) < 2)
    {
        return 0;
    }

    for (tmp = str; *tmp; ++tmp)
    {
        // 알파벳과 수자만 허용
        if (isdigit(*tmp) || isalpha(*tmp))
        {
            continue;
        }

        else
        {
            return 0;
        }
    }

    return check_name_independent(str);
}

void LocaleService_LoadLocaleStringFile()
{
    if (g_stLocaleFilename.empty())
    {
        return;
    }

    if (g_bAuthServer)
    {
        return;
    }

    fprintf(stderr, "LocaleService %s\n", g_stLocaleFilename.c_str());

    locale_init(g_stLocaleFilename.c_str());
}

static void __LocaleService_Init_DEFAULT()
{
    g_stLocaleFilename = "";

    g_stServiceBasePath = "locale/" + g_stServiceName;
    g_stServiceMapPath = g_stServiceBasePath + "/map";
    g_stQuestDir = g_stServiceBasePath + "/quest";

    g_setQuestObjectDir.clear();
    g_setQuestObjectDir.insert(g_stQuestDir + "/object");
}

static void __LocaleService_Init_English()
{
    g_stLocale = "";
    g_stServiceBasePath = "locale/english";
    g_stQuestDir = "locale/english/quest";
    g_stServiceMapPath = "locale/english/map";

    g_setQuestObjectDir.clear();
    g_setQuestObjectDir.insert("locale/english/quest/object");
    g_stLocaleFilename = "locale/english/eng_string.txt";

    check_name = check_name_alphabet;
}

static void __LocaleService_Init_YMIR()
{
    g_stLocaleFilename = "";

    g_stServiceBasePath = "locale/" + g_stServiceName;
    g_stServiceMapPath = g_stServiceBasePath + "/map";
    g_stQuestDir = g_stServiceBasePath + "/quest";

    g_setQuestObjectDir.clear();
    g_setQuestObjectDir.insert(g_stQuestDir + "/object");

    PK_PROTECT_LEVEL = 30;

    exp_table = exp_table_common;
}

static void __LocaleService_Init_Arabia()
{
    g_stLocale = "cp1256";
    g_stServiceBasePath = "locale/arabia";
    g_stQuestDir = "locale/arabia/quest";
    g_stServiceMapPath = "locale/arabia/map";

    g_setQuestObjectDir.clear();
    g_setQuestObjectDir.insert("locale/arabia/quest/object");
    g_stLocaleFilename = "locale/arabia/locale_string.txt";

    check_name = check_name_alphabet;

    PK_PROTECT_LEVEL = 15;
}

static void __CheckPlayerSlot(const std::string& service_name)
{
    if (PLAYER_PER_ACCOUNT != 4)
    {
        printf("<ERROR> PLAYER_PER_ACCOUNT = %d\n", PLAYER_PER_ACCOUNT);
        exit(0);
    }
}

bool LocaleService_Init(const std::string& c_rstServiceName)
{
    if (!g_stServiceName.empty())
    {
        sys_err("ALREADY exist service");
        return false;
    }

    g_stServiceName = c_rstServiceName;

    if ("english" == g_stServiceName)
    {
        __LocaleService_Init_English();
    }

    else if ("ymir" == g_stServiceName)
    {
        __LocaleService_Init_YMIR();
    }

    else if ("arabia" == g_stServiceName)
    {
        __LocaleService_Init_Arabia();
    }

    else
    {
        __LocaleService_Init_DEFAULT();
    }

    fprintf(stdout, "Setting Locale \"%s\" (Path: %s)\n", g_stServiceName.c_str(), g_stServiceBasePath.c_str());

    __CheckPlayerSlot(g_stServiceName);

    if (false == LC_InitLocalization(c_rstServiceName))
    {
        return false;
    }

    return true;
}

void LocaleService_TransferDefaultSetting()
{
    if (!check_name)
    {
        check_name = check_name_alphabet;
    }

    if (!exp_table)
    {
        exp_table = exp_table_common;
    }

    if (!CTableBySkill::instance().Check())
    {
        exit(1);
    }

    if (!aiPercentByDeltaLevForBoss)
    {
        aiPercentByDeltaLevForBoss = aiPercentByDeltaLevForBoss_euckr;
    }

    if (!aiPercentByDeltaLev)
    {
        aiPercentByDeltaLev = aiPercentByDeltaLev_euckr;
    }

    if (!aiChainLightningCountBySkillLevel)
    {
        aiChainLightningCountBySkillLevel = aiChainLightningCountBySkillLevel_euckr;
    }
}

const std::string& LocaleService_GetBasePath()
{
    return g_stServiceBasePath;
}

const std::string& LocaleService_GetMapPath()
{
    return g_stServiceMapPath;
}

const std::string& LocaleService_GetQuestPath()
{
    return g_stQuestDir;
}

bool LC_InitLocalization(const std::string& szLocal)
{
    g_stLocal = szLocal;

    if (!g_stLocal.compare("english"))
    {
        g_eLocalType = LC_ENGLISH;
    }

    else if (!g_stLocal.compare("arabia"))
    {
        g_eLocalType = LC_ARABIA;
    }

    else
    {
        return false;
    }

    return true;
}

eLocalization LC_GetLocalType()
{
    return g_eLocalType;
}

bool LC_IsLocale(const eLocalization t)
{
    return LC_GetLocalType() == t ? true : false;
}

bool LC_IsEnglish()
{
    return LC_GetLocalType() == LC_ENGLISH ? true : false;
}
