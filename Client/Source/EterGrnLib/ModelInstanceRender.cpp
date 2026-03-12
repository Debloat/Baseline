#include "StdAfx.h"
#include "../EterLib/StateManager.h"
#include "ModelInstance.h"
#include "Model.h"

#include "../EterLib/ShaderVertexDeclarations.h"

void CGrannyModelInstance::DeformNoSkin(const D3DXMATRIX * c_pWorldMatrix)
{
    if (IsEmpty())
    {
        return;
    }

    UpdateWorldPose();
    UpdateWorldMatrices(c_pWorldMatrix);
}

//// Render

void CGrannyModelInstance::__RenderModelFFP(EModelTexturePath eTexPath, EModelRenderPass ePass)
{
    // Keep original early-out behavior
    if (IsEmpty())
        return;

    STATEMANAGER.SetVertexDeclaration(CShaderInputLayouts::Get(EShaderInputLayout::PNT));

    if (eTexPath == MODEL_TEX_NONE)
    {
        // Preserve RenderWithoutTexture side effects exactly
        STATEMANAGER.SetTexture(0, NULL);
        STATEMANAGER.SetTexture(1, NULL);
    }

    // WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
    // END_OF_WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();

    // Determine meshlist mode from tex path
    EMeshNodeListRenderMode eMode;

    if (eTexPath == MODEL_TEX_ONE)
    {
        eMode = MESHNODELIST_ONE_TEXTURE;
    }
    else if (eTexPath == MODEL_TEX_TWO)
    {
        eMode = MESHNODELIST_TWO_TEXTURE;
    }
    else
    {
        eMode = MESHNODELIST_NO_TEXTURE;
    }

    auto RenderType = [&](CGrannyMesh::EType eMeshType, LPDIRECT3DVERTEXBUFFER9 vb, UINT stride, CGrannyMaterial::EType eMtrlType)
        {
            if (!vb)
                return;

            STATEMANAGER.SetStreamSource(0, vb, stride);
            RenderMeshNodeList(eMeshType, eMtrlType, eMode);
        };

    // What material type(s) to draw
    if (ePass == MODEL_PASS_OPAQUE)
    {
        RenderType(CGrannyMesh::TYPE_DEFORM, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex), CGrannyMaterial::TYPE_DIFFUSE_PNT);
        RenderType(CGrannyMesh::TYPE_RIGID, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex), CGrannyMaterial::TYPE_DIFFUSE_PNT);
    }
    else if (ePass == MODEL_PASS_BLEND)
    {
        RenderType(CGrannyMesh::TYPE_DEFORM, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex), CGrannyMaterial::TYPE_BLEND_PNT);
        RenderType(CGrannyMesh::TYPE_RIGID, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex), CGrannyMaterial::TYPE_BLEND_PNT);
    }
    else // MODEL_PASS_ALL (RenderWithoutTexture behavior)
    {
        RenderType(CGrannyMesh::TYPE_DEFORM, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex), CGrannyMaterial::TYPE_DIFFUSE_PNT);
        RenderType(CGrannyMesh::TYPE_DEFORM, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex), CGrannyMaterial::TYPE_BLEND_PNT);

        RenderType(CGrannyMesh::TYPE_RIGID, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex), CGrannyMaterial::TYPE_DIFFUSE_PNT);
        RenderType(CGrannyMesh::TYPE_RIGID, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex), CGrannyMaterial::TYPE_BLEND_PNT);
    }
}

//// Render Mesh List

void CGrannyModelInstance::RenderMeshNodeList(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType, EMeshNodeListRenderMode eMode)
{
    assert(m_pModel != NULL);

    LPDIRECT3DINDEXBUFFER9 lpd3dIdxBuf = m_pModel->GetD3DIndexBuffer();
    assert(lpd3dIdxBuf != NULL);

    const CGrannyModel::TMeshNode* pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);

    while (pMeshNode)
    {
        const CGrannyMesh* pMesh = pMeshNode->pMesh;
        const int vtxMeshBasePos = pMesh->GetVertexBasePosition();

        STATEMANAGER.SetIndices(lpd3dIdxBuf, vtxMeshBasePos);
        STATEMANAGER.SetTransform(D3DTS_WORLD, &m_meshMatrices[pMeshNode->iMesh]);

        const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
        const int vtxCount = pMesh->GetVertexCount();

        while (pTriGroupNode)
        {
            ms_faceCount += pTriGroupNode->triCount;

            switch (eMode)
            {
            case MESHNODELIST_ONE_TEXTURE:
            {
                CGrannyMaterial& rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);

                // ------------------------------------------------------------------
                // Stage 0: Diffuse
                // ------------------------------------------------------------------
                STATEMANAGER.SetTexture(0, rkMtrl.GetD3DTexture(0));

                // ------------------------------------------------------------------
                // Two-sided handling
                // ------------------------------------------------------------------
                DWORD oldCull = 0;
                if (rkMtrl.IsTwoSided())
                {
                    oldCull = STATEMANAGER.GetRenderState(D3DRS_CULLMODE);
                    STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
                }

                // ------------------------------------------------------------------
                // Specular (FFP sphere map) — PHASE 4
                // ------------------------------------------------------------------
                bool specularActive = false;

                if (rkMtrl.IsSpecularEnabled() && !STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE))
                {
                    specularActive = true;

                    // Bind sphere map texture to stage 1
                    CGraphicTexture* pkTexture = CGrannyMaterial::GetSphereMapTexture(rkMtrl.GetSphereMapIndex());

                    STATEMANAGER.SetTexture(1, pkTexture ? pkTexture->GetD3DTexture() : NULL);

                    // Texture factor (specular power in alpha)
                    const D3DXCOLOR& specColor = CGrannyMaterial::GetSpecularColor();
                    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(specColor.r, specColor.g, specColor.b, rkMtrl.GetSpecularPower()));

                    // ------------------------------------------------------
                    // Stage 0 alpha = texture * textureFactor (spec power)
                    // ------------------------------------------------------
                    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
                    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

                    // Stage1 combiner setup (match old behavior)
                    STATEMANAGER.SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

                    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
                    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
                    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATEALPHA_ADDCOLOR);

                    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
                    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

                    STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &CGrannyMaterial::GetSpecularMatrix());

                    STATEMANAGER.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
                }

                // ------------------------------------------------------------------
                // Draw
                // ------------------------------------------------------------------
                STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);

                // ------------------------------------------------------------------
                // Specular cleanup
                // ------------------------------------------------------------------
                if (specularActive)
                {
                    // --------------------------------------------------
                    // Restore Stage 0 alpha state
                    // --------------------------------------------------
                    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
                    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

                    // --------------------------------------------------
                    // Disable Stage 1
                    // --------------------------------------------------
                    STATEMANAGER.SetTexture(1, NULL);

                    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
                    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                    STATEMANAGER.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
                }

                // ------------------------------------------------------------------
                // Restore cull
                // ------------------------------------------------------------------
                if (rkMtrl.IsTwoSided())
                {
                    STATEMANAGER.SetRenderState(D3DRS_CULLMODE, oldCull);
                }
            }
            break;

            case MESHNODELIST_TWO_TEXTURE:
            {
                const CGrannyMaterial& rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
                STATEMANAGER.SetTexture(0, rkMtrl.GetD3DTexture(0));
                STATEMANAGER.SetTexture(1, rkMtrl.GetD3DTexture(1));
                STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
            }
            break;

            case MESHNODELIST_NO_TEXTURE:
            {
                STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
            }
            break;
            }

            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        pMeshNode = pMeshNode->pNextMeshNode;
    }
}
