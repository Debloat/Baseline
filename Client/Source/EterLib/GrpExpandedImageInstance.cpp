#include "StdAfx.h"
#include "../EterBase/CRC32.h"
#include "GrpExpandedImageInstance.h"
#include "StateManager.h"
#include <array>

#include "ShaderProvider.h"
#include "GrpDevice.h"

CDynamicPool<CGraphicExpandedImageInstance>		CGraphicExpandedImageInstance::ms_kPool;

void CGraphicExpandedImageInstance::CreateSystem(UINT uCapacity)
{
    ms_kPool.Create(uCapacity);
}

void CGraphicExpandedImageInstance::DestroySystem()
{
    ms_kPool.Destroy();
}

CGraphicExpandedImageInstance* CGraphicExpandedImageInstance::New()
{
    return ms_kPool.Alloc();
}

void CGraphicExpandedImageInstance::Delete(CGraphicExpandedImageInstance* pkImgInst)
{
    pkImgInst->Destroy();
    ms_kPool.Free(pkImgInst);
}

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 1> kImageSamplers =
    { {
        { 0, ESamplerState::LinearWrap }
    } };

    // Alpha
    constexpr PipelineStateDesc kImageAlphaPipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::Disabled,
        EBlendState::AlphaBlend,
        ERasterState::CullNone,
        kImageSamplers.data(),
        kImageSamplers.size()
    };

    constexpr PipelineStateDesc kImageAlphaCullFrontPipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::Disabled,
        EBlendState::AlphaBlend,
        ERasterState::CullFront,
        kImageSamplers.data(),
        kImageSamplers.size()
    };

    // Screen / Color Dodge
    constexpr PipelineStateDesc kImageScreenPipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::Disabled,
        EBlendState::One_InvSrcColor,
        ERasterState::CullNone,
        kImageSamplers.data(),
        kImageSamplers.size()
    };

    constexpr PipelineStateDesc kImageScreenCullFrontPipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::Disabled,
        EBlendState::One_InvSrcColor,
        ERasterState::CullFront,
        kImageSamplers.data(),
        kImageSamplers.size()
    };

    // Modulate (ZERO / SRCCOLOR)
    constexpr PipelineStateDesc kImageModulatePipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::Disabled,
        EBlendState::Zero_SrcColor,
        ERasterState::CullNone,
        kImageSamplers.data(),
        kImageSamplers.size()
    };

    constexpr PipelineStateDesc kImageModulateCullFrontPipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::Disabled,
        EBlendState::Zero_SrcColor,
        ERasterState::CullFront,
        kImageSamplers.data(),
        kImageSamplers.size()
    };
}

void CGraphicExpandedImageInstance::OnRender()
{
    CGraphicImage* pImage = m_roImage.GetPointer();
    CGraphicTexture* pTexture = pImage->GetTexturePointer();

    const RECT& c_rRect = pImage->GetRectReference();

    float texReverseWidth = 1.0f / float(pTexture->GetWidth());
    float texReverseHeight = 1.0f / float(pTexture->GetHeight());

    float su = (c_rRect.left - m_RenderingRect.left) * texReverseWidth;
    float sv = (c_rRect.top - m_RenderingRect.top) * texReverseHeight;
    float eu = (c_rRect.left + m_RenderingRect.right + (c_rRect.right - c_rRect.left)) * texReverseWidth;
    float ev = (c_rRect.top + m_RenderingRect.bottom + (c_rRect.bottom - c_rRect.top)) * texReverseHeight;

    std::array<TPDTVertex, 4> vertices{ {
        { { m_v2Position.x - 0.5f, m_v2Position.y - 0.5f, m_fDepth }, m_DiffuseColor, TTextureCoordinate(su, sv) },
        { { m_v2Position.x - 0.5f, m_v2Position.y - 0.5f, m_fDepth }, m_DiffuseColor, TTextureCoordinate(eu, sv) },
        { { m_v2Position.x - 0.5f, m_v2Position.y - 0.5f, m_fDepth }, m_DiffuseColor, TTextureCoordinate(su, ev) },
        { { m_v2Position.x - 0.5f, m_v2Position.y - 0.5f, m_fDepth }, m_DiffuseColor, TTextureCoordinate(eu, ev) }
    } };

    if (0.0f == m_fRotation)
    {
        float fimgWidth = float(pImage->GetWidth()) * m_v2Scale.x;
        float fimgHeight = float(pImage->GetHeight()) * m_v2Scale.y;

        vertices[0].position.x -= m_RenderingRect.left;
        vertices[0].position.y -= m_RenderingRect.top;

        vertices[1].position.x += fimgWidth + m_RenderingRect.right;
        vertices[1].position.y -= m_RenderingRect.top;

        vertices[2].position.x -= m_RenderingRect.left;
        vertices[2].position.y += fimgHeight + m_RenderingRect.bottom;

        vertices[3].position.x += fimgWidth + m_RenderingRect.right;
        vertices[3].position.y += fimgHeight + m_RenderingRect.bottom;
    }
    else
    {
        float fimgHalfWidth = float(pImage->GetWidth()) / 2.0f * m_v2Scale.x;
        float fimgHalfHeight = float(pImage->GetHeight()) / 2.0f * m_v2Scale.y;

        for (int i = 0; i < 4; ++i)
        {
            vertices[i].position.x += m_v2Origin.x;
            vertices[i].position.y += m_v2Origin.y;
        }

        float fRadian = D3DXToRadian(m_fRotation);

        vertices[0].position.x += (-fimgHalfWidth * cosf(fRadian)) - (-fimgHalfHeight * sinf(fRadian));
        vertices[0].position.y += (-fimgHalfWidth * sinf(fRadian)) + (-fimgHalfHeight * cosf(fRadian));

        vertices[1].position.x += (+fimgHalfWidth * cosf(fRadian)) - (-fimgHalfHeight * sinf(fRadian));
        vertices[1].position.y += (+fimgHalfWidth * sinf(fRadian)) + (-fimgHalfHeight * cosf(fRadian));

        vertices[2].position.x += (-fimgHalfWidth * cosf(fRadian)) - (+fimgHalfHeight * sinf(fRadian));
        vertices[2].position.y += (-fimgHalfWidth * sinf(fRadian)) + (+fimgHalfHeight * cosf(fRadian));

        vertices[3].position.x += (+fimgHalfWidth * cosf(fRadian)) - (+fimgHalfHeight * sinf(fRadian));
        vertices[3].position.y += (+fimgHalfWidth * sinf(fRadian)) + (+fimgHalfHeight * cosf(fRadian));
    }

    const bool flipCull =
        (0.0f < m_v2Scale.x && 0.0f > m_v2Scale.y) ||
        (0.0f > m_v2Scale.x && 0.0f < m_v2Scale.y);

    const PipelineStateDesc* pipeline = &kImageAlphaPipeline;

    switch (m_iRenderingMode)
    {
    case RENDERING_MODE_SCREEN:
    case RENDERING_MODE_COLOR_DODGE:
        pipeline = flipCull ?
            &kImageScreenCullFrontPipeline :
            &kImageScreenPipeline;
        break;

    case RENDERING_MODE_MODULATE:
        pipeline = flipCull ?
            &kImageModulateCullFrontPipeline :
            &kImageModulatePipeline;
        break;

    default:
        pipeline = flipCull ?
            &kImageAlphaCullFrontPipeline :
            &kImageAlphaPipeline;
        break;
    }

    if (CGraphicBase::SetPDTStream(vertices.data(), 4))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(*pipeline))
            return;

        ScreenPrimitiveShaderInputs in{};

        const D3DVIEWPORT9& vp = CGraphicBase::GetViewport();

        sp->FillScreenPrimitive2DOrthoPixel(
            static_cast<float>(vp.Width),
            static_cast<float>(vp.Height),
            in
        );

        in.ps.mode[0] = 1.0f;
        in.ps.colorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

        STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
        STATEMANAGER.SetTexture(1, nullptr);

        STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
    }
}

