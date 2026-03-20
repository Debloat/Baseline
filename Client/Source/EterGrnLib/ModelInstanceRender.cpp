#include "StdAfx.h"
#include "../EterLib/StateManager.h"
#include "ModelInstance.h"
#include "Model.h"

#include "../EterLib/ShaderVertexDeclarations.h"
#include "../EterLib/GrpDevice.h"

void CGrannyModelInstance::DeformNoSkin(const D3DXMATRIX * c_pWorldMatrix)
{
    if (IsEmpty())
    {
        return;
    }

    UpdateWorldPose();
    UpdateWorldMatrices(c_pWorldMatrix);
}

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 2> ModelSamplers =
    { {
        { 0, ESamplerState::LinearWrap },
        { 1, ESamplerState::LinearWrap }
    } };

    constexpr PipelineStateDesc ModelOpaquePipeline =
    {
        ShaderID::Model,
        EDepthState::EnabledWrite,
        EBlendState::Opaque,
        ERasterState::CullFront,
        ModelSamplers.data(),
        ModelSamplers.size()
    };

    constexpr PipelineStateDesc ModelBlendPipeline =
    {
        ShaderID::Model,
        EDepthState::EnabledWrite,
        EBlendState::AlphaBlend,
        ERasterState::CullFront,
        ModelSamplers.data(),
        ModelSamplers.size()
    };
}

//// Render

void CGrannyModelInstance::__RenderModelFFP(EModelTexturePath eTexPath, EModelRenderPass ePass)
{
    // Keep original early-out behavior
    if (IsEmpty())
        return;

    IShaderProvider const* sp = GetShaderProvider();
    if (!sp)
        return;

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

            if (eMtrlType == CGrannyMaterial::TYPE_BLEND_PNT)
            {
                if (!sp->BindPipelineState(ModelBlendPipeline))
                    return;
            }
            else
            {
                if (!sp->BindPipelineState(ModelOpaquePipeline))
                    return;
            }

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

    IShaderProvider const* sp = GetShaderProvider();
    if (!sp)
    {
        return;
    }

    LPDIRECT3DINDEXBUFFER9 lpd3dIdxBuf = m_pModel->GetD3DIndexBuffer();
    assert(lpd3dIdxBuf != NULL);

    const CGrannyModel::TMeshNode* pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);

    while (pMeshNode)
    {
        const CGrannyMesh* pMesh = pMeshNode->pMesh;
        const int vtxMeshBasePos = pMesh->GetVertexBasePosition();

        STATEMANAGER.SetIndices(lpd3dIdxBuf, vtxMeshBasePos);

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

                ModelShaderInputs in{};

                // world
                std::memcpy(in.vs.world.data(), &m_meshMatrices[pMeshNode->iMesh], sizeof(D3DXMATRIX));

                // viewProj
                D3DXMATRIX viewProj = CGraphicBase::GetViewMatrix() * CGraphicBase::GetProjMatrix();
                std::memcpy(in.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));
                in.ps.textureFlags[0] = rkMtrl.GetD3DTexture(0) ? 1.0f : 0.0f;
                in.ps.textureFlags[1] = 0.0f;
                in.ps.textureFlags[2] = 0.0f;
                in.ps.textureFlags[3] = 0.0f;

                CGraphicDevice::UploadModelConstants(in);

                if (rkMtrl.IsTwoSided())
                {
                    sp->BindRasterState(ERasterState::CullNone);
                }
                else
                {
                    sp->BindRasterState(ERasterState::CullFront);
                }

                STATEMANAGER.SetTexture(0, rkMtrl.GetD3DTexture(0));
                STATEMANAGER.SetTexture(1, NULL);

                STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
            }
            break;

            case MESHNODELIST_TWO_TEXTURE:
            {
                const CGrannyMaterial& rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);

                ModelShaderInputs in{};

                // world
                std::memcpy(in.vs.world.data(), &m_meshMatrices[pMeshNode->iMesh], sizeof(D3DXMATRIX));

                // viewProj
                D3DXMATRIX viewProj = CGraphicBase::GetViewMatrix() * CGraphicBase::GetProjMatrix();
                std::memcpy(in.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));
                in.ps.textureFlags[0] = rkMtrl.GetD3DTexture(0) ? 1.0f : 0.0f;
                in.ps.textureFlags[1] = rkMtrl.GetD3DTexture(1) ? 1.0f : 0.0f;
                in.ps.textureFlags[2] = 0.0f;
                in.ps.textureFlags[3] = 0.0f;

                CGraphicDevice::UploadModelConstants(in);

                if (rkMtrl.IsTwoSided())
                {
                    sp->BindRasterState(ERasterState::CullNone);
                }
                else
                {
                    sp->BindRasterState(ERasterState::CullFront);
                }

                STATEMANAGER.SetTexture(0, rkMtrl.GetD3DTexture(0));
                STATEMANAGER.SetTexture(1, rkMtrl.GetD3DTexture(1));

                STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
            }
            break;

            case MESHNODELIST_NO_TEXTURE:
            {
                ModelShaderInputs in{};

                // world
                std::memcpy(in.vs.world.data(), &m_meshMatrices[pMeshNode->iMesh], sizeof(D3DXMATRIX));

                // viewProj
                D3DXMATRIX viewProj = CGraphicBase::GetViewMatrix() * CGraphicBase::GetProjMatrix();
                std::memcpy(in.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));
                in.ps.textureFlags[0] = 0.0f;
                in.ps.textureFlags[1] = 0.0f;
                in.ps.textureFlags[2] = 0.0f;
                in.ps.textureFlags[3] = 0.0f;

                CGraphicDevice::UploadModelConstants(in);

                sp->BindRasterState(ERasterState::CullFront);

                STATEMANAGER.SetTexture(0, NULL);
                STATEMANAGER.SetTexture(1, NULL);

                STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
            }
            break;
            }

            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        pMeshNode = pMeshNode->pNextMeshNode;
    }
}
