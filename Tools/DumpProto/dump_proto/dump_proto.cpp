#include "dump_proto.h"
#include "utils.h"

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
     ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

using BYTE = unsigned char;
using WORD = unsigned short;
using DWORD = unsigned long;

enum EMisc
{
    CHARACTER_NAME_MAX_LEN = 24,
    MOB_SKILL_MAX_NUM = 5,
};

enum EMobEnchants
{
    MOB_ENCHANT_CURSE,
    MOB_ENCHANT_SLOW,
    MOB_ENCHANT_POISON,
    MOB_ENCHANT_STUN,
    MOB_ENCHANT_CRITICAL,
    MOB_ENCHANT_PENETRATE,
    MOB_ENCHANTS_MAX_NUM
};

enum EMobResists
{
    MOB_RESIST_SWORD,
    MOB_RESIST_TWOHAND,
    MOB_RESIST_DAGGER,
    MOB_RESIST_BELL,
    MOB_RESIST_FAN,
    MOB_RESIST_BOW,
    MOB_RESIST_FIRE,
    MOB_RESIST_ELECT,
    MOB_RESIST_MAGIC,
    MOB_RESIST_WIND,
    MOB_RESIST_POISON,
    MOB_RESISTS_MAX_NUM
};


#pragma pack(1)
using TMobSkillLevel = struct SMobSkillLevel
{
    DWORD	dwVnum;
    BYTE	bLevel;
};

#pragma pack()

#pragma pack(1)
using TMobTable = struct SMobTable
{
    DWORD	dwVnum;
    char	szName[CHARACTER_NAME_MAX_LEN + 1];

    BYTE	bType;			// Monster, NPC
    BYTE	bRank;			// PAWN, KNIGHT, KING
    BYTE	bBattleType;		// MELEE, etc..
    BYTE	bLevel;			// Level
    BYTE	bSize;

    DWORD	dwGoldMin;
    DWORD	dwGoldMax;
    DWORD	dwExp;
    DWORD	dwMaxHP;
    BYTE	bRegenCycle;
    BYTE	bRegenPercent;
    WORD	wDef;

    DWORD	dwAIFlag;
    DWORD	dwRaceFlag;
    DWORD	dwImmuneFlag;

    BYTE	bStr, bDex, bCon, bInt;
    DWORD	dwDamageRange[2];

    short	sAttackSpeed;
    short	sMovingSpeed;
    BYTE	bAggresiveHPPct;
    WORD	wAggressiveSight;
    WORD	wAttackRange;

    char	cEnchants[MOB_ENCHANTS_MAX_NUM];
    char	cResists[MOB_RESISTS_MAX_NUM];

    DWORD	dwResurrectionVnum;
    DWORD	dwDropItemVnum;

    BYTE	bMountCapacity;
    BYTE	bOnClickType;

    BYTE	bEmpire;
    char	szFolder[64 + 1];

    float	fDamMultiply;

    DWORD	dwSummonVnum;
    DWORD	dwDrainSP;
    DWORD	dwMobColor;
    DWORD	dwPolymorphItemVnum;

    TMobSkillLevel Skills[MOB_SKILL_MAX_NUM];

    BYTE	bBerserkPoint;
    BYTE	bStoneSkinPoint;
    BYTE	bGodSpeedPoint;
    BYTE	bDeathBlowPoint;
    BYTE	bRevivePoint;
};

#pragma pack()


using namespace std;

TMobTable* m_pMobTable = NULL;
int m_iMobTableSize = 0;

enum EItemMisc
{
    ITEM_NAME_MAX_LEN = 24,
    ITEM_VALUES_MAX_NUM = 6,
    ITEM_SMALL_DESCR_MAX_LEN = 256,
    ITEM_LIMIT_MAX_NUM = 2,
    ITEM_APPLY_MAX_NUM = 3,
    ITEM_SOCKET_MAX_NUM = 3,
    ITEM_MAX_COUNT = 200,
    ITEM_ATTRIBUTE_MAX_NUM = 7,
    ITEM_ATTRIBUTE_MAX_LEVEL = 5,
    ITEM_AWARD_WHY_MAX_LEN = 50,

    REFINE_MATERIAL_MAX_NUM = 5,

    ITEM_ELK_VNUM = 50026,
};
#pragma pack(1)
using TItemLimit = struct SItemLimit
{
    BYTE	bType;
    long	lValue;
};

#pragma pack()

