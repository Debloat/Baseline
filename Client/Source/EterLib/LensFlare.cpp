///////////////////////////////////////////////////////////////////////
//	Preprocessor
#include "StdAfx.h"
#include "LensFlare.h"
#include "Camera.h"
#include "StateManager.h"
#include "ResourceManager.h"

#include <math.h>

#include "ShaderProvider.h"
#include "GrpDevice.h"

using namespace std;

static string g_strFiles[] =
{
    "flare2.dds",
    "flare1.dds",
    "flare2.dds",
    "flare1.dds",
    "flare6.dds",
    "flare4.dds",
    "flare2.dds",
    "flare3.dds",
    ""
};
static float g_fPosition[] =
{
    -0.55f,
    -0.5f,
    -0.45f,
    0.2f,
    0.3f,
    0.95f,
    0.9f,
    1.0f
};
static float g_fWidth[] =
{
    20.0f,
    32.0f,
    20.0f,
    32.0f,
    100.0f,
    32.0f,
    20.0f,
    250.0f
};

static float g_afColors[][4] =
{
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 0.8f },
    { 0.3f, 0.5f, 1.0f, 0.9f },
    { 0.3f, 0.5f, 1.0f, 0.6f },
    { 1.0f, 0.6f, 0.9f, 0.4f },
    { 1.0f, 0.0f, 0.0f, 0.5f },
    { 1.0f, 0.6f, 0.3f, 0.4f }
};

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 1> LensFlareSamplers =
    { {
        { 0, ESamplerState::LinearClamp }
    } };

    // Main sun sprite: alpha blend
    constexpr PipelineStateDesc LensFlareSunPipeline =
    {
        ShaderID::LensFlare,
        EDepthState::Disabled,
        EBlendState::AlphaBlend,
        ERasterState::CullNone,
        LensFlareSamplers.data(),
        LensFlareSamplers.size()
    };

    // Flare pieces: SRCALPHA / ONE (alpha-scaled additive)
    constexpr PipelineStateDesc LensFlarePiecePipeline =
    {
        ShaderID::LensFlare,
        EDepthState::Disabled,
        EBlendState::AlphaAdditive,
        ERasterState::CullNone,
        LensFlareSamplers.data(),
        LensFlareSamplers.size()
    };
}

CLensFlare::CLensFlare() :
    m_fBeforeBright(0.0f),
    m_fAfterBright(0.0f),
    m_bFlareVisible(false),
    m_bDrawFlare(true),
    m_bDrawBrightScreen(true),
    m_fSunSize(0),
    m_bEnabled(true),
    m_bShowMainFlare(true),
    m_fMaxBrightness(1.0f)
{
    m_pControlPixels = new float[c_nDepthTestDimension * c_nDepthTestDimension];
    m_pTestPixels = new float[c_nDepthTestDimension * c_nDepthTestDimension];
    m_afColor[0] = m_afColor[1] = m_afColor[2] = 1.0f;
}

CLensFlare::~CLensFlare()
{
    delete[] m_pControlPixels;
    delete[] m_pTestPixels;
}

float CLensFlare::Interpolate(float fStart, float fEnd, float fPercent)
{
    return fStart + (fEnd - fStart) * fPercent;
}

