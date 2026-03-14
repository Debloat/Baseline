#include "StdAfx.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/JpegFile.h"
#include "PythonGraphic.h"

/* - YOSUN_CONTROL_CENTER ------------------------------ */
#include "../SphereLib/YosunControlCenter.h"
/* ----------------------------------------------------- */
#include <array>

void CPythonGraphic::Destroy()
{
}

LPDIRECT3D9EX CPythonGraphic::GetD3D()
{
    return ms_lpd3d;
}

float CPythonGraphic::GetOrthoDepth()
{
    return m_fOrthoDepth;
}

void CPythonGraphic::SetInterfaceRenderState()
{
    STATEMANAGER.SetTransform(D3DTS_PROJECTION, &ms_matIdentity);
    STATEMANAGER.SetTransform(D3DTS_VIEW, &ms_matIdentity);
    STATEMANAGER.SetTransform(D3DTS_WORLD, &ms_matIdentity);

    CPythonGraphic::Instance().SetOrtho2D(ms_iWidth, ms_iHeight, GetOrthoDepth());
}

void CPythonGraphic::SetCursorPosition(int x, int y)
{
    CScreen::SetCursorPosition(x, y, ms_iWidth, ms_iHeight);
}

void CPythonGraphic::SetViewport(float fx, float fy, float fWidth, float fHeight)
{
    ms_lpd3dDevice->GetViewport(&m_backupViewport);

    D3DVIEWPORT9 ViewPort;
    ViewPort.X = fx;
    ViewPort.Y = fy;
    ViewPort.Width = fWidth;
    ViewPort.Height = fHeight;
    ViewPort.MinZ = 0.0f;
    ViewPort.MaxZ = 1.0f;

    if (FAILED(
                ms_lpd3dDevice->SetViewport(&ViewPort)
            ))
    {
        Tracef("CPythonGraphic::SetViewport(%d, %d, %d, %d) - Error",
               ViewPort.X, ViewPort.Y,
               ViewPort.Width, ViewPort.Height
              );
    }
}

void CPythonGraphic::RestoreViewport()
{
    ms_lpd3dDevice->SetViewport(&m_backupViewport);
}

void CPythonGraphic::SetGamma(float fGammaFactor)
{
    D3DCAPS9		d3dCaps;
    D3DGAMMARAMP	NewRamp;
    int				ui, val;

    ms_lpd3dDevice->GetDeviceCaps(&d3dCaps);

    if (D3DCAPS2_FULLSCREENGAMMA != (d3dCaps.Caps2 & D3DCAPS2_FULLSCREENGAMMA))
    {
        return;
    }

    for (int i = 0; i < 256; ++i)
    {
        val	= (int)(i * fGammaFactor * 255.0f);
        ui = 0;

        if (val > 32767)
        {
            val = val - 32767;
            ui = 1;
        }

        if (val > 32767)
        {
            val = 32767;
        }

        NewRamp.red[i] = (WORD)(val | (32768 * ui));
        NewRamp.green[i] = (WORD)(val | (32768 * ui));
        NewRamp.blue[i] = (WORD)(val | (32768 * ui));
    }

    ms_lpd3dDevice->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &NewRamp);
}

void GenScreenShotTag(const char* src, DWORD crc32, char* leaf, size_t leafLen)
{
    const char* p = src;
    const char* n = p;

    while (n = strchr(p, '\\'))
    {
        p = n + 1;
    }

    _snprintf(leaf, leafLen, "YMIR_METIN2:%s:0x%.8x", p, crc32);
}

bool CPythonGraphic::SaveJPEG(const char* pszFileName, LPBYTE pbyBuffer, UINT uWidth, UINT uHeight)
{
    return jpeg_save(pbyBuffer, uWidth, uHeight, 100, pszFileName) != 0;
}

