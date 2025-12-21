#pragma once

#include <string>
#include <map>

#include <DirectX9/d3d9.h>
#include <DirectX9/d3dx9shader.h>

#include "../EterPack/EterPackManager.h"

class ShaderManager
{
public:
    static ShaderManager& Instance();

    struct VertexShaderHandle
    {
        LPDIRECT3DVERTEXSHADER9 shader = nullptr;
        LPD3DXCONSTANTTABLE constants = nullptr;
    };

    struct PixelShaderHandle
    {
        LPDIRECT3DPIXELSHADER9 shader = nullptr;
        LPD3DXCONSTANTTABLE constants = nullptr;
    };

public:
    ShaderManager() = default;
    ~ShaderManager();

    void SetDevice(LPDIRECT3DDEVICE9 device);
    void Clear();

    bool GetVertexShaderFromPack(const char* packFilename, VertexShaderHandle* out);
    bool GetPixelShaderFromPack(const char* packFilename, PixelShaderHandle* out);

private:
    struct ShaderKey
    {
        std::string packFilename;
        std::string entryPoint;
        std::string profile;

        bool operator<(const ShaderKey& rhs) const
        {
            if (packFilename != rhs.packFilename) return packFilename < rhs.packFilename;
            if (entryPoint   != rhs.entryPoint)   return entryPoint   < rhs.entryPoint;
            return profile < rhs.profile;
        }
    };

private:
    static bool ReadFileFromPack(const char* packFilename, std::string* outCode);

    bool CompileHLSL(
        const char* hlslCode,
        std::size_t hlslCodeSize,
        const char* entryPoint,
        const char* profile,
        LPD3DXBUFFER* outBytecode,
        LPD3DXCONSTANTTABLE* outConstants);

private:
    LPDIRECT3DDEVICE9 m_device = nullptr;

    std::map<ShaderKey, VertexShaderHandle> m_vsCache;
    std::map<ShaderKey, PixelShaderHandle>  m_psCache;
};
