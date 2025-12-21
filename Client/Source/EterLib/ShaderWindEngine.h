#pragma once
#include <DirectX9/d3dx9math.h>
#include "ShaderParameters.h"

struct ShaderWindState
{
    D3DXVECTOR2 direction; // normalized, world XZ
    float       strength;  // >= 0
};

class ShaderWindEngine
{
public:
    void Reset();

    void Advance(float deltaSeconds);

    void SetDirection(const D3DXVECTOR2& dir);
    void SetStrength(float strength);

    ShaderWindState GetState() const;

private:
    

    D3DXVECTOR2 m_baseDirection;
    float       m_baseStrength;

    ShaderWindState m_cached;
};