bool CPythonGraphic::SaveScreenShot(const char* c_pszFileName)
{
    HRESULT hr;
    LPDIRECT3DSURFACE9 lpSurface;
    D3DSURFACE_DESC stSurfaceDesc;

    if (FAILED(hr = ms_lpd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &lpSurface)))
    {
        TraceError("Failed to get back buffer (0x%08x)", hr);
        return false;
    }

    if (FAILED(hr = lpSurface->GetDesc(&stSurfaceDesc)))
    {
        TraceError("Failed to get surface desc (0x%08x)", hr);
        SAFE_RELEASE(lpSurface);
        return false;
    }

    UINT uWidth = stSurfaceDesc.Width;
    UINT uHeight = stSurfaceDesc.Height;

    switch (stSurfaceDesc.Format)
    {
        case D3DFMT_R8G8B8 :
        case D3DFMT_A8R8G8B8 :
        case D3DFMT_X8R8G8B8 :
        case D3DFMT_R5G6B5 :
        case D3DFMT_X1R5G5B5 :
        case D3DFMT_A1R5G5B5 :
            break;

        case D3DFMT_A4R4G4B4 :
        case D3DFMT_R3G3B2 :
        case D3DFMT_A8R3G3B2 :
        case D3DFMT_X4R4G4B4 :
        case D3DFMT_A2B10G10R10 :
            TraceError("Unsupported BackBuffer Format(%d). Please contact Metin 2 Administrator.", stSurfaceDesc.Format);
            SAFE_RELEASE(lpSurface);
            return false;
    }

    D3DLOCKED_RECT lockRect;

    if (FAILED(hr = lpSurface->LockRect(&lockRect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK)))
    {
        TraceError("Failed to lock the surface (0x%08x)", hr);
        SAFE_RELEASE(lpSurface);
        return false;
    }

    BYTE* pbyBuffer = new BYTE[uWidth * uHeight * 3];

    if (pbyBuffer == NULL)
    {
        lpSurface->UnlockRect();
        lpSurface->Release();
        lpSurface = NULL;
        TraceError("Failed to allocate screenshot buffer");
        return false;
    }

    BYTE* pbySource = (BYTE*) lockRect.pBits;
    BYTE* pbyDestination = (BYTE*) pbyBuffer;

    for (UINT y = 0; y < uHeight; ++y)
    {
        BYTE *pRow = pbySource;

        switch (stSurfaceDesc.Format)
        {
            case D3DFMT_R8G8B8 :
                for (UINT x = 0; x < uWidth; ++x)
                {
                    *pbyDestination++ = pRow[2];	// Blue
                    *pbyDestination++ = pRow[1];	// Green
                    *pbyDestination++ = pRow[0];	// Red
                    pRow += 3;
                }

                break;

            case D3DFMT_A8R8G8B8 :
            case D3DFMT_X8R8G8B8 :
                for (UINT x = 0; x < uWidth; ++x)
                {
                    *pbyDestination++ = pRow[2];	// Blue
                    *pbyDestination++ = pRow[1];	// Green
                    *pbyDestination++ = pRow[0];	// Red
                    pRow += 4;
                }

                break;

            case D3DFMT_R5G6B5 :
            {
                for (UINT x = 0; x < uWidth; ++x)
                {
                    UINT uColor		= *((UINT*) pRow);
                    BYTE byBlue		= (uColor >> 11) & 0x1F;
                    BYTE byGreen	= (uColor >> 5) & 0x3F;
                    BYTE byRed		= uColor & 0x1F;

                    *pbyDestination++ = (byBlue << 3)	| (byBlue >> 2);		// Blue
                    *pbyDestination++ = (byGreen << 2)	| (byGreen >> 2);		// Green
                    *pbyDestination++ = (byRed << 3)	| (byRed >> 2);			// Red
                    pRow += 2;
                }
            }
            break;

            case D3DFMT_X1R5G5B5 :
            case D3DFMT_A1R5G5B5 :
            {
                for (UINT x = 0; x < uWidth; ++x)
                {
                    UINT uColor		= *((UINT*) pRow);
                    BYTE byBlue		= (uColor >> 10) & 0x1F;
                    BYTE byGreen	= (uColor >> 5) & 0x1F;
                    BYTE byRed		= uColor & 0x1F;

                    *pbyDestination++ = (byBlue << 3)	| (byBlue >> 2);		// Blue
                    *pbyDestination++ = (byGreen << 3)	| (byGreen >> 2);		// Green
                    *pbyDestination++ = (byRed << 3)	| (byRed >> 2);			// Red
                    pRow += 2;
                }
            }
            break;
        }

        // increase by one line
        pbySource += lockRect.Pitch;
    }

    if (lpSurface)
    {
        lpSurface->UnlockRect();
        lpSurface->Release();
        lpSurface = NULL;
    }

    bool bSaved = SaveJPEG(c_pszFileName, pbyBuffer, uWidth, uHeight);

    if (pbyBuffer)
    {
        delete [] pbyBuffer;
        pbyBuffer = NULL;
    }

    if (bSaved == false)
    {
        TraceError("Failed to save JPEG file. (%s, %d, %d)", c_pszFileName, uWidth, uHeight);
        return false;
    }

    return true;
}

