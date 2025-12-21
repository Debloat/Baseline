#pragma once

#include <map>

class CNetworkPacketHeaderMap
{
    public:
        using TPacketType = struct SPacketType
        {
            SPacketType(int iSize = 0, bool bFlag = false)
            {
                iPacketSize = iSize;
                isDynamicSizePacket = bFlag;
            }

            int iPacketSize;
            bool isDynamicSizePacket;
        };

    public:
        CNetworkPacketHeaderMap();
        virtual ~CNetworkPacketHeaderMap();

        void Set(int header, TPacketType rPacketType);
        bool Get(int header, TPacketType * pPacketType);

    protected:
        std::map<int, TPacketType> m_headerMap;
};
