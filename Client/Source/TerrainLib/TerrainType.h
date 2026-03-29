#pragma once

#include "../EterLib/GrpVertexBuffer.h"
#include "../EterLib/GrpIndexBuffer.h"

#define TERRAIN_PATCHSIZE	16
#define TERRAIN_SIZE		128
#define TERRAIN_PATCHCOUNT	TERRAIN_SIZE/TERRAIN_PATCHSIZE
#define MAXTERRAINTEXTURES	256

using TTerainSplat = struct
{
    long					Active;
    long					NeedsUpdate;
    LPDIRECT3DTEXTURE9		pd3dTexture;
};

using TTerrainSplatPatch = struct
{
    DWORD			TileCount[MAXTERRAINTEXTURES];
    DWORD			PatchTileCount[TERRAIN_PATCHCOUNT * TERRAIN_PATCHCOUNT][MAXTERRAINTEXTURES];
    TTerainSplat 	Splats[MAXTERRAINTEXTURES];
    bool			m_bNeedsUpdate;
};

using TERRAIN_VBUFFER = struct
{
    char					used;
    short					mat;

    CGraphicVertexBuffer	vb;
    CGraphicIndexBuffer		ib;
    long					VertexSize;

    short					NumIndices;

    float minx, maxx;
    float miny, maxy;
    float minz, maxz;
};

using PR_MATERIAL = struct
{
    char name[19];
    float ambi_r;
    float ambi_g;
    float ambi_b;
    float ambi_a;		/* Ambient Color */
    float diff_r;
    float diff_g;
    float diff_b;
    float diff_a;		/* Diffuse Color */
    float spec_r;
    float spec_g;
    float spec_b;
    float spec_a;		/* Specular Color */
    float spec_power;   /* Specular power */
};

using TTerrainGlobals = struct
{
    /* Public Settings */
    float			PageUVLength;
    long			SquaresPerTexture;              /* Heightfield squares per texture (128 texels) */
    long			SplatTilesX;					/* Number of splat textures across map */
    long			SplatTilesY;					/* Number of splat textures down map */
    long			DisableWrapping;

    /* - SHADOWS ------------------------------------------- */
    long			DisableShadow;
    long			ShadowMode;
    /* ----------------------------------------------------- */

    long			OutsideVisible;
    D3DXVECTOR3		SunLocation;
};

/* Converts a floating point number to an integer by truncation, using
   the FISTP instruction */
#define PR_FLOAT_TO_INTASM __asm	\
    {									\
        __asm fld PR_FCNV				\
        __asm fistp PR_ICNV				\
    }

#define PR_FLOAT_TO_FIXED(inreal, outint)	\
    {											\
        PR_FCNV = (inreal) * 65536.0f;			\
        PR_FLOAT_TO_INTASM;						\
        (outint) = PR_ICNV;						\
    }

#define PR_FLOAT_TO_INT(inreal, outint)							\
    {																\
        PR_FCNV = (inreal);											\
        PR_FLOAT_TO_INTASM;											\
        (outint) = PR_ICNV > PR_FCNV ? PR_ICNV - 1 : PR_ICNV;		\
    }

#define PR_FLOAT_ADD_TO_INT(inreal, outint)	\
    {											\
        PR_FCNV = (inreal);						\
        PR_FLOAT_TO_INTASM;						\
        (outint) += PR_ICNV;					\
    }