#pragma pack(1)
using TItemApply = struct SItemApply
{
    BYTE	bType;
    long	lValue;
};

#pragma pack()

#pragma pack(1)
using TClientItemTable = struct
{
    DWORD       dwVnum;
    DWORD		dwVnumRange;
    char        szName[ITEM_NAME_MAX_LEN + 1];
    BYTE	bType;
    BYTE	bSubType;

    BYTE        bWeight;
    BYTE	bSize;

    DWORD	dwAntiFlags;
    DWORD	dwFlags;
    DWORD	dwWearFlags;
    DWORD	dwImmuneFlag;

    DWORD       dwGold;
    DWORD       dwShopBuyPrice;

    TItemLimit	aLimits[ITEM_LIMIT_MAX_NUM];
    TItemApply	aApplies[ITEM_APPLY_MAX_NUM];
    long        alValues[ITEM_VALUES_MAX_NUM];
    long	alSockets[ITEM_SOCKET_MAX_NUM];
    DWORD	dwRefinedVnum;
    WORD	wRefineSet;
    BYTE	bAlterToMagicItemPct;
    BYTE	bSpecular;
    BYTE	bGainSocketPct;
};

#pragma pack()
bool	operator < (const TClientItemTable& lhs, const TClientItemTable& rhs)
{
    return lhs.dwVnum < rhs.dwVnum;
}

TClientItemTable* m_pItemTable = NULL;
int m_iItemTableSize = 0;


bool Set_Proto_Mob_Table(TMobTable* mobTable, cCsvTable& csvTable, const std::unordered_map<int, std::string_view>& nameMap)
{
    int col = 0;

    mobTable->dwVnum = atoi(csvTable.AsStringByIndex(col++));

    /*======== NAME =======*/
    strncpy(mobTable->szName, csvTable.AsStringByIndex(col++), CHARACTER_NAME_MAX_LEN);

    // Overwrite name if localized name exists
    if (auto it = nameMap.find(mobTable->dwVnum); it != nameMap.end())
    {
        const std::string_view name = it->second;
        strncpy(mobTable->szName, name.data(), CHARACTER_NAME_MAX_LEN);
    }

    //4. RANK
    int rankValue = get_Mob_Rank_Value(csvTable.AsStringByIndex(col++));
    mobTable->bRank = rankValue;
    //5. TYPE
    int typeValue = get_Mob_Type_Value(csvTable.AsStringByIndex(col++));
    mobTable->bType = typeValue;
    //6. BATTLE_TYPE
    int battleTypeValue = get_Mob_BattleType_Value(csvTable.AsStringByIndex(col++));
    mobTable->bBattleType = battleTypeValue;

    mobTable->bLevel = atoi(csvTable.AsStringByIndex(col++));
    //8. SIZE
    int sizeValue = get_Mob_Size_Value(csvTable.AsStringByIndex(col++));
    mobTable->bSize = sizeValue;
    //9. AI_FLAG
    int aiFlagValue = get_Mob_AIFlag_Value(csvTable.AsStringByIndex(col++));
    mobTable->dwAIFlag = aiFlagValue;
    col++; //mount_capacity;
    //10. RACE_FLAG
    int raceFlagValue = get_Mob_RaceFlag_Value(csvTable.AsStringByIndex(col++));
    mobTable->dwRaceFlag = raceFlagValue;
    //11. IMMUNE_FLAG
    int immuneFlagValue = get_Mob_ImmuneFlag_Value(csvTable.AsStringByIndex(col++));
    mobTable->dwImmuneFlag = immuneFlagValue;

    mobTable->bEmpire = atoi(csvTable.AsStringByIndex(col++));

    //folder
    strncpy(mobTable->szFolder, csvTable.AsStringByIndex(col++), sizeof(mobTable->szFolder));


    mobTable->bOnClickType = atoi(csvTable.AsStringByIndex(col++));

    mobTable->bStr = atoi(csvTable.AsStringByIndex(col++));
    mobTable->bDex = atoi(csvTable.AsStringByIndex(col++));
    mobTable->bCon = atoi(csvTable.AsStringByIndex(col++));
    mobTable->bInt = atoi(csvTable.AsStringByIndex(col++));
    mobTable->dwDamageRange[0] = atoi(csvTable.AsStringByIndex(col++));
    mobTable->dwDamageRange[1] = atoi(csvTable.AsStringByIndex(col++));
    mobTable->dwMaxHP = atoi(csvTable.AsStringByIndex(col++));
    mobTable->bRegenCycle = atoi(csvTable.AsStringByIndex(col++));
    mobTable->bRegenPercent = atoi(csvTable.AsStringByIndex(col++));

    col++;	//gold min
    col++;	//gold max
    mobTable->dwExp = atoi(csvTable.AsStringByIndex(col++));
    mobTable->wDef = atoi(csvTable.AsStringByIndex(col++));
    mobTable->sAttackSpeed = atoi(csvTable.AsStringByIndex(col++));
    mobTable->sMovingSpeed = atoi(csvTable.AsStringByIndex(col++));
    mobTable->bAggresiveHPPct = atoi(csvTable.AsStringByIndex(col++));
    mobTable->wAggressiveSight = atoi(csvTable.AsStringByIndex(col++));
    mobTable->wAttackRange = atoi(csvTable.AsStringByIndex(col++));

    mobTable->dwDropItemVnum = atoi(csvTable.AsStringByIndex(col++));
    col++;	//resurrectionVnum


    for (int i = 0; i < MOB_ENCHANTS_MAX_NUM; ++i)
    {
        mobTable->cEnchants[i] = atoi(csvTable.AsStringByIndex(col++));
    }

    for (int i = 0; i < MOB_RESISTS_MAX_NUM; ++i)
    {
        mobTable->cResists[i] = atoi(csvTable.AsStringByIndex(col++));
    }

    mobTable->fDamMultiply = atoi(csvTable.AsStringByIndex(col++));
    mobTable->dwSummonVnum = atoi(csvTable.AsStringByIndex(col++));
    mobTable->dwDrainSP = atoi(csvTable.AsStringByIndex(col++));
    mobTable->dwMobColor = atoi(csvTable.AsStringByIndex(col++));

    return true;
}


