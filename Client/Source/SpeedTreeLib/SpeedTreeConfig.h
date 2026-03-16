///////////////////////////////////////////////////////////////////////
//  SpeedTreeRT runtime configuration #defines
//
//  (c) 2003 IDV, Inc.
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com

#pragma once

const int       c_nNumWindMatrices = 4;
const float     c_fNearLodFactor = 2.0f;
const float     c_fFarLodFactor = 9.0f;

// vertex shader constant locations
const int c_nVertexShader_LeafLightingAdjustment = 70;
const int c_nVertexShader_Light = 71;
const int c_nVertexShader_Material = 74;
const int c_nVertexShader_TreePos = 52;
const int c_nVertexShader_CompoundMatrix = 0;
const int c_nVertexShader_WindMatrices = 54;
const int c_nVertexShader_LeafTables = 4;

/* ================= WIND ================= */
#define WRAPPER_USE_GPU_WIND
/* ======================================== */

/* ================= LEAF ================= */
#define WRAPPER_USE_GPU_LEAF_PLACEMENT
/* ======================================== */

/* ============= SELF SHADOWS ============= */
#define WRAPPER_RENDER_SELF_SHADOWS
/* ======================================== */

// texture coordinates (enable this define for DirectX-based engines)
#define WRAPPER_FLIP_T_TEXCOORD

// up vector
//#define WRAPPER_UP_POS_Y
#define WRAPPER_UP_POS_Z

// loading from STF or clones/instances? (enable ONE of the two below)
//#define WRAPPER_FOREST_FROM_STF
#define WRAPPER_FOREST_FROM_INSTANCES

// billboard modes
#define WRAPPER_BILLBOARD_MODE
//#define WRAPPER_RENDER_HORIZONTAL_BILLBOARD


/* ERRORS */
#if defined WRAPPER_UP_POS_Y && defined WRAPPER_UP_POS_Z
#error Please define exactly one up vector
#endif

#if defined WRAPPER_FOREST_FROM_STF && defined WRAPPER_FOREST_FROM_INSTANCES
#error Please define exactly one loading mechanism
#endif