void CLensFlare::Compute(const D3DXVECTOR3 & c_rv3LightDirection)
{

    D3DXVECTOR3 v3Target = CCameraManager::Instance().GetCurrentCamera()->GetTarget();

    constexpr float kSunDistance = 99999999.0f;
    std::array<float, 3> afSunPos{
        v3Target.x - c_rv3LightDirection.x * kSunDistance,
        v3Target.y - c_rv3LightDirection.y * kSunDistance,
        v3Target.z - c_rv3LightDirection.z * kSunDistance
    };

    float fX, fY;
    ProjectPosition(afSunPos[0], afSunPos[1], afSunPos[2], &fX, &fY);

    // set flare location
    SetFlareLocation(fX, fY);

    // determine visibility
    float fSunVectorMagnitude = sqrtf(afSunPos[0] * afSunPos[0] +
                                      afSunPos[1] * afSunPos[1] +
                                      afSunPos[2] * afSunPos[2]);

    const float invMagnitude = 1.0f / fSunVectorMagnitude;
    std::array<float, 3> afSunVector{
        -afSunPos[0] * invMagnitude,
        -afSunPos[1] * invMagnitude,
        -afSunPos[2] * invMagnitude
    };

    std::array<float, 3> afCameraDirection{
        ms_matView._13,
        ms_matView._23,
        ms_matView._33
    };

    if (float fDotProduct =
        (afSunVector[0] * afCameraDirection[0]) +
        (afSunVector[1] * afCameraDirection[1]) +
        (afSunVector[2] * afCameraDirection[2]); acosf(fDotProduct) < 0.5f * D3DX_PI)
    {
        SetVisible(true);
    }

    else
    {
        SetVisible(false);
    }

    // set flare brightness
    fX /= ms_Viewport.Width;
    fY /= ms_Viewport.Height;

    float fDistance = sqrtf(((0.5f - fX) * (0.5f - fX)) + ((0.5f - fY) * (0.5f - fY)));
    float fBeforeBright = Interpolate(0.0f, c_fHalfMaxBright, 1.0f - (fDistance * c_fDistanceScale));
    float fAfterBright = Interpolate(0.0f, 1.0f, 1.0f - (fDistance * c_fDistanceScale));

    SetBrightnesses(fBeforeBright, fAfterBright);
}

void CLensFlare::DrawBeforeFlare()
{
    if (!m_bFlareVisible || !m_bEnabled || !m_bShowMainFlare)
    {
        return;
    }

    if (m_SunFlareImageInstance.IsEmpty())
    {
        return;
    }

    float fAspectRatio = static_cast<float>(ms_Viewport.Width) / static_cast<float>(ms_Viewport.Height);
    float fHeight = m_fSunSize * fAspectRatio;
    D3DXCOLOR color(1.0f, 1.0f, 1.0f, 1.0f);

    std::array<SVertex, 4> vertices{ {
        { -m_fSunSize, -fHeight, 0.0f, color, 0.0f, 0.0f },
        { -m_fSunSize,  fHeight, 0.0f, color, 0.0f, 1.0f },
        {  m_fSunSize, -fHeight, 0.0f, color, 1.0f, 0.0f },
        {  m_fSunSize,  fHeight, 0.0f, color, 1.0f, 1.0f },
    } };

    /* - SHADER [LENSFLARE] ----------------------------------- */
    IShaderProvider const* sp = GetShaderProvider();
    if (!sp || !sp->BindPipelineState(LensFlareSunPipeline))
    {
        TraceError("LensFlare pipeline bind failed");
        return;
    }
    /* ----------------------------------------------------- */

    LensFlareShaderInputs in{};

    ScreenPrimitiveShaderInputs spIn{};

    D3DXMATRIX world;
    D3DXMatrixTranslation(&world, m_afFlarePos[0], m_afFlarePos[1], 0.0f);

    sp->FillScreenPrimitive2DOrtho01World(world, spIn);

    in.vs.worldViewProj = spIn.vs.worldViewProj;

    in.ps.brightnessColor = {
        m_afColor[0],
        m_afColor[1],
        m_afColor[2],
        m_fMaxBrightness
    };

    CGraphicDevice::UploadLensFlareConstants(in);

    STATEMANAGER.SetTexture(0, m_SunFlareImageInstance.GetTexturePointer()->GetD3DTexture());
    STATEMANAGER.SetTexture(1, nullptr);

    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices.data(), sizeof(SVertex));
}