static bool BuildMobTable()
{
    /* [KaptanYosun Dev Note]
        File formats:
            mob_names.txt :
                VNUM<TAB>NAME

            mob_proto.txt :
                Monster prototype table (tab-separated, first row = header)

        Processing steps:
        [1] Load mob_names.txt and build a local VNUM -> NAME lookup map.
        [2] Load mob_proto.txt and validate file access.
        [3] Reserve capacity for temporary mob vector.
        [4] Emplace and parse each mob row.
        [5] Dump generated mob data.
    */

    fprintf(stderr, "sizeof(TMobTable): %u\n", sizeof(TMobTable));

    /* - [1] ----------------------------------------------- */
    std::unordered_map<int, std::string_view> localMap;
    cCsvTable nameData;

    if (!nameData.Load("mob_names.txt", '\t'))
    {
        fprintf(stderr, "Failed to load mob_names.txt\n");
    }

    else
    {
        nameData.Next(); // skip header

        while (nameData.Next())
        {
            int vnum = 0;

            if (!str_to_number(vnum, nameData.AsStringByIndex(0)))
            {
                continue;
            }

            localMap.try_emplace(vnum, nameData.AsStringByIndex(1));
        }
    }

    /* ----------------------------------------------------- */


    /* - [2] ----------------------------------------------- */
    cCsvTable data;

    if (!data.Load("mob_proto.txt", '\t'))
    {
        fprintf(stderr, "Failed to load mob_proto.txt\n");
        return false;
    }

    data.Next(); // skip header
    /* ----------------------------------------------------- */


    /* - [3] ----------------------------------------------- */
    if (m_pMobTable)
    {
        delete[] m_pMobTable;
        m_pMobTable = nullptr;
        m_iMobTableSize = 0;
    }

    const std::size_t rowCount = data.m_File.GetRowCount() - 1;

    std::vector<TMobTable> mobs;
    mobs.reserve(rowCount);
    /* ----------------------------------------------------- */


    /* - [4] ----------------------------------------------- */
    while (data.Next())
    {
        mobs.emplace_back();
        TMobTable& mob = mobs.back();

        if (!Set_Proto_Mob_Table(&mob, data, localMap))
        {
            fprintf(stderr, "Failed to parse mob_proto row.\n");
            mobs.pop_back();
            continue;
        }
    }

    /* ----------------------------------------------------- */


    /* - [5] ----------------------------------------------- */
    m_iMobTableSize = static_cast<int>(mobs.size());
    m_pMobTable = new TMobTable[m_iMobTableSize];
    std::memcpy(m_pMobTable, mobs.data(),
                sizeof(TMobTable) * m_iMobTableSize);

    for (const auto& mob : mobs)
    {
        fprintf(stdout,
                "MOB #%-5d %-16s sight: %u color %u\n",
                mob.dwVnum,
                mob.szName,
                mob.wAggressiveSight,
                mob.dwMobColor);
    }

    /* ----------------------------------------------------- */

    return true;
}

