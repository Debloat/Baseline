#pragma once

#include "Resource.h"
#include "Ref.h"
#include "CollisionData.h"

using THeightData = struct SHeightData
{
    char szName[32 + 1];
    std::vector<D3DXVECTOR3> v3VertexVector;
};

using THeightDataVector = std::vector<THeightData>;

class CAttributeData : public CResource
{
    public:
        using TRef = CRef<CAttributeData>;

    public:
        static TType Type();

    public:
        CAttributeData(const char* c_szFileName);
        virtual ~CAttributeData();

        //DWORD GetCollisionDataCount() const;
        //BOOL GetCollisionDataPointer(DWORD dwIndex, const TCollisionData ** c_ppCollisionData) const;
        const CStaticCollisionDataVector& GetCollisionDataVector() const;
        const THeightDataVector& GetHeightDataVector() const;

        size_t AddCollisionData(const CStaticCollisionData& collisionData);	// return m_StaticCollisionDataVector.size();

        DWORD GetHeightDataCount() const;
        BOOL GetHeightDataPointer(DWORD dwIndex, const THeightData** c_ppHeightData) const;

        float GetMaximizeRadius();

    protected:
        bool OnLoad(int iSize, const void* c_pvBuf);
        void OnClear();
        bool OnIsEmpty() const;
        bool OnIsType(TType type);
        void OnSelfDestruct();

    protected:
        float m_fMaximizeRadius;

        //std::vector<TCollisionData> m_CollisionDataVector;
        CStaticCollisionDataVector m_StaticCollisionDataVector;
        THeightDataVector m_HeightDataVector;
};