void CLensFlare::DrawAfterFlare()
{
    if (m_bEnabled && m_fAfterBright != 0.0f && m_bDrawBrightScreen)
    {
        SetDiffuseColor(m_afColor[0], m_afColor[1], m_afColor[2], m_fAfterBright);
        RenderBar2d(0.0f, 0.0f, static_cast<float>(ms_Viewport.Width), static_cast<float>(ms_Viewport.Height));
    }
}

void CLensFlare::SetMainFlare(string strSunFile, float fSunSize)
{
    if (m_bEnabled && m_bShowMainFlare)
    {
        m_fSunSize = fSunSize;
        CResource * pResource = CResourceManager::Instance().GetResourcePointer(strSunFile.c_str());

        if (!pResource->IsType(CGraphicImage::Type()))
        {
            assert(false);
        }

        m_SunFlareImageInstance.SetImagePointer(static_cast<CGraphicImage*> (pResource));
    }
}

void CLensFlare::DrawFlare()
{
    if (m_bEnabled && m_bFlareVisible && m_bDrawFlare && m_fAfterBright != 0.0f)
    {
        // Bright screen overlay uses RenderBar2d -> ScreenPrimitive pipeline (handled there)
        DrawAfterFlare();

        // Flare pieces: bind explicit lens flare pipeline inside CFlare::Draw
        m_cFlare.Draw(m_fAfterBright,
            ms_Viewport.Width,
            ms_Viewport.Height,
            static_cast<int>(m_afFlareWinPos[0]),
            static_cast<int>(m_afFlareWinPos[1]));
    }
}

void CLensFlare::CharacterizeFlare(bool bEnabled, bool bShowMainFlare, float fMaxBrightness, const D3DXCOLOR & c_rColor)
{
    m_bEnabled = bEnabled;
    m_bShowMainFlare = bShowMainFlare;
    m_fMaxBrightness = fMaxBrightness;

    m_afColor[0] = c_rColor.r;
    m_afColor[1] = c_rColor.g;
    m_afColor[2] = c_rColor.b;
}

void CLensFlare::Initialize(std::string strPath)
{
    if (m_bEnabled)
    {
        m_cFlare.Init(strPath);
    }
}

void CLensFlare::SetFlareLocation(double dX, double dY)
{
    if (m_bEnabled)
    {
        m_afFlareWinPos[0] = float(dX);
        m_afFlareWinPos[1] = float(dY);

        m_afFlarePos[0] = float(dX) / static_cast<float>(ms_Viewport.Width);
        m_afFlarePos[1] = float(dY) / static_cast<float>(ms_Viewport.Height);
    }
}

void CLensFlare::SetBrightnesses(float fBeforeBright, float fAfterBright)
{
    if (m_bEnabled)
    {
        m_fBeforeBright = fBeforeBright;
        m_fAfterBright = fAfterBright;

        ClampBrightness();
    }
}

void CLensFlare::ReadControlPixels()
{
    if (m_bEnabled)
    {
        ReadDepthPixels(m_pControlPixels);
    }
}

void CLensFlare::AdjustBrightness()
{
    if (m_bEnabled)
    {
        ReadDepthPixels(m_pTestPixels);

        int nDifferent = 0;

        for (int i = 0; i < c_nDepthTestDimension * c_nDepthTestDimension; ++i)
            if (m_pTestPixels[i] != m_pControlPixels[i])
            {
                ++nDifferent;
            }

        float fAdjust = (static_cast<float>(nDifferent) / (c_nDepthTestDimension * c_nDepthTestDimension));
        fAdjust = sqrtf(fAdjust) * 0.85f;
        m_fAfterBright *= 1.0f - fAdjust;
    }
}

void CLensFlare::ReadDepthPixels(float* /*pPixels*/)
{
    LPDIRECT3DSURFACE9 lpSurface;
    if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&lpSurface)))
    	assert(false);

    D3DLOCKED_RECT rect;
    lpSurface->LockRect(&rect, NULL, D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE);

    lpSurface->UnlockRect();
}

