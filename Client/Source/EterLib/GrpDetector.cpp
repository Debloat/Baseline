#include "StdAfx.h"
#include "../EterBase/Stl.h"
#include "GrpDetector.h"

struct FIsEqualD3DDisplayMode
{
    FIsEqualD3DDisplayMode(D3DDISPLAYMODEEX* pkD3DDMChk)
    {
        m_pkD3DDMChk = pkD3DDMChk;
    }

    BOOL operator()(D3DDISPLAYMODEEX& rkD3DDMTest)
    {
        if (rkD3DDMTest.Width != m_pkD3DDMChk->Width)
        {
            return FALSE;
        }

        if (rkD3DDMTest.Height != m_pkD3DDMChk->Height)
        {
            return FALSE;
        }

        if (rkD3DDMTest.Format != m_pkD3DDMChk->Format)
        {
            return FALSE;
        }

        return TRUE;
    }

    D3DDISPLAYMODEEX* m_pkD3DDMChk;
};

/////////////////////////////////////////////////////////////////////////////////

UINT D3D_CAdapterDisplayModeList::GetDisplayModeNum()
{
    return m_uD3DDMNum;
}

UINT D3D_CAdapterDisplayModeList::GetPixelFormatNum()
{
    return m_uD3DFmtNum;
}


const D3DDISPLAYMODEEX&	D3D_CAdapterDisplayModeList::GetDisplayModer(UINT iD3DDM)
{
    assert(iD3DDM < m_uD3DDMNum);
    return m_akD3DDM[iD3DDM];
}

const D3DFORMAT&		D3D_CAdapterDisplayModeList::GetPixelFormatr(UINT iD3DFmt)
{
    assert(iD3DFmt < m_uD3DFmtNum);
    return m_aeD3DFmt[iD3DFmt];
}

VOID D3D_CAdapterDisplayModeList::Build(IDirect3D9Ex& rkD3D, D3DFORMAT eD3DFmtDefault, UINT iD3DAdapterInfo)
{
    D3DDISPLAYMODEEX* akD3DDM = m_akD3DDM;
    D3DFORMAT* aeD3DFmt = m_aeD3DFmt;

    UINT uD3DDMNum = 0;
    UINT uD3DFmtNum = 0;

    aeD3DFmt[uD3DFmtNum++] = eD3DFmtDefault;

    D3DDISPLAYMODEFILTER filter = {};
    filter.Size = sizeof(D3DDISPLAYMODEFILTER);
    filter.Format = eD3DFmtDefault;

    UINT uAdapterModeNum = rkD3D.GetAdapterModeCountEx(iD3DAdapterInfo, &filter);

    for (UINT iD3DAdapterInfoMode = 0; iD3DAdapterInfoMode < uAdapterModeNum; iD3DAdapterInfoMode++)
    {
        D3DDISPLAYMODEEX kD3DDMCur{};
        kD3DDMCur.Size = sizeof(D3DDISPLAYMODEEX);
        rkD3D.EnumAdapterModesEx(iD3DAdapterInfo, &filter, iD3DAdapterInfoMode, &kD3DDMCur);

        // IsFilterOutLowResolutionMode
        if (kD3DDMCur.Width  < FILTEROUT_LOWRESOLUTION_WIDTH || kD3DDMCur.Height < FILTEROUT_LOWRESOLUTION_HEIGHT)
        {
            continue;
        }

        // FindDisplayMode
        D3DDISPLAYMODEEX* pkD3DDMEnd = akD3DDM + uD3DDMNum;
        D3DDISPLAYMODEEX* pkD3DDMFind = std::find_if(akD3DDM, pkD3DDMEnd, FIsEqualD3DDisplayMode(&kD3DDMCur));

        // IsNewDisplayMode
        if (pkD3DDMFind == pkD3DDMEnd && uD3DDMNum < D3DDISPLAYMODE_MAX)
        {
            D3DDISPLAYMODEEX& rkD3DDMNew = akD3DDM[uD3DDMNum++];
            rkD3DDMNew.Width = kD3DDMCur.Width;
            rkD3DDMNew.Height = kD3DDMCur.Height;
            rkD3DDMNew.Format = kD3DDMCur.Format;

            // FindFormat
            D3DFORMAT* peD3DFmtEnd = aeD3DFmt + uD3DFmtNum;
            D3DFORMAT* peD3DFmtFind = std::find(aeD3DFmt, peD3DFmtEnd, kD3DDMCur.Format);

            // IsNewFormat
            if (peD3DFmtFind == peD3DFmtEnd && uD3DFmtNum < D3DFORMAT_MAX)
            {
                aeD3DFmt[uD3DFmtNum++] = kD3DDMCur.Format;
            }
        }
    }

    std::sort(akD3DDM, akD3DDM + uD3DDMNum, [](const D3DDISPLAYMODEEX & a, const D3DDISPLAYMODEEX & b)
    {
        if (a.Format != b.Format)
            return a.Format > b.Format;

        if (a.Width != b.Width)
            return a.Width < b.Width;

        return a.Height < b.Height;
    });

    m_uD3DFmtNum = uD3DFmtNum;
    m_uD3DDMNum = uD3DDMNum;
}

