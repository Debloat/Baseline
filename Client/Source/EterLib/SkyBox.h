// SkyBox.h: interface for the CSkyBox class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "GrpBase.h"
#include "GrpScreen.h"
#include "GrpImageInstance.h"

#include <map>
#include <string>

/* - SHADER -------------------------------------------- */
#include "ShaderParameters.h"
/* ----------------------------------------------------- */

using TColor = struct SColor
{
    SColor(float _r = 0.0f, float _g = 0.0f, float _b = 0.0f, float _a = 0.0f) : r(_r), g(_g), b(_b), a(_a) {}

    float r, g, b, a;
};

using TCloudTint = D3DXCOLOR;

class CSkyObjectQuad
{
    public:
        CSkyObjectQuad();
        virtual ~CSkyObjectQuad();

        void SetVertex(const unsigned char& c_rucNumVertex, const TPDTVertex & c_rPDTVertex);

        void Render();

    private:
        TPDTVertex m_Vertex[4];
        TIndex m_Indices[4];					// 인덱스 버퍼...
};

class CSkyObject : public CScreen
{
    public:
        CSkyObject();
        virtual ~CSkyObject();

        virtual void Destroy() = 0;

        virtual void Render() = 0;
        virtual void Update() = 0;

    protected:
        CGraphicImageInstance* GenerateTexture(const char* szfilename);
        void DeleteTexture(CGraphicImageInstance * pGraphicImageInstance);

    protected:
        //////////////////////////////////////////////////////////////////////////
        // 타입 정의

        using TSkyObjectQuadVector = std::vector<CSkyObjectQuad>;
        using TSkyObjectQuadIterator = TSkyObjectQuadVector::iterator;

        using TSkyObjectFace = struct CSkyBox
        {
            void Render();
            std::string m_strfacename;
            std::string m_strFaceTextureFileName;
            TSkyObjectQuadVector m_SkyObjectQuadVector;
        };

        using TGraphicImageInstanceMap = std::map<std::string, CGraphicImageInstance*>;

        //////////////////////////////////////////////////////////////////////////

        // 구름...
        TSkyObjectFace m_FaceCloud;		// 구름 일단 한장...
        D3DXMATRIX m_matWorldCloud;
        D3DXMATRIX m_matTextureCloud;
        float m_fCloudScaleX;
        float m_fCloudScaleY;
        float m_fCloudHeight;
        float m_fCloudTextureScaleX;
        float m_fCloudTextureScaleY;
        float m_fCloudScrollSpeedU;
        float m_fCloudScrollSpeedV;

        D3DXCOLOR m_cloudTintMultiplier;

        // 스카이 박스 이미지...
        TGraphicImageInstanceMap m_GraphicImageInstanceMap;

        // Transform...
        D3DXMATRIX m_matWorld;
        D3DXMATRIX m_matTranslation;
        D3DXVECTOR3 m_v3Position;
        float m_fScaleX;
        float m_fScaleY;
        float m_fScaleZ;

        std::string m_strCurTime;
        bool m_bSkyMatrixUpdated;
};

class CSkyBox : public CSkyObject
{
    public:
        CSkyBox();
        virtual ~CSkyBox();

        void Render();
        void RenderCloud();

        void Update() override;
        void Destroy();
        void Unload();

        void SetSkyBoxScale(const D3DXVECTOR3 & c_rv3Scale);
        void SetTexture(const char* c_szFileName);
        void SetCloudTexture(const char* c_szFileName);
        void SetCloudScale(const D3DXVECTOR2 & c_rv2CloudScale);
        void SetCloudHeight(float fHeight);
        void SetCloudTextureScale(const D3DXVECTOR2 & c_rv2CloudTextureScale);
        void SetCloudScrollSpeed(const D3DXVECTOR2 & c_rv2CloudScrollSpeed);
        void SetCloudTintMultiplier(const D3DXCOLOR& c_rvCloudTintMultiplier);
        void Refresh();

        CGraphicImageInstance* GetSkyTextureInstance() const;

    protected:
        void SetSkyObjectQuadHorizon(
            TSkyObjectQuadVector* pSkyObjectQuadVector,
            const D3DXVECTOR3* c_pv3QuadPoints,
            float uMin, float uMax,
            float vMin, float vMax);

    protected:
        TSkyObjectFace m_Faces[6];
        std::string m_strSkyTextureFileName;
};