DWORD g_adwMobProtoKey[4] =
{
    4813894,
    18955,
    552631,
    6822045
};


void SaveMobProto()
{
    FILE* fp;

    fp = fopen("mob_proto", "wb");

    if (!fp)
    {
        printf("cannot open %s for writing\n", "mob_proto");
        return;
    }

    DWORD fourcc = MAKEFOURCC('M', 'M', 'P', 'T');
    fwrite(&fourcc, sizeof(DWORD), 1, fp);

    DWORD dwElements = m_iMobTableSize;
    fwrite(&dwElements, sizeof(DWORD), 1, fp);

    CLZObject zObj;

    printf("sizeof(TMobTable) %d\n", sizeof(TMobTable));

    if (!CLZO::instance().CompressEncryptedMemory(zObj, m_pMobTable, sizeof(TMobTable) * m_iMobTableSize, g_adwMobProtoKey))
    {
        printf("cannot compress\n");
        fclose(fp);
        return;
    }

    const CLZObject::THeader& r = zObj.GetHeader();

    printf("MobProto count %u\n%u --Compress--> %u --Encrypt--> %u, GetSize %u\n",
           m_iMobTableSize, r.dwRealSize, r.dwCompressedSize, r.dwEncryptSize, zObj.GetSize());

    DWORD dwDataSize = zObj.GetSize();
    fwrite(&dwDataSize, sizeof(DWORD), 1, fp);
    fwrite(zObj.GetBuffer(), dwDataSize, 1, fp);

    fclose(fp);
}

void LoadMobProto()
{
    FILE* fp;
    DWORD fourcc, tableSize, dataSize;

    fp = fopen("mob_proto", "rb");

    fread(&fourcc, sizeof(DWORD), 1, fp);
    fread(&tableSize, sizeof(DWORD), 1, fp);
    fread(&dataSize, sizeof(DWORD), 1, fp);
    BYTE* data = (BYTE*)malloc(dataSize);

    if (data)
    {
        fread(data, dataSize, 1, fp);

        CLZObject zObj;

        if (CLZO::instance().Decompress(zObj, data, g_adwMobProtoKey))
        {
            printf("real_size %u\n", zObj.GetSize());

            for (DWORD i = 0; i < tableSize; ++i)
            {
                TMobTable& rTable = *((TMobTable*)zObj.GetBuffer() + i);
                printf("%u %s\n", rTable.dwVnum, rTable.szName);
            }
        }

        free(data);
    }

    fclose(fp);
}


//==													==//
//==													==//
//==													==//
//===== 여기에서부터 아이템 =====//
//==													==//
//==													==//
//==													==//