/////////////////////////////////////////////////////////////////////////////////

VOID D3D_SModeInfo::GetString(std::string* pstEnumList)
{
    UINT uScrDepthBits = 32;
    const char* vp = (m_deviceFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) ? "HWVP" : "INVALID VP";


    char szText[128];
    _snprintf(szText, sizeof(szText), "%dx%dx%d %s\r\n", m_uScrWidth, m_uScrHeight, uScrDepthBits, vp);
    pstEnumList->append(szText);
}

/////////////////////////////////////////////////////////////////////////////////
UINT D3D_CDeviceInfo::GetD3DModeInfoNum()
{
    return m_uD3DModeInfoNum;
}

D3D_SModeInfo* D3D_CDeviceInfo::GetD3DModeInfop(UINT iD3D_SModeInfo)
{
    if (iD3D_SModeInfo >= m_uD3DModeInfoNum)
    {
        return NULL;
    }

    return &m_akD3DModeInfo[iD3D_SModeInfo];
}

BOOL D3D_CDeviceInfo::FindDepthStencilFormat(IDirect3D9Ex& rkD3D, UINT iD3DAdapterInfo, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat)
{
    if (SUCCEEDED(rkD3D.CheckDeviceFormat(iD3DAdapterInfo, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8)) &&
            SUCCEEDED(rkD3D.CheckDepthStencilMatch(iD3DAdapterInfo, DeviceType, TargetFormat, TargetFormat, D3DFMT_D24S8)))
    {
        *pDepthStencilFormat = D3DFMT_D24S8;
        return TRUE;
    }

    TraceError("D3D: Required depth-stencil format D24S8 not supported (adapter=%u, devType=%u, targetFmt=%u)", iD3DAdapterInfo, DeviceType, TargetFormat);
    return FALSE;
}

