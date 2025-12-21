#include "StdAfx.h"
#include "ShaderProvider.h"

static IShaderProvider* g_shaderProvider = nullptr;

void RegisterShaderProvider(IShaderProvider* provider)
{
    g_shaderProvider = provider;
}

const IShaderProvider* GetShaderProvider()
{
    return g_shaderProvider;
}

IShaderProvider* GetShaderProviderMutable()
{
    return g_shaderProvider;
}
