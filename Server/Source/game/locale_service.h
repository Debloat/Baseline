#ifndef __LOCALE_SERVCICE__
#define __LOCALE_SERVCICE__

bool LocaleService_Init(const std::string& c_rstServiceName);
void LocaleService_LoadLocaleStringFile();
void LocaleService_TransferDefaultSetting();
const std::string& LocaleService_GetBasePath();
const std::string& LocaleService_GetMapPath();
const std::string& LocaleService_GetQuestPath();

enum eLocalization
{
    LC_NOSET = 0,

    LC_ENGLISH,
    LC_ARABIA,

    LC_MAX_VALUE
};

eLocalization LC_GetLocalType();

bool LC_IsLocale(const eLocalization t);
bool LC_IsEnglish();
#endif