BOOL D3D_CDeviceInfo::Build(IDirect3D9Ex& rkD3D, UINT iD3DAdapterInfo, D3D_CAdapterDisplayModeList& rkD3DADMList, BOOL (*pfnConfirmDevice)(D3DCAPS9& rkD3DCaps,
                            UINT uBehavior, D3DFORMAT eD3DFmt))
{
    assert(pfnConfirmDevice != NULL && "D3D_CDeviceInfo::Build");

    const D3DDEVTYPE	c_eD3DDevType = D3DDEVTYPE_HAL;
    const TCHAR*		c_szD3DDevDesc = "HAL";

    m_eD3DDevType = c_eD3DDevType;
    rkD3D.GetDeviceCaps(iD3DAdapterInfo, c_eD3DDevType, &m_kD3DCaps);

    m_szDevDesc = c_szD3DDevDesc;
    m_uD3DModeInfoNum = 0;

    /* [KaptanYosun TODO] implement MSAA */
    m_eD3DMultiSampleType = D3DMULTISAMPLE_NONE;

    BOOL  aisFormatConfirmed[20] {};
    DWORD adwD3DBehavior[20] {};
    D3DFORMAT aeD3DFmtDepthStencil[20] {};

    // GetFlagInfo
    {
        UINT uD3DFmtNum = rkD3DADMList.GetPixelFormatNum();

        for (DWORD iFmt = 0; iFmt < uD3DFmtNum; ++iFmt)
        {
            D3DFORMAT eD3DFmtPixel = rkD3DADMList.GetPixelFormatr(iFmt);
            DWORD dwD3DBehavior = 0;

            aeD3DFmtDepthStencil[iFmt] = D3DFMT_UNKNOWN;

            // SkipNoRenderTargetFormat;
            if (FAILED(rkD3D.CheckDeviceType(iD3DAdapterInfo, m_eD3DDevType, eD3DFmtPixel, eD3DFmtPixel, FALSE)))
            {
                continue;
            }

            // Confirm the device/format for HW vertex processing
            if ((m_kD3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) &&
                    pfnConfirmDevice(m_kD3DCaps, D3DCREATE_HARDWARE_VERTEXPROCESSING, eD3DFmtPixel) &&
                    FindDepthStencilFormat(rkD3D, iD3DAdapterInfo, c_eD3DDevType, eD3DFmtPixel, &aeD3DFmtDepthStencil[iFmt]))
            {
                dwD3DBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
                aisFormatConfirmed[iFmt] = TRUE;
            }

            adwD3DBehavior[iFmt] = dwD3DBehavior;
        }
    }

    // BuildModeInfoList
    {
        UINT uD3DDMNum = rkD3DADMList.GetDisplayModeNum();
        UINT uD3DFmtNum = rkD3DADMList.GetPixelFormatNum();


        for (UINT iD3DDM = 0; iD3DDM < uD3DDMNum; ++iD3DDM)
        {
            const D3DDISPLAYMODEEX& c_rkD3DDM = rkD3DADMList.GetDisplayModer(iD3DDM);

            for (DWORD iFmt = 0; iFmt < uD3DFmtNum; ++iFmt)
            {
                if (rkD3DADMList.GetPixelFormatr(iFmt) == c_rkD3DDM.Format)
                {
                    if (aisFormatConfirmed[iFmt] == TRUE)
                    {
                        D3D_SModeInfo& rkModeInfo = m_akD3DModeInfo[m_uD3DModeInfoNum++];
                        rkModeInfo.m_uScrWidth = c_rkD3DDM.Width;
                        rkModeInfo.m_uScrHeight = c_rkD3DDM.Height;
                        rkModeInfo.m_eD3DFmtPixel = c_rkD3DDM.Format;
                        rkModeInfo.m_deviceFlags = adwD3DBehavior[iFmt];
                        rkModeInfo.m_eD3DFmtDepthStencil = aeD3DFmtDepthStencil[iFmt];
                    }
                }
            }
        }
    }

    if (m_uD3DModeInfoNum == 0)
    {
        TraceError("No compatible D3D modes found (Hardware VertexProcessing required).");
        return FALSE;
    }

    return TRUE;
}

BOOL D3D_CDeviceInfo::Find(UINT uScrWidth, UINT uScrHeight, UINT* piD3DModeInfo)
{
    bool foundResolution = false;

    for (UINT i = 0; i < m_uD3DModeInfoNum; ++i)
    {
        const D3D_SModeInfo& mode = m_akD3DModeInfo[i];

        if (mode.m_uScrWidth == uScrWidth && mode.m_uScrHeight == uScrHeight)
        {
            foundResolution = true;

            if (mode.m_eD3DFmtPixel == D3DFMT_A8R8G8B8 || mode.m_eD3DFmtPixel == D3DFMT_X8R8G8B8)
            {
                *piD3DModeInfo = i;
                return TRUE;
            }

            TraceError("D3D: Rejected pixel format %u for %ux%u (32-bit color backbuffer required)", mode.m_eD3DFmtPixel, uScrWidth, uScrHeight);
        }
    }

    if (foundResolution)
    {
        TraceError("D3D: No compatible pixel format found for %ux%u (32-bit color backbuffer required)", uScrWidth, uScrHeight);
    }

    else
    {
        TraceError("D3D: Resolution %ux%u not supported by adapter", uScrWidth, uScrHeight);
    }

    return FALSE;
}

VOID D3D_CDeviceInfo::GetString(std::string* pstEnumList)
{
    char szText[1024 + 1];
    _snprintf(szText, sizeof(szText), "%s\r\n========================================\r\n", m_szDevDesc);
    pstEnumList->append(szText);

    for (UINT iD3D_SModeInfo = 0; iD3D_SModeInfo < m_uD3DModeInfoNum; ++iD3D_SModeInfo)
    {
        _snprintf(szText, sizeof(szText), "%d. ", iD3D_SModeInfo);
        pstEnumList->append(szText);

        D3D_SModeInfo& rkModeInfo = m_akD3DModeInfo[iD3D_SModeInfo];
        rkModeInfo.GetString(pstEnumList);
    }

    pstEnumList->append("\r\n");
}

