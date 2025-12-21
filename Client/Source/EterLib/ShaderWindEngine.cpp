#include "StdAfx.h"
#include "ShaderWindEngine.h"

void ShaderWindEngine::Reset()
{
    m_baseDirection = D3DXVECTOR2(1.0f, 0.0f);
    m_baseStrength = 0.0f;
    m_cached.direction = m_baseDirection;
    m_cached.strength = 0.0f;
}

void ShaderWindEngine::SetDirection(const D3DXVECTOR2& dir)
{
    if (D3DXVec2LengthSq(&dir) > 1e-6f)
        m_baseDirection = dir;
}

void ShaderWindEngine::SetStrength(float strength)
{
    m_baseStrength = (strength < 0.0f) ? 0.0f : strength;
}

void ShaderWindEngine::Advance(float /*deltaSeconds*/)
{
    // v1: no time-based modulation, just normalize + clamp

    D3DXVECTOR2 dir = m_baseDirection;
    float len = D3DXVec2Length(&dir);

    if (len > 1e-6f)
        dir /= len;
    else
        dir = D3DXVECTOR2(1.0f, 0.0f);

    m_cached.direction = dir;
    m_cached.strength = m_baseStrength;
}

ShaderWindState ShaderWindEngine::GetState() const
{
    return m_cached;
}
