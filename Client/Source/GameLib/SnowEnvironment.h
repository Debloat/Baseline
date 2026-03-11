#pragma once

#include "../EterLib/GrpScreen.h"

class CSnowParticle;

class CSnowEnvironment : public CScreen
{
    public:
        CSnowEnvironment();
        virtual ~CSnowEnvironment();

        bool Create();
        void Destroy();

        void Enable();
        void Disable();

        void Update(const D3DXVECTOR3 & c_rv3Pos);
        void Deform();
        void Render();

    protected:
        void __Initialize();
        bool __CreateGeometry();

    protected:
        LPDIRECT3DVERTEXBUFFER9 m_pVB;
        LPDIRECT3DINDEXBUFFER9 m_pIB;

        D3DXVECTOR3 m_v3Center;

        CGraphicImageInstance* m_pImageInstance;
        std::vector<CSnowParticle*> m_kVct_pkParticleSnow;

        DWORD m_dwParticleMaxNum;

        BOOL m_bSnowEnable;
};
