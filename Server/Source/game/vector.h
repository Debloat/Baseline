#pragma once

typedef struct SVector
{
    float x;
    float y;
    float z;
} VECTOR;

extern void		Normalize(VECTOR * pV1, VECTOR * pV2);
extern float    GetDegreeFromPosition(float x, float y);
extern float    GetDegreeFromPositionXY(long sx, long sy, long ex, long ey);
extern void     GetDeltaByDegree(float fDegree, float fDistance, float* x, float* y);
extern float	GetDegreeDelta(float iDegree, float iDegree2);