/////////////////////////////////////////////////////////////////////////////

D3DDISPLAYMODEEX&	D3D_CAdapterInfo::GetDesktopD3DDisplayModer()
{
    return m_kD3DDMDesktop;
}

D3DDISPLAYMODEEX*	D3D_CAdapterInfo::GetDesktopD3DDisplayModep()
{
    return &m_kD3DDMDesktop;
}

D3D_CDeviceInfo* D3D_CAdapterInfo::GetD3DDeviceInfop(UINT iD3DDevInfo)
{
    if (iD3DDevInfo >= m_uD3DDevInfoNum)
    {
        return NULL;
    }

    return &m_akD3DDevInfo[iD3DDevInfo];
}

D3D_SModeInfo* D3D_CAdapterInfo::GetD3DModeInfop(UINT iD3DDevInfo, UINT iD3D_SModeInfo)
{
    D3D_CDeviceInfo* pkD3DDevInfo = GetD3DDeviceInfop(iD3DDevInfo);

    if (pkD3DDevInfo)
    {
        D3D_SModeInfo* pkD3DModeInfo = pkD3DDevInfo->GetD3DModeInfop(iD3D_SModeInfo);

        if (pkD3DModeInfo)
        {
            return pkD3DModeInfo;
        }
    }

    return NULL;
}

