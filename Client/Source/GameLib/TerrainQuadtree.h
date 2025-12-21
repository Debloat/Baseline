// TerrainQuadtreeNode.h: interface for the CTerrainQuadtreeNode class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CTerrainQuadtreeNode
{
    public:
        CTerrainQuadtreeNode();
        virtual ~CTerrainQuadtreeNode();

    public:
        long					x0, y0, x1, y1;
        CTerrainQuadtreeNode* 	NW_Node;
        CTerrainQuadtreeNode* 	NE_Node;
        CTerrainQuadtreeNode* 	SW_Node;
        CTerrainQuadtreeNode* 	SE_Node;
        long					Size;
        long					PatchNum;
        D3DXVECTOR3				center;
        float					radius;
        BYTE					m_byLODLevel;
};