void CLensFlare::ClampBrightness()
{
    // before
    if (m_fBeforeBright < 0.0f)
    {
        m_fBeforeBright = 0.0f;
    }

    else if (m_fBeforeBright > 1.0f)
    {
        m_fBeforeBright = 1.0f;
    }

    m_fBeforeBright *= m_fMaxBrightness;

    if (m_fAfterBright < 0.0f)
    {
        m_fAfterBright = 0.0f;
    }

    else if (m_fAfterBright > 1.0f)
    {
        m_fAfterBright = 1.0f;
    }

    m_fAfterBright *= m_fMaxBrightness;
}

CFlare::CFlare()
{
}

CFlare::~CFlare()
{
}

void CFlare::Init(std::string strPath)
{
    m_vFlares.clear();

    int i = 0;

    while (g_strFiles[i] != "")
    {
        CResource * pResource = CResourceManager::Instance().GetResourcePointer((strPath + "/" + string(g_strFiles[i])).c_str());

        if (!pResource->IsType(CGraphicImage::Type()))
        {
            assert(false);
        }

        SFlarePiece * pPiece = new SFlarePiece;

        pPiece->m_imageInstance.SetImagePointer(static_cast<CGraphicImage*> (pResource));
        pPiece->m_fPosition = g_fPosition[i];
        pPiece->m_fWidth = g_fWidth[i];
        pPiece->m_pColor = g_afColors[i];

        m_vFlares.push_back(pPiece);
        i++;
    }
}

void CFlare::Draw(float fBrightScale, int nWidth, int nHeight, int nX, int nY)
{

    IShaderProvider const* sp = GetShaderProvider();
    if (!sp || !sp->BindPipelineState(LensFlarePiecePipeline))
    {
        return;
    }

    LensFlareShaderInputs in{};

    ScreenPrimitiveShaderInputs spIn{};
    sp->FillScreenPrimitive2DOrthoPixel(static_cast<float>(nWidth), static_cast<float>(nHeight), spIn);

    in.vs.worldViewProj = spIn.vs.worldViewProj;

    in.ps.brightnessColor = {
        1.0f,              // no extra tint here
        1.0f,
        1.0f,
        fBrightScale       // flare intensity already computed
    };

    CGraphicDevice::UploadLensFlareConstants(in);

    float fDX = float(nX) - float(nWidth) / 2.0f;
    float fDY = float(nY) - float(nHeight) / 2.0f;

    for (unsigned int i = 0; i < m_vFlares.size(); i++)
    {
        float fCenterX = float(nX) - (m_vFlares[i]->m_fPosition + 1.0f) * fDX;
        float fCenterY = float(nY) - (m_vFlares[i]->m_fPosition + 1.0f) * fDY;
        float fW = m_vFlares[i]->m_fWidth;

        D3DXCOLOR d3dColor(m_vFlares[i]->m_pColor[0] * fBrightScale,
                           m_vFlares[i]->m_pColor[1] * fBrightScale,
                           m_vFlares[i]->m_pColor[2] * fBrightScale,
                           m_vFlares[i]->m_pColor[3] * fBrightScale);

        STATEMANAGER.SetTexture(0, m_vFlares[i]->m_imageInstance.GetTexturePointer()->GetD3DTexture());

        std::array<TVertex, 4> vertices =
        { {
            {0.0f, 0.0f, fCenterX - fW, fCenterY - fW, 0.0f, d3dColor},
            {0.0f, 1.0f, fCenterX - fW, fCenterY + fW, 0.0f, d3dColor},
            {1.0f, 0.0f, fCenterX + fW, fCenterY - fW, 0.0f, d3dColor},
            {1.0f, 1.0f, fCenterX + fW, fCenterY + fW, 0.0f, d3dColor},
        } };

        STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices.data(), sizeof(TVertex));
    }
}