static bool Set_Proto_Item_Table(TClientItemTable* itemTable, cCsvTable& csvTable, const std::unordered_map<int, std::string_view>& nameMap)
{
    // vnum 및 vnum range 읽기.
    {
        std::string s(csvTable.AsStringByIndex(0));
        int pos = s.find("~");

        // vnum 필드에 '~'가 없다면 패스
        if (std::string::npos == pos)
        {
            itemTable->dwVnum = atoi(s.c_str());

            if (0 == itemTable->dwVnum)
            {
                printf("INVALID VNUM %s\n", s.c_str());
                return false;
            }

            itemTable->dwVnumRange = 0;
        }

        else
        {
            std::string s_start_vnum(s.substr(0, pos));
            std::string s_end_vnum(s.substr(pos + 1));

            int start_vnum = atoi(s_start_vnum.c_str());
            int end_vnum = atoi(s_end_vnum.c_str());

            if (0 == start_vnum || (0 != end_vnum && end_vnum < start_vnum))
            {
                printf("INVALID VNUM RANGE%s\n", s.c_str());
                return false;
            }

            itemTable->dwVnum = start_vnum;
            itemTable->dwVnumRange = end_vnum - start_vnum;
        }
    }

    int col = 1;

    strncpy(itemTable->szName, csvTable.AsStringByIndex(col++), ITEM_NAME_MAX_LEN);
    // Overwrite name with localized name if present in item_names.txt
    auto it = nameMap.find(itemTable->dwVnum);

    if (it != nameMap.end())
    {
        const std::string_view name = it->second;
        strncpy(itemTable->szName, name.data(), ITEM_NAME_MAX_LEN);
    }

    itemTable->bType = get_Item_Type_Value(csvTable.AsStringByIndex(col++));
    itemTable->bSubType = get_Item_SubType_Value(itemTable->bType, csvTable.AsStringByIndex(col++));
    itemTable->bSize = atoi(csvTable.AsStringByIndex(col++));
    itemTable->dwAntiFlags = get_Item_AntiFlag_Value(csvTable.AsStringByIndex(col++));
    itemTable->dwFlags = get_Item_Flag_Value(csvTable.AsStringByIndex(col++));
    itemTable->dwWearFlags = get_Item_WearFlag_Value(csvTable.AsStringByIndex(col++));
    itemTable->dwImmuneFlag = get_Item_Immune_Value(csvTable.AsStringByIndex(col++));
    itemTable->dwGold = atoi(csvTable.AsStringByIndex(col++));
    itemTable->dwShopBuyPrice = atoi(csvTable.AsStringByIndex(col++));
    itemTable->dwRefinedVnum = atoi(csvTable.AsStringByIndex(col++));
    itemTable->wRefineSet = atoi(csvTable.AsStringByIndex(col++));
    itemTable->bAlterToMagicItemPct = atoi(csvTable.AsStringByIndex(col++));

    int i;

    for (i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
    {
        itemTable->aLimits[i].bType = get_Item_LimitType_Value(csvTable.AsStringByIndex(col++));
        itemTable->aLimits[i].lValue = atoi(csvTable.AsStringByIndex(col++));
    }

    for (i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
    {
        itemTable->aApplies[i].bType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col++));
        itemTable->aApplies[i].lValue = atoi(csvTable.AsStringByIndex(col++));
    }

    for (i = 0; i < ITEM_VALUES_MAX_NUM; ++i)
    {
        itemTable->alValues[i] = atoi(csvTable.AsStringByIndex(col++));
    }

    itemTable->bSpecular = atoi(csvTable.AsStringByIndex(col++));
    itemTable->bGainSocketPct = atoi(csvTable.AsStringByIndex(col++));
    col++; //AddonType

    itemTable->bWeight = 0;

    return true;
}

static bool BuildItemTable()
{
    /* [KaptanYosun Dev Note]
        File formats:
            item_names.txt :
                VNUM<TAB>NAME

            item_proto.txt :
                Item prototype table (tab-separated, first row = header)

        Processing steps:
        [1] Load item_names.txt and build a local VNUM -> NAME lookup map.
        [2] Load item_proto.txt and validate file access.
        [3] Reserve capacity for temporary item vector.
        [4] Emplace and parse each item row into TClientItemTable.
        [5] Dump generated item data.
    */

    fprintf(stderr, "sizeof(TClientItemTable): %u\n", sizeof(TClientItemTable));

    /* - [1] ----------------------------------------------- */
    std::unordered_map<int, std::string_view> localMap;
    cCsvTable nameData;

    if (!nameData.Load("item_names.txt", '\t'))
    {
        fprintf(stderr, "Failed to load item_names.txt\n");
    }

    else
    {
        nameData.Next(); // skip header

        while (nameData.Next())
        {
            int vnum = 0;

            if (!str_to_number(vnum, nameData.AsStringByIndex(0)))
            {
                continue;
            }

            localMap.try_emplace(vnum, nameData.AsStringByIndex(1));
        }
    }

    /* ----------------------------------------------------- */


    /* - [2] ----------------------------------------------- */
    cCsvTable data;

    if (!data.Load("item_proto.txt", '\t'))
    {
        fprintf(stderr, "Failed to load item_proto.txt\n");
        return false;
    }

    data.Next(); // skip header
    /* ----------------------------------------------------- */


    /* - [3] Reserve target vector capacity -------------------------------------
       reserve() allocates raw memory only.
       No TClientItemTable objects are constructed at this stage.
       This prevents re-allocation during parsing.
    -------------------------------------------------------------------------- */
    if (m_pItemTable)
    {
        delete[] m_pItemTable;
        m_pItemTable = nullptr;
        m_iItemTableSize = 0;
    }

    const std::size_t rowCount = data.m_File.GetRowCount() - 1;

    std::vector<TClientItemTable> items;
    items.reserve(rowCount);
    /* ----------------------------------------------------- */


    /* - [4] Emplace and parse rows ---------------------------------------------
       Each TClientItemTable is constructed directly inside the vector.
       Set_Proto_Item_Table fills the object in-place.
       No temporary objects, no copy/move, no index-based assignment.
    -------------------------------------------------------------------------- */
    while (data.Next())
    {
        items.emplace_back();
        TClientItemTable& item = items.back();

        if (!Set_Proto_Item_Table(&item, data, localMap))
        {
            fprintf(stderr, "Failed to parse item_proto row.\n");
            items.pop_back();
            continue;
        }
    }

    /* - [5] ----------------------------------------------- */
    m_iItemTableSize = static_cast<int>(items.size());
    m_pItemTable = new TClientItemTable[m_iItemTableSize];
    std::memcpy(m_pItemTable, items.data(),
                sizeof(TClientItemTable) * m_iItemTableSize);

    for (const auto& item : items)
    {
        fprintf(stdout,
                "ITEM #%-5u %-24s VAL: %ld %ld %ld %ld %ld %ld "
                "WEAR %u ANTI %u IMMUNE %u REFINE %u\n",
                item.dwVnum,
                item.szName,
                item.alValues[0],
                item.alValues[1],
                item.alValues[2],
                item.alValues[3],
                item.alValues[4],
                item.alValues[5],
                item.dwWearFlags,
                item.dwAntiFlags,
                item.dwImmuneFlag,
                item.dwRefinedVnum);
    }

    /* ----------------------------------------------------- */

    return true;
}

