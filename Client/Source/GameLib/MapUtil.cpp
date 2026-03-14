#include "StdAfx.h"
#include "MapUtil.h"

void Environment_Init(SEnvironmentData& envData)
{
    envData.fWindStrength = 0.2f;
    envData.fWindRandom = 0.0f;

    envData.v3SkyBoxScale = D3DXVECTOR3(3500.0f, 3500.0f, 3500.0f);

    envData.v2CloudScale = D3DXVECTOR2(20000.0f, 20000.0f);
    envData.fCloudHeight = 3000.0f;
    envData.v2CloudTextureScale = D3DXVECTOR2(4.0f, 4.0f);
    envData.v2CloudSpeed = D3DXVECTOR2(0.001f, 0.001f);
    envData.strCloudTextureFileName = "";
    envData.cloudTintMultiplier = D3DXCOLOR{ 1.f, 1.f, 1.f, 1.f };

    envData.bReserve = FALSE;
}

bool Environment_Load(SEnvironmentData& envData, const char* envFileName)
{
    CTextFileLoader textLoader;

    if (!textLoader.Load(envFileName))
    {
        return false;
    }

    textLoader.SetTop();

    textLoader.GetTokenBoolean("reserved", &envData.bReserve);

    if (textLoader.SetChildNode("skybox"))
    {
        textLoader.GetTokenVector3("scale", &envData.v3SkyBoxScale);

        textLoader.GetTokenString("skytexture", &envData.strSkyBoxTextureFileName);

        textLoader.GetTokenVector2("cloudscale", &envData.v2CloudScale);
        textLoader.GetTokenFloat("cloudheight", &envData.fCloudHeight);
        textLoader.GetTokenVector2("cloudtexturescale", &envData.v2CloudTextureScale);
        textLoader.GetTokenVector2("cloudspeed", &envData.v2CloudSpeed);
        textLoader.GetTokenString("cloudtexturefilename", &envData.strCloudTextureFileName);
        textLoader.GetTokenColor("cloudcolor", &envData.cloudTintMultiplier);

        textLoader.SetParentNode();
    }

    return true;
}

void GetInterpolatedPosition(float curPositionRate, TPixelPosition * PixelPosition)
{
}

float GetLinearInterpolation(float begin, float end, float curRate)
{
    return (end - begin) * curRate + begin;
}

void PixelPositionToAttributeCellPosition(TPixelPosition PixelPosition, TCellPosition * pAttrCellPosition)
{
    pAttrCellPosition->x = PixelPosition.x / c_Section_xAttributeCellSize;
    pAttrCellPosition->y = PixelPosition.y / c_Section_yAttributeCellSize;
}

void AttributeCellPositionToPixelPosition(TCellPosition AttrCellPosition, TPixelPosition * pPixelPosition)
{
    pPixelPosition->x = AttrCellPosition.x * c_Section_xAttributeCellSize;
    pPixelPosition->y = AttrCellPosition.y * c_Section_yAttributeCellSize;
}

float GetPixelPositionDistance(const TPixelPosition & c_rsrcPosition, const TPixelPosition & c_rdstPosition)
{
    int idx = c_rsrcPosition.x - c_rdstPosition.x;
    int idy = c_rsrcPosition.y - c_rdstPosition.y;

    return sqrtf(float(idx * idx + idy * idy));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEaseOutInterpolation::CEaseOutInterpolation()
{
    Initialize();
}

CEaseOutInterpolation::~CEaseOutInterpolation()
{
}

void CEaseOutInterpolation::Initialize()
{
    m_fRemainingTime = 0.0f;
    m_fValue = 0.0f;
    m_fSpeed = 0.0f;
    m_fAcceleration = 0.0f;
    m_fStartValue = 0.0f;
    m_fLastValue = 0.0f;
}


BOOL CEaseOutInterpolation::Setup(float fStart, float fEnd, float fTime)
{
    //for safety
    if (fabs(fTime) < FLT_EPSILON)
    {
        fTime = 0.01f;
    }

    m_fValue = fStart;
    m_fStartValue = fStart;
    m_fLastValue = fStart;

    m_fSpeed = (2.0f * (fEnd - fStart)) / fTime;
    m_fAcceleration = 2.0f * (fEnd - fStart) / (fTime * fTime) - 2.0f * m_fSpeed / fTime;
    m_fRemainingTime = fTime;

    return TRUE;
}

void CEaseOutInterpolation::Interpolate(float fElapsedTime)
{
    m_fLastValue = m_fValue;

    m_fRemainingTime -= fElapsedTime;
    m_fSpeed += m_fAcceleration * fElapsedTime;
    m_fValue += m_fSpeed * fElapsedTime;

    if (!isPlaying())
    {
        m_fValue = 0.0f;
        m_fLastValue = 0.0f;
    }
}

BOOL CEaseOutInterpolation::isPlaying()
{
    return m_fRemainingTime > 0.0f;
}

float CEaseOutInterpolation::GetValue()
{
    return m_fValue;
}

float CEaseOutInterpolation::GetChangingValue()
{
    return m_fValue - m_fLastValue;
}