BOOL D3D_CAdapterInfo::Find(UINT uScrWidth, UINT uScrHeight, UINT* piD3DModeInfo, UINT* piD3DDevInfo)
{
    for (UINT iDevInfo = 0; iDevInfo < m_uD3DDevInfoNum; ++iDevInfo)
    {
        D3D_CDeviceInfo& rkD3DDevInfo = m_akD3DDevInfo[iDevInfo];

        if (rkD3DDevInfo.Find(uScrWidth, uScrHeight, piD3DModeInfo))
        {
            *piD3DDevInfo = iDevInfo;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL D3D_CAdapterInfo::Build(IDirect3D9Ex& rkD3D, UINT iD3DAdapterInfo, PFNCONFIRMDEVICE pfnConfirmDevice)
{
    D3DDISPLAYMODEEX modeEx = {};
    modeEx.Size = sizeof(D3DDISPLAYMODEEX);

    if (FAILED(rkD3D.GetAdapterDisplayModeEx(iD3DAdapterInfo, &modeEx, NULL)))
    {
        return FALSE;
    }

    D3DDISPLAYMODEEX& rkD3DDMDesktop = m_kD3DDMDesktop;
    rkD3DDMDesktop.Width = modeEx.Width;
    rkD3DDMDesktop.Height = modeEx.Height;
    rkD3DDMDesktop.Format = modeEx.Format;
    rkD3DDMDesktop.RefreshRate = modeEx.RefreshRate;

    rkD3D.GetAdapterIdentifier(iD3DAdapterInfo, D3DENUM_WHQL_LEVEL, &m_kD3DAdapterIdentifier);

    m_iCurD3DDevInfo = 0;
    m_uD3DDevInfoNum = 0;

    D3D_CAdapterDisplayModeList kD3DADMList;
    kD3DADMList.Build(rkD3D, m_kD3DDMDesktop.Format, iD3DAdapterInfo);

    D3D_CDeviceInfo* akD3DDevInfo = m_akD3DDevInfo;
    D3D_CDeviceInfo& rkD3DDevInfo = akD3DDevInfo[m_uD3DDevInfoNum];

    if (rkD3DDevInfo.Build(rkD3D, iD3DAdapterInfo, kD3DADMList, pfnConfirmDevice))
    {
        m_uD3DDevInfoNum = 1;
    }

    if (m_uD3DDevInfoNum > 0)
    {
        return TRUE;
    }

    TraceError("D3D: Adapter %u rejected (no compatible HAL device found: HW VP + D24S8 required)", iD3DAdapterInfo);
    return FALSE;
}

VOID D3D_CAdapterInfo::GetString(std::string* pstEnumList)
{
    for (UINT iDevInfo = 0; iDevInfo < m_uD3DDevInfoNum; ++iDevInfo)
    {
        char szText[1024 + 1];
        _snprintf(szText, sizeof(szText), "Device %d\r\n", iDevInfo);
        pstEnumList->append(szText);

        D3D_CDeviceInfo& rkD3DDevInfo = m_akD3DDevInfo[iDevInfo];
        rkD3DDevInfo.GetString(pstEnumList);
    }
}

/////////////////////////////////////////////////////////////////////////////

D3D_CDisplayModeAutoDetector::D3D_CDisplayModeAutoDetector()
{
    m_uD3DAdapterInfoCount = 0;
}

D3D_CDisplayModeAutoDetector::~D3D_CDisplayModeAutoDetector()
{
}

D3D_CAdapterInfo* D3D_CDisplayModeAutoDetector::GetD3DAdapterInfop(UINT iD3DAdapterInfo)
{
    if (iD3DAdapterInfo >= m_uD3DAdapterInfoCount)
    {
        return NULL;
    }

    return &m_akD3DAdapterInfo[iD3DAdapterInfo];
}

D3D_SModeInfo* D3D_CDisplayModeAutoDetector::GetD3DModeInfop(UINT iD3DAdapterInfo, UINT iD3DDevInfo, UINT iD3D_SModeInfo)
{
    D3D_CAdapterInfo* pkD3DAdapterInfo = GetD3DAdapterInfop(iD3DAdapterInfo);

    if (pkD3DAdapterInfo)
    {
        D3D_CDeviceInfo* pkD3DDevInfo = pkD3DAdapterInfo->GetD3DDeviceInfop(iD3DDevInfo);

        if (pkD3DDevInfo)
        {
            D3D_SModeInfo* pkD3D_SModeInfo = pkD3DDevInfo->GetD3DModeInfop(iD3D_SModeInfo);

            if (pkD3D_SModeInfo)
            {
                return pkD3D_SModeInfo;
            }
        }
    }

    return NULL;
}

BOOL D3D_CDisplayModeAutoDetector::Find(UINT uScrWidth, UINT uScrHeight, UINT* piD3DModeInfo, UINT* piD3DDevInfo, UINT* piD3DAdapterInfo)
{
    for (UINT iD3DAdapterInfo = 0; iD3DAdapterInfo < m_uD3DAdapterInfoCount; ++iD3DAdapterInfo)
    {
        D3D_CAdapterInfo& rkAdapterInfo = m_akD3DAdapterInfo[iD3DAdapterInfo];

        if (rkAdapterInfo.Find(uScrWidth, uScrHeight, piD3DModeInfo, piD3DDevInfo))
        {
            *piD3DAdapterInfo = iD3DAdapterInfo;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL D3D_CDisplayModeAutoDetector::Build(IDirect3D9Ex& rkD3D, PFNCONFIRMDEVICE pfnConfirmDevice)
{
    m_uD3DAdapterInfoCount = 0;

    UINT uTotalAdapterCount = rkD3D.GetAdapterCount();
    uTotalAdapterCount = min(uTotalAdapterCount, D3DADAPTERINFO_NUM);

    for (UINT iD3DAdapterInfo = 0; iD3DAdapterInfo < uTotalAdapterCount; ++iD3DAdapterInfo)
    {
        D3D_CAdapterInfo& rkAdapterInfo = m_akD3DAdapterInfo[m_uD3DAdapterInfoCount];

        if (rkAdapterInfo.Build(rkD3D, iD3DAdapterInfo, pfnConfirmDevice))
        {
            ++m_uD3DAdapterInfoCount;
        }
    }

    if (m_uD3DAdapterInfoCount > 0)
    {
        return TRUE;
    }

    return FALSE;
}

VOID D3D_CDisplayModeAutoDetector::GetString(std::string* pstEnumList)
{
    for (UINT iD3DAdapterInfo = 0; iD3DAdapterInfo < m_uD3DAdapterInfoCount; ++iD3DAdapterInfo)
    {
        char szText[1024 + 1];
        _snprintf(szText, sizeof(szText), "Adapter %d\r\n", iD3DAdapterInfo);
        pstEnumList->append(szText);

        D3D_CAdapterInfo& rkAdapterInfo = m_akD3DAdapterInfo[iD3DAdapterInfo];
        rkAdapterInfo.GetString(pstEnumList);
    }
}