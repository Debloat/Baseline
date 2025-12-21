#pragma once
#include <d3d9.h>
#include <cstddef>
#include <cstdint>
#include <utility>

enum class EShaderInputLayout : std::uint8_t
{
    PT,     // Position + Tex
    PTC,    // Position + Tex + Color
    PCT,    // Position + Color + Tex
    PNT,    // Position + Normal + Tex
    PNTT,   // Position + Normal + Tex0 + Tex1
    Count
};

class CShaderInputLayouts
{
public:
    static bool CreateAll(LPDIRECT3DDEVICE9 device);
    static void DestroyAll();
    static LPDIRECT3DVERTEXDECLARATION9 Get(EShaderInputLayout layout);

private:
    static LPDIRECT3DVERTEXDECLARATION9 ms_layouts[
        static_cast<std::size_t>(std::to_underlying(EShaderInputLayout::Count))
    ];
};
