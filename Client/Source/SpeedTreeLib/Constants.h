///////////////////////////////////////////////////////////////////////
//  Constants
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
//

#pragma once

// used to isolate SpeedGrass usage
#define USE_SPEEDGRASS

// vertex shader constant locations
const   int     c_nShaderLightPosition = 16;
const   int     c_nShaderGrassBillboard = 4;
const   int     c_nShaderGrassLodParameters = 8;
const   int     c_nShaderCameraPosition = 9;
const   int     c_nShaderGrassWindDirection = 10;
const   int     c_nShaderGrassPeriods = 11;
const   int     c_nShaderLeafAmbient = 47;
const   int     c_nShaderLeafDiffuse = 48;
const   int     c_nShaderLeafSpecular = 49;
const   int     c_nShaderLeafLightingAdjustment = 50;

// grass
const   float   c_fDefaultGrassFarLod = 300.0f;
const   float   c_fGrassFadeLength = 50.0f;
const   float   c_fMinBladeNoise = -0.2f;
const   float   c_fMaxBladeNoise = 0.2f;
const   float   c_fMinBladeThrow = 1.0f;
const   float   c_fMaxBladeThrow = 2.5f;
const   float   c_fMinBladeSize = 7.5f;
const   float   c_fMaxBladeSize = 10.0f;
const   int     c_nNumBladeMaps = 2;
const   float   c_fGrassBillboardWideScalar = 1.75f;
const   float   c_fWalkerHeight = 12.0f;
const   int     c_nDefaultGrassBladeCount = 33000;
const   int     c_nGrassRegionCount = 20;

// misc
const   float   c_fPi = 3.1415926535897932384626433832795f;
const   float   c_fHalfPi = c_fPi * 0.5f;
const   float   c_fQuarterPi = c_fPi * 0.25f;
const   float   c_fTwoPi = 2.0f * c_fPi;
const   float   c_fDeg2Rad = 57.29578f;                             // divide degrees by this to get radians
const   float   c_f90 = 0.5f * c_fPi;
