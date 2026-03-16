#pragma once

#include "SpeedTreeConfig.h"
#include <map>
#include <string>

///////////////////////////////////////////////////////////////////////
//  Branch & Frond Vertex Formats

static DWORD D3DFVF_SPEEDTREE_BRANCH_VERTEX =
    D3DFVF_XYZ |
    D3DFVF_NORMAL |
    D3DFVF_TEXCOORDSIZE2(0) | // m_fTexCoords
    D3DFVF_TEXCOORDSIZE2(1)   // m_fWindIndex, m_fWindWeight
#ifdef WRAPPER_USE_GPU_WIND
    | D3DFVF_TEX2;
#endif

///////////////////////////////////////////////////////////////////////
// FVF Branch Vertex Structure

struct SFVFBranchVertex
{
    D3DXVECTOR3     m_vPosition;            // Always Used
    D3DXVECTOR3     m_vNormal;
    FLOAT           m_fTexCoords[2];        // Always Used
#ifdef WRAPPER_USE_GPU_WIND
    FLOAT           m_fWindIndex;           // GPU Only
    FLOAT           m_fWindWeight;
#endif
};


///////////////////////////////////////////////////////////////////////
//  Branch/Frond Vertex Program

static const char g_achSimpleVertexProgram[] =
{
    "vs.1.1\n"                                              // identity shader version

    "dcl_position		v0\n"                            // declare the data coming in
    "dcl_texcoord0		v7\n"
    "dcl_texcoord1		v8\n"

    "mov		oT0.xy,		v7\n"                            // always pass texcoord0 through

    // retrieve and convert wind matrix index
    "mov		a0.x,	v8.x\n"

    // perform wind interpolation
    "m4x4		r1,			v0,			c[54+a0.x]\n"        // compute full wind effect
    "sub		r2,			r1,			v0\n"             // compute difference between full wind and none
    "mov		r3.x,	v8.y\n"                         // mad can't access two v's at once, use r3.x as tmp
    "mad		r1,			r2,			r3.x,		v0\n"  // perform interpolation

    "add		r2,			c[52],		r1\n"               // translate to tree's position
    "m4x4		oPos,		r2,			c[0]\n"             // project to screen
};


///////////////////////////////////////////////////////////////////////
//  LoadBranchShader
#if defined WRAPPER_USE_GPU_LEAF_PLACEMENT || defined WRAPPER_USE_GPU_WIND
static LPDIRECT3DVERTEXSHADER9 LoadBranchShader(LPDIRECT3DDEVICE9EX pDx)
{
    // assemble shader
    LPDIRECT3DVERTEXSHADER9 dwShader = nullptr;
    LPD3DXBUFFER           pCode;
    LPD3DXBUFFER           pError;

    if (D3DXAssembleShader(g_achSimpleVertexProgram, sizeof(g_achSimpleVertexProgram) - 1, nullptr, nullptr, 0, &pCode, &pError) == D3D_OK)
    {
        if (pDx->CreateVertexShader(static_cast<const DWORD*> (pCode->GetBufferPointer()), &dwShader) != D3D_OK)
        {
            char szError[1024];
            sprintf(szError, "Failed to create branch vertex shader.");
            MessageBox(nullptr, szError, "Vertex Shader Error", MB_ICONSTOP);
        }
    }

    else
    {
        char szError[1024];
        sprintf(szError, "Failed to assemble branch vertex shader.\nThe error reported is [ %s ].\n", pError->GetBufferPointer());
        MessageBox(nullptr, szError, "Vertex Shader Error", MB_ICONSTOP);
    }

    if (pCode)
    {
        pCode->Release();
    }

    return dwShader;
}
#endif

///////////////////////////////////////////////////////////////////////
//  Leaf Vertex Formats

static DWORD D3DFVF_SPEEDTREE_LEAF_VERTEX =
    D3DFVF_XYZ |                            // always have the position
    D3DFVF_TEXCOORDSIZE2(0) |
    D3DFVF_TEXCOORDSIZE2(1) |
    D3DFVF_TEXCOORDSIZE2(2) |
#if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
    D3DFVF_TEX3;
#endif


///////////////////////////////////////////////////////////////////////
// FVF Leaf Vertex Structure

struct SFVFLeafVertex
{
    D3DXVECTOR3     m_vPosition;            // Always Used
    D3DXVECTOR3     m_vNormal;
    FLOAT           m_fTexCoords[2];        // Always Used
#if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
    FLOAT           m_fWindIndex;           // Only used when GPU is involved
    FLOAT           m_fWindWeight;
    FLOAT           m_fLeafPlacementIndex;
    FLOAT           m_fLeafScalarValue;
#endif
};


///////////////////////////////////////////////////////////////////////
//  Leaf Vertex Program

static const char g_achLeafVertexProgram[] =
{
    "vs.1.1\n"                                          // identity shader version

    "dcl_position		v0\n"                            // declare the data coming in
    "dcl_texcoord0		v7\n"
    "dcl_texcoord1		v8\n"
    "dcl_texcoord2		v9\n"

    "mov		oT0.xy,	v7\n"                         // always pass texcoord0 through

#ifdef WRAPPER_USE_GPU_WIND
    // retrieve and convert wind matrix index
    "mov		a0.x,	v8.x\n"

    // perform wind interpolation
    "m4x4		r1,		v0,			c[54+a0.x]\n"     // compute full wind effect
    "sub		r2,		r1,			v0\n"              // compute difference between full wind and none
    "mov		r3.x,	v8.y\n"                         // mad can't access two v's at once, use r3.x as tmp
    "mad		r0,		r2,			r3.x,		v0\n"   // perform interpolation
#endif

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
    "mov		a0.x,	v9.x\n"                         // place the leaves
    "mul		r1,		c[a0.x],	v9.y\n"
    "add		r0,		r1,			r0\n"
#endif

    "add		r0,		c[52],		r0\n"                // translate to tree's position
    "m4x4		oPos,	r0,			c[0]\n"              // project to screen
};


///////////////////////////////////////////////////////////////////////
//  LoadLeafShader
#if defined WRAPPER_USE_GPU_LEAF_PLACEMENT || defined WRAPPER_USE_GPU_WIND
static LPDIRECT3DVERTEXSHADER9 LoadLeafShader(LPDIRECT3DDEVICE9EX pDx)
{
    LPDIRECT3DVERTEXSHADER9 dwShader = nullptr;

    // assemble shader
    LPD3DXBUFFER pCode;
    LPD3DXBUFFER pError;

    if (D3DXAssembleShader(g_achLeafVertexProgram, sizeof(g_achLeafVertexProgram) - 1, nullptr, nullptr, 0, &pCode, &pError) == D3D_OK)
    {
        if (pDx->CreateVertexShader(static_cast<const DWORD*> (pCode->GetBufferPointer()), &dwShader) != D3D_OK)
        {
            char szError[1024];
            sprintf(szError, "Failed to create leaf vertex shader.");
            (void)MessageBox(nullptr, szError, "Vertex Shader Error", MB_ICONSTOP);
        }
    }

    else
    {
        char szError[1024];
        sprintf(szError, "Failed to assemble leaf vertex shader. The error reported is [ %s ].\n", (char*)pError->GetBufferPointer());
        (void)MessageBox(nullptr, szError, "Vertex Shader Error", MB_ICONSTOP);
    }

    if (pCode)
    {
        pCode->Release();
    }

    return dwShader;
}
#endif