void CGraphicExpandedImageInstance::SetDepth(float fDepth)
{
    m_fDepth = fDepth;
}

void CGraphicExpandedImageInstance::SetOrigin()
{
    SetOrigin(float(GetWidth()) / 2.0f, float(GetHeight()) / 2.0f);
}

void CGraphicExpandedImageInstance::SetOrigin(float fx, float fy)
{
    m_v2Origin.x = fx;
    m_v2Origin.y = fy;
}

void CGraphicExpandedImageInstance::SetRotation(float fRotation)
{
    m_fRotation = fRotation;
}

void CGraphicExpandedImageInstance::SetScale(float fx, float fy)
{
    m_v2Scale.x = fx;
    m_v2Scale.y = fy;
}

void CGraphicExpandedImageInstance::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
    if (IsEmpty())
    {
        return;
    }

    float fWidth = float(GetWidth());
    float fHeight = float(GetHeight());

    m_RenderingRect.left = fWidth * fLeft;
    m_RenderingRect.top = fHeight * fTop;
    m_RenderingRect.right = fWidth * fRight;
    m_RenderingRect.bottom = fHeight * fBottom;
}

void CGraphicExpandedImageInstance::SetRenderingMode(int iMode)
{
    m_iRenderingMode = iMode;
}

DWORD CGraphicExpandedImageInstance::Type()
{
    static DWORD s_dwType = GetCRC32("CGraphicExpandedImageInstance", strlen("CGraphicExpandedImageInstance"));
    return s_dwType;
}

void CGraphicExpandedImageInstance::OnSetImagePointer()
{
    if (IsEmpty())
    {
        return;
    }

    SetOrigin(float(GetWidth()) / 2.0f, float(GetHeight()) / 2.0f);
}

BOOL CGraphicExpandedImageInstance::OnIsType(DWORD dwType)
{
    if (CGraphicExpandedImageInstance::Type() == dwType)
    {
        return TRUE;
    }

    return CGraphicImageInstance::IsType(dwType);
}

void CGraphicExpandedImageInstance::Initialize()
{
    m_iRenderingMode = RENDERING_MODE_NORMAL;
    m_fDepth = 0.0f;
    m_v2Origin.x = m_v2Origin.y = 0.0f;
    m_v2Scale.x = m_v2Scale.y = 1.0f;
    m_fRotation = 0.0f;
    memset(&m_RenderingRect, 0, sizeof(RECT));
}

void CGraphicExpandedImageInstance::Destroy()
{
    CGraphicImageInstance::Destroy();
    Initialize();
}

CGraphicExpandedImageInstance::CGraphicExpandedImageInstance()
{
    Initialize();
}

CGraphicExpandedImageInstance::~CGraphicExpandedImageInstance()
{
    Destroy();
}