DWORD g_adwItemProtoKey[4] =
{
    173217,
    72619434,
    408587239,
    27973291
};

void SaveItemProto()
{
    FILE* fp;

    fp = fopen("item_proto", "wb");

    if (!fp)
    {
        printf("cannot open %s for writing\n", "item_proto");
        return;
    }

    DWORD fourcc = MAKEFOURCC('M', 'I', 'P', 'X');
    fwrite(&fourcc, sizeof(DWORD), 1, fp);

    DWORD dwVersion = 0x00000001;
    fwrite(&dwVersion, sizeof(DWORD), 1, fp);

    DWORD dwStride = sizeof(TClientItemTable);
    fwrite(&dwStride, sizeof(DWORD), 1, fp);

    DWORD dwElements = m_iItemTableSize;
    fwrite(&dwElements, sizeof(DWORD), 1, fp);

    CLZObject zObj;
    std::vector <TClientItemTable> vec_item_table(&m_pItemTable[0], &m_pItemTable[m_iItemTableSize - 1]);
    sort(&m_pItemTable[0], &m_pItemTable[0] + m_iItemTableSize);

    if (!CLZO::instance().CompressEncryptedMemory(zObj, m_pItemTable, sizeof(TClientItemTable) * m_iItemTableSize, g_adwItemProtoKey))
    {
        printf("cannot compress\n");
        fclose(fp);
        return;
    }

    const CLZObject::THeader& r = zObj.GetHeader();

    printf("Elements %d\n%u --Compress--> %u --Encrypt--> %u, GetSize %u\n",
           m_iItemTableSize,
           r.dwRealSize,
           r.dwCompressedSize,
           r.dwEncryptSize,
           zObj.GetSize());

    DWORD dwDataSize = zObj.GetSize();
    fwrite(&dwDataSize, sizeof(DWORD), 1, fp);
    fwrite(zObj.GetBuffer(), dwDataSize, 1, fp);

    fclose(fp);

    fp = fopen("item_proto", "rb");

    if (!fp)
    {
        printf("Error!!\n");
        return;
    }

    fread(&fourcc, sizeof(DWORD), 1, fp);
    fread(&dwElements, sizeof(DWORD), 1, fp);

    printf("Elements Check %u fourcc match %d\n", dwElements, fourcc == MAKEFOURCC('M', 'I', 'P', 'T'));
    fclose(fp);
}


int main(int argc, char** argv)
{


    if (BuildMobTable())
    {
        SaveMobProto();
        LoadMobProto();
        cout << "BuildMobTable working normal" << endl;
    }


    if (BuildItemTable())
    {
        SaveItemProto();
        cout << "BuildItemTable working normal" << endl;
    }


    return 0;
}

