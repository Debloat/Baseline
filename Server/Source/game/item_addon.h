#pragma once

class CItemAddonManager : public singleton<CItemAddonManager>
{
    public:
        CItemAddonManager();
        virtual ~CItemAddonManager();

        void ApplyAddonTo(int iAddonType, LPITEM pItem);
};
