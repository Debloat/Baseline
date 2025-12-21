#include "StdAfx.h"
#include "../EterLib/StateManager.h"
#include "ModelInstance.h"
#include "Model.h"

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

// With One Texture
void CGrannyModelInstance::RenderWithOneTexture()
{
    // FIXME : Deform, Render, BlendRender를 묶어 상위에서 걸러주는 것이 더 나을 듯 - [levites]
    if (IsEmpty())
    {
        return;
    }

    STATEMANAGER.SetVertexDeclaration(ms_pntVS);

    // WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
    // END_OF_WORK

    LPDIRECT3DVERTEXBUFFER9 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();

    if (lpd3dDeformPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
    }

    if (lpd3dRigidPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
    }
}

void CGrannyModelInstance::BlendRenderWithOneTexture()
{
    if (IsEmpty())
    {
        return;
    }

    // WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
    // END_OF_WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();

    STATEMANAGER.SetVertexDeclaration(ms_pntVS);

    if (lpd3dDeformPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
    }

    if (lpd3dRigidPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
    }
}

// With Two Texture
void CGrannyModelInstance::RenderWithTwoTexture()
{
    // FIXME : Deform, Render, BlendRender를 묶어 상위에서 걸러주는 것이 더 나을 듯 - [levites]
    if (IsEmpty())
    {
        return;
    }

    STATEMANAGER.SetVertexDeclaration(ms_pntVS);

    // WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
    // END_OF_WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();

    if (lpd3dDeformPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
    }

    if (lpd3dRigidPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
    }
}

void CGrannyModelInstance::BlendRenderWithTwoTexture()
{
    if (IsEmpty())
    {
        return;
    }

    // WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
    // END_OF_WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();

    STATEMANAGER.SetVertexDeclaration(ms_pntVS);

    if (lpd3dDeformPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
    }

    if (lpd3dRigidPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
    }
}

void CGrannyModelInstance::RenderWithoutTexture()
{
    if (IsEmpty())
    {
        return;
    }

    STATEMANAGER.SetVertexDeclaration(ms_pntVS);
    STATEMANAGER.SetTexture(0, NULL);
    STATEMANAGER.SetTexture(1, NULL);

    // WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
    // END_OF_WORK
    LPDIRECT3DVERTEXBUFFER9 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();

    if (lpd3dDeformPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
        RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
    }

    if (lpd3dRigidPNTVtxBuf)
    {
        STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
        RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
        RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
    }
}


//// Render Mesh List

// With One Texture
void CGrannyModelInstance::RenderMeshNodeListWithOneTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
    assert(m_pModel != NULL);

    LPDIRECT3DINDEXBUFFER9 lpd3dIdxBuf = m_pModel->GetD3DIndexBuffer();
    assert(lpd3dIdxBuf != NULL);

    const CGrannyModel::TMeshNode * pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);

    while (pMeshNode)
    {
        const CGrannyMesh * pMesh = pMeshNode->pMesh;
        int vtxMeshBasePos = pMesh->GetVertexBasePosition();

        STATEMANAGER.SetIndices(lpd3dIdxBuf, vtxMeshBasePos);
        STATEMANAGER.SetTransform(D3DTS_WORLD, &m_meshMatrices[pMeshNode->iMesh]);

        /////
        const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
        int vtxCount = pMesh->GetVertexCount();

        while (pTriGroupNode)
        {
            ms_faceCount += pTriGroupNode->triCount;

            CGrannyMaterial& rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
            rkMtrl.ApplyRenderState();
            STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
            rkMtrl.RestoreRenderState();

            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        /////

        pMeshNode = pMeshNode->pNextMeshNode;
    }
}

// With Two Texture
void CGrannyModelInstance::RenderMeshNodeListWithTwoTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
    assert(m_pModel != NULL);

    LPDIRECT3DINDEXBUFFER9 lpd3dIdxBuf = m_pModel->GetD3DIndexBuffer();
    assert(lpd3dIdxBuf != NULL);

    const CGrannyModel::TMeshNode * pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);

    while (pMeshNode)
    {
        const CGrannyMesh * pMesh = pMeshNode->pMesh;
        int vtxMeshBasePos = pMesh->GetVertexBasePosition();

        STATEMANAGER.SetIndices(lpd3dIdxBuf, vtxMeshBasePos);
        STATEMANAGER.SetTransform(D3DTS_WORLD, &m_meshMatrices[pMeshNode->iMesh]);

        /////
        const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
        int vtxCount = pMesh->GetVertexCount();

        while (pTriGroupNode)
        {
            ms_faceCount += pTriGroupNode->triCount;

            const CGrannyMaterial& rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
            STATEMANAGER.SetTexture(0, rkMtrl.GetD3DTexture(0));
            STATEMANAGER.SetTexture(1, rkMtrl.GetD3DTexture(1));
            STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        /////

        pMeshNode = pMeshNode->pNextMeshNode;
    }
}

// Without Texture
void CGrannyModelInstance::RenderMeshNodeListWithoutTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
    assert(m_pModel != NULL);

    LPDIRECT3DINDEXBUFFER9 lpd3dIdxBuf = m_pModel->GetD3DIndexBuffer();
    assert(lpd3dIdxBuf != NULL);

    const CGrannyModel::TMeshNode * pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);

    while (pMeshNode)
    {
        const CGrannyMesh * pMesh = pMeshNode->pMesh;
        int vtxMeshBasePos = pMesh->GetVertexBasePosition();

        STATEMANAGER.SetIndices(lpd3dIdxBuf, vtxMeshBasePos);
        STATEMANAGER.SetTransform(D3DTS_WORLD, &m_meshMatrices[pMeshNode->iMesh]);

        /////
        const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
        int vtxCount = pMesh->GetVertexCount();

        while (pTriGroupNode)
        {
            ms_faceCount += pTriGroupNode->triCount;
            STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
            pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
        }

        /////

        pMeshNode = pMeshNode->pNextMeshNode;
    }
}