void CPythonGraphic::PushState()
{
    TState curState;

    curState.matProj = ms_matProj;
    curState.matView = ms_matView;

    m_stateStack.push(curState);
}

void CPythonGraphic::PopState()
{
    if (m_stateStack.empty())
    {
        assert(!"PythonGraphic::PopState StateStack is EMPTY");
        return;
    }

    TState & rState = m_stateStack.top();

    ms_matProj = rState.matProj;
    ms_matView = rState.matView;

    UpdatePipeLineMatrix();

    m_stateStack.pop();
}

void CPythonGraphic::RenderCoolTimeBox(float fxCenter, float fyCenter, float fRadius, float fTime)
{
    if (fTime >= 1.0f)
    {
        return;
    }

    fTime = max(0.0f, fTime);

    static D3DXCOLOR color = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);

    static std::array<D3DXVECTOR2, 8> s_v2BoxPos{ {
        { -1.0f, -1.0f },
        { -1.0f,  0.0f },
        { -1.0f, +1.0f },
        {  0.0f, +1.0f },
        { +1.0f, +1.0f },
        { +1.0f,  0.0f },
        { +1.0f, -1.0f },
        {  0.0f, -1.0f },
    } };

    int iTriCount = int(8 - 8.0f * fTime);
    float fLastPercentage = (8 - 8.0f * fTime) - iTriCount;

    std::vector<TPDTVertex> vertices;
    TPDTVertex vertex;
    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter;
    vertex.position.z = 0.0f;
    vertex.diffuse = color;
    vertex.texCoord.x = 0.0f;
    vertex.texCoord.x = 0.0f;
    vertices.push_back(vertex);
    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter - fRadius;
    vertex.position.z = 0.0f;
    vertex.diffuse = color;
    vertex.texCoord.x = 0.0f;
    vertex.texCoord.x = 0.0f;
    vertices.push_back(vertex);

    for (int j = 0; j < iTriCount; ++j)
    {
        vertex.position.x = fxCenter + s_v2BoxPos[j].x * fRadius;
        vertex.position.y = fyCenter + s_v2BoxPos[j].y * fRadius;
        vertices.push_back(vertex);
    }

    if (fLastPercentage > 0.0f)
    {
        D3DXVECTOR2 * pv2Pos;
        D3DXVECTOR2 * pv2LastPos;

        assert((iTriCount - 1 + 8) % 8 >= 0 && (iTriCount - 1 + 8) % 8 < 8);
        assert((iTriCount + 8) % 8 >= 0 && (iTriCount + 8) % 8 < 8);
        pv2LastPos = &s_v2BoxPos[(iTriCount - 1 + 8) % 8];
        pv2Pos = &s_v2BoxPos[(iTriCount + 8) % 8];

        vertex.position.x = fxCenter + ((pv2Pos->x - pv2LastPos->x) * fLastPercentage + pv2LastPos->x) * fRadius;
        vertex.position.y = fyCenter + ((pv2Pos->y - pv2LastPos->y) * fLastPercentage + pv2LastPos->y) * fRadius;
        vertices.push_back(vertex);
        ++iTriCount;
    }

    if (vertices.empty())
    {
        return;
    }

    if (SetPDTStream(&vertices[0], vertices.size()))
    {
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG1);
        STATEMANAGER.SetTexture(0, NULL);
        STATEMANAGER.SetTexture(1, NULL);
        STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLEFAN, 0, iTriCount);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
    }
}

long CPythonGraphic::GenerateColor(float r, float g, float b, float a)
{
    return GetColor(r, g, b, a);
}

DWORD CPythonGraphic::GetAvailableMemory()
{
    return ms_lpd3dDevice->GetAvailableTextureMem();
}

/* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
LPDIRECT3DDEVICE9EX CPythonGraphic::GetD3DDevice()
{
    return ms_lpd3dDevice;
}

#endif
/* ----------------------------------------------------- */

CPythonGraphic::CPythonGraphic()
{
    m_lightColor = GetColor(1.0f, 1.0f, 1.0f);
    m_darkColor = GetColor(0.0f, 0.0f, 0.0f);

    memset(&m_backupViewport, 0, sizeof(D3DVIEWPORT9));

    m_fOrthoDepth = 1000.0f;
}

CPythonGraphic::~CPythonGraphic()
{
    Tracef("Python Graphic Clear\n");
}
