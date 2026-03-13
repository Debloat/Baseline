#include "StdAfx.h"
#include "ShaderVertexDeclarations.h"

#include <utility>

LPDIRECT3DVERTEXDECLARATION9 CShaderInputLayouts::ms_layouts[
    static_cast<std::size_t>(std::to_underlying(EShaderInputLayout::Count))
] = { nullptr };

namespace
{
    // Position (float3) + TexCoord (float2)
    static const D3DVERTEXELEMENT9 kDecl_PT[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    // Position (float3) + TexCoord (float2) + Color (d3dcolor)
    static const D3DVERTEXELEMENT9 kDecl_PTC[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        D3DDECL_END()
    };

    // Position (float3) + Color (d3dcolor) + TexCoord (float2)
    static const D3DVERTEXELEMENT9 kDecl_PCT[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        { 0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    // Position + Normal
    static const D3DVERTEXELEMENT9 kDecl_PN[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        D3DDECL_END()
    };

    // Position + Normal + TexCoord
    static const D3DVERTEXELEMENT9 kDecl_PNT[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    // Position + Normal + TexCoord0 + TexCoord1
    static const D3DVERTEXELEMENT9 kDecl_PNTT[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
        D3DDECL_END()
    };

    const D3DVERTEXELEMENT9* GetDesc(EShaderInputLayout layout)
    {
        switch (layout)
        {
        case EShaderInputLayout::PT:
            return kDecl_PT;

        case EShaderInputLayout::PTC:
            return kDecl_PTC;

        case EShaderInputLayout::PCT:
            return kDecl_PCT;

        case EShaderInputLayout::PN:
            return kDecl_PN;

        case EShaderInputLayout::PNT:
            return kDecl_PNT;

        case EShaderInputLayout::PNTT:
            return kDecl_PNTT;

        default:
            return nullptr;
        }
    }
}

bool CShaderInputLayouts::CreateAll(LPDIRECT3DDEVICE9 device)
{
    if (!device)
        return false;

    for (std::size_t i = 0; i < static_cast<std::size_t>(std::to_underlying(EShaderInputLayout::Count)); ++i)
    {
        const auto layout = static_cast<EShaderInputLayout>(i);
        const D3DVERTEXELEMENT9* desc = GetDesc(layout);

        if (!desc)
            return false;

        if (FAILED(device->CreateVertexDeclaration(desc, &ms_layouts[i])))
            return false;
    }

    return true;
}

void CShaderInputLayouts::DestroyAll()
{
    for (std::size_t i = 0; i < static_cast<std::size_t>(std::to_underlying(EShaderInputLayout::Count)); ++i)
    {
        if (ms_layouts[i])
        {
            ms_layouts[i]->Release();
            ms_layouts[i] = nullptr;
        }
    }
}

LPDIRECT3DVERTEXDECLARATION9 CShaderInputLayouts::Get(EShaderInputLayout layout)
{
    return ms_layouts[static_cast<std::size_t>(std::to_underlying(layout))];
}
