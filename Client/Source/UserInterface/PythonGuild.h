#pragma once

#include "Packet.h"

class CPythonGuild : public CSingleton<CPythonGuild>
{
    public:
        enum
        {
            GUILD_SKILL_MAX_NUM = 12,
            ENEMY_GUILD_SLOT_MAX_COUNT = 6,
        };

        using TGuildInfo = struct SGulidInfo
        {
            DWORD dwGuildID;
            char szGuildName[GUILD_NAME_MAX_LEN + 1];
            DWORD dwMasterPID;
            DWORD dwGuildLevel;
            DWORD dwCurrentExperience;
            DWORD dwCurrentMemberCount;
            DWORD dwMaxMemberCount;
            DWORD dwGuildMoney;
            BOOL bHasLand;
        };

        using TGuildGradeData = struct SGuildGradeData
        {
            SGuildGradeData() {}

            SGuildGradeData(BYTE byAuthorityFlag_, const char* c_szName_) : byAuthorityFlag(byAuthorityFlag_), strName(c_szName_) {}

            BYTE byAuthorityFlag;
            std::string strName;
        };

        using TGradeDataMap = std::map<BYTE, TGuildGradeData>;

        using TGuildMemberData = struct SGuildMemberData
        {
            DWORD dwPID;

            std::string strName;
            BYTE byGrade;
            BYTE byJob;
            BYTE byLevel;
            BYTE byGeneralFlag;
            DWORD dwOffer;
        };

        using TGuildMemberDataVector = std::vector<TGuildMemberData>;

        using TGuildBoardCommentData = struct SGuildBoardCommentData
        {
            DWORD dwCommentID;
            std::string strName;
            std::string strComment;
        };

        using TGuildBoardCommentDataVector = std::vector<TGuildBoardCommentData>;

        using TGuildSkillData = struct SGuildSkillData
        {
            BYTE bySkillPoint;
            BYTE bySkillLevel[GUILD_SKILL_MAX_NUM];
            WORD wGuildPoint;
            WORD wMaxGuildPoint;
        };

        using TGuildNameMap = std::map<DWORD, std::string>;

    public:
        CPythonGuild();
        virtual ~CPythonGuild();

        void Destroy();

        void EnableGuild();
        void SetGuildMoney(DWORD dwMoney);
        void SetGuildEXP(BYTE byLevel, DWORD dwEXP);
        void SetGradeData(BYTE byGradeNumber, TGuildGradeData & rGuildGradeData);
        void SetGradeName(BYTE byGradeNumber, const char* c_szName);
        void SetGradeAuthority(BYTE byGradeNumber, BYTE byAuthority);
        void ClearComment();
        void RegisterComment(DWORD dwCommentID, const char* c_szName, const char* c_szComment);
        void RegisterMember(TGuildMemberData & rGuildMemberData);
        void ChangeGuildMemberGrade(DWORD dwPID, BYTE byGrade);
        void ChangeGuildMemberGeneralFlag(DWORD dwPID, BYTE byFlag);
        void RemoveMember(DWORD dwPID);
        void RegisterGuildName(DWORD dwID, const char* c_szName);

        BOOL IsMainPlayer(DWORD dwPID);
        BOOL IsGuildEnable();
        TGuildInfo& GetGuildInfoRef();
        BOOL GetGradeDataPtr(DWORD dwGradeNumber, TGuildGradeData** ppData);
        const TGuildBoardCommentDataVector& GetGuildBoardCommentVector();
        DWORD GetMemberCount();
        BOOL GetMemberDataPtr(DWORD dwIndex, TGuildMemberData** ppData);
        BOOL GetMemberDataPtrByPID(DWORD dwPID, TGuildMemberData** ppData);
        BOOL GetMemberDataPtrByName(const char* c_szName, TGuildMemberData** ppData);
        DWORD GetGuildMemberLevelSummary();
        DWORD GetGuildMemberLevelAverage();
        DWORD GetGuildExperienceSummary();
        TGuildSkillData& GetGuildSkillDataRef();
        bool GetGuildName(DWORD dwID, std::string * pstrGuildName);
        DWORD GetGuildID();
        BOOL HasGuildLand();

        void StartGuildWar(DWORD dwEnemyGuildID);
        void EndGuildWar(DWORD dwEnemyGuildID);
        DWORD GetEnemyGuildID(DWORD dwIndex);
        BOOL IsDoingGuildWar();

    protected:
        void __CalculateLevelAverage();
        void __SortMember();
        BOOL __IsGradeData(BYTE byGradeNumber);

        void __Initialize();

    protected:
        TGuildInfo m_GuildInfo;
        TGradeDataMap m_GradeDataMap;
        TGuildMemberDataVector m_GuildMemberDataVector;
        TGuildBoardCommentDataVector m_GuildBoardCommentVector;
        TGuildSkillData m_GuildSkillData;
        TGuildNameMap m_GuildNameMap;
        DWORD m_adwEnemyGuildID[ENEMY_GUILD_SLOT_MAX_COUNT];

        DWORD m_dwMemberLevelSummary;
        DWORD m_dwMemberLevelAverage;
        DWORD m_dwMemberExperienceSummary;

        BOOL m_bGuildEnable;
};