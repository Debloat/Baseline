#include "StdAfx.h"
#include "ShaderManager.h"

ShaderManager& ShaderManager::Instance()
{
    static ShaderManager s_instance;
    return s_instance;
}

ShaderManager::~ShaderManager()
{
    Clear();
}

void ShaderManager::SetDevice(LPDIRECT3DDEVICE9 device)
{
    m_device = device;
}

void ShaderManager::Clear()
{
    for (auto& it : m_vsCache)
    {
        if (it.second.shader)
        {
            it.second.shader->Release();
            it.second.shader = nullptr;
        }
        if (it.second.constants)
        {
            it.second.constants->Release();
            it.second.constants = nullptr;
        }
    }
    m_vsCache.clear();

    for (auto& it : m_psCache)
    {
        if (it.second.shader)
        {
            it.second.shader->Release();
            it.second.shader = nullptr;
        }
        if (it.second.constants)
        {
            it.second.constants->Release();
            it.second.constants = nullptr;
        }
    }
    m_psCache.clear();
}

bool ShaderManager::ReadFileFromPack(const char* packFilename, std::string* outCode)
{
    if (!packFilename || !outCode)
        return false;

    outCode->clear();

    CMappedFile mappedFile;
    LPCVOID pData = nullptr;

    if (!CEterPackManager::Instance().Get(mappedFile, packFilename, &pData))
        return false;

    outCode->assign(static_cast<const char*>(pData), mappedFile.Size());
    return true;
}

bool ShaderManager::CompileHLSL(
    const char* hlslCode,
    std::size_t hlslCodeSize,
    const char* entryPoint,
    const char* profile,
    LPD3DXBUFFER* outBytecode,
    LPD3DXCONSTANTTABLE* outConstants)
{
    if (!hlslCode || hlslCodeSize == 0 || !entryPoint || !profile || !outBytecode)
        return false;

    *outBytecode = nullptr;
    if (outConstants)
        *outConstants = nullptr;

    LPD3DXBUFFER errorBuffer = nullptr;

    const HRESULT hr = D3DXCompileShader(
        hlslCode,
        hlslCodeSize,
        nullptr,
        nullptr,
        entryPoint,
        profile,
        0,
        outBytecode,
        &errorBuffer,
        outConstants);

    if (FAILED(hr))
    {
        if (errorBuffer)
        {
            const auto* msg = static_cast<const char*>(errorBuffer->GetBufferPointer());
            TraceError("Shader compile failed (%s / %s): %s", entryPoint, profile, msg ? msg : "");
            errorBuffer->Release();
        }
        else
        {
            TraceError("Shader compile failed (%s / %s): HRESULT=0x%08x", entryPoint, profile, hr);
        }

        if (*outBytecode)
        {
            (*outBytecode)->Release();
            *outBytecode = nullptr;
        }
        if (outConstants && *outConstants)
        {
            (*outConstants)->Release();
            *outConstants = nullptr;
        }

        return false;
    }

    if (errorBuffer)
        errorBuffer->Release();

    return true;
}

namespace
{
    constexpr const char* kEntryPoint = "main";
    constexpr const char* kVSProfile = "vs_3_0";
    constexpr const char* kPSProfile = "ps_3_0";
}

bool ShaderManager::GetVertexShaderFromPack(const char* packFilename, VertexShaderHandle* out)
{
    if (!m_device || !packFilename || !out)
        return false;

    const ShaderKey key{ packFilename, kEntryPoint, kVSProfile };

    if (const auto found = m_vsCache.find(key); found != m_vsCache.end())
    {
        *out = found->second;
        return (out->shader != nullptr);
    }

    std::string code;
    if (!ReadFileFromPack(packFilename, &code))
    {
        TraceError("VS ReadFileFromPack failed: %s", packFilename);
        return false;
    }

    LPD3DXBUFFER bytecode = nullptr;
    LPD3DXCONSTANTTABLE constants = nullptr;
    if (!CompileHLSL(code.data(), code.size(), kEntryPoint, kVSProfile, &bytecode, &constants))
        return false;

    LPDIRECT3DVERTEXSHADER9 shader = nullptr;
    const HRESULT hr = m_device->CreateVertexShader(static_cast<const DWORD*>(bytecode->GetBufferPointer()), &shader);

    bytecode->Release();
    bytecode = nullptr;

    if (FAILED(hr) || !shader)
    {
        TraceError("CreateVertexShader failed: %s (HRESULT=0x%08x)", packFilename, hr);
        if (constants) constants->Release();
        return false;
    }

    VertexShaderHandle handle;
    handle.shader = shader;
    handle.constants = constants;

    m_vsCache.try_emplace(key, handle);
    *out = handle;
    return true;
}

bool ShaderManager::GetPixelShaderFromPack(const char* packFilename, PixelShaderHandle* out)
{
    if (!m_device || !packFilename || !out)
        return false;

    const ShaderKey key{ packFilename, kEntryPoint, kPSProfile };

    if (const auto found = m_psCache.find(key); found != m_psCache.end())
    {
        *out = found->second;
        return (out->shader != nullptr);
    }

    std::string code;
    if (!ReadFileFromPack(packFilename, &code))
    {
        TraceError("PS ReadFileFromPack failed: %s", packFilename);
        return false;
    }

    LPD3DXBUFFER bytecode = nullptr;
    LPD3DXCONSTANTTABLE constants = nullptr;
    if (!CompileHLSL(code.data(), code.size(), kEntryPoint, kPSProfile, &bytecode, &constants))
        return false;

    LPDIRECT3DPIXELSHADER9 shader = nullptr;
    const HRESULT hr = m_device->CreatePixelShader(static_cast<const DWORD*>(bytecode->GetBufferPointer()), &shader);

    bytecode->Release();
    bytecode = nullptr;

    if (FAILED(hr) || !shader)
    {
        TraceError("CreatePixelShader failed: %s (HRESULT=0x%08x)", packFilename, hr);
        if (constants) constants->Release();
        return false;
    }

    PixelShaderHandle handle;
    handle.shader = shader;
    handle.constants = constants;

    m_psCache.try_emplace(key, handle);
    *out = handle;
    return true;
}
